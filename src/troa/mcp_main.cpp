// SPDX-License-Identifier: GPL-3.0-or-later
// Local MCP stdio companion. Controller operation stays in the GUI process.
#include "identity.h"
#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocalSocket>
#include <iostream>
#include <string>

namespace {
QJsonObject rpcError(const QJsonValue &id, int code, const QString &message)
{
    return {{"jsonrpc", "2.0"}, {"id", id}, {"error", QJsonObject{{"code", code}, {"message", message}}}};
}
QJsonObject tool(const QString &name, const QString &description, QJsonObject properties = {},
                 QJsonArray required = {}, bool readOnly = true)
{
    return {{"name", name}, {"description", description},
        {"inputSchema", QJsonObject{{"type", "object"}, {"properties", properties}, {"required", required},
                                    {"additionalProperties", false}}},
        {"annotations", QJsonObject{{"readOnlyHint", readOnly}, {"destructiveHint", false}, {"openWorldHint", false}}}};
}
QJsonArray tools()
{
    const QJsonObject text{{"type", "string"}};
    return {
        tool("mapper_status", "Read mapper status, supported profile inputs and keys, and managed profile location."),
        tool("list_controllers", "List connected controllers, capabilities, active profile, and stable/live identifiers."),
        tool("application_context", "Read the focused app, actual active profile/layout per controller, application-rule matches and switch warnings."),
        tool("list_application_rules", "Read saved application rules and the revision needed to change them."),
        tool("save_application_rule", "Create/update a Windows application rule. Read rules and context first. rule has id, name, "
             "executable (existing .exe), controller_id (persistent id from application_context), profile_path (existing native "
             ".amgp/.xml), modes [{set:1..8,name}], optional keyboard_shortcut and controller_button (zero-based normalized "
             "SDL index, -1 disables). Optional controller_name/button_name are display labels. Profile actions remain in "
             "the native file. Saving enables automatic activation when that app is focused; refuses unsaved controller edits. "
             "Controller switch buttons must be unassigned in every selected set.",
             {{"rule", QJsonObject{{"type", "object"}}}, {"expected_revision", text}}, {"rule", "expected_revision"}, false),
        tool("remove_application_rule", "Remove an application rule without deleting its profile file. Requires the current rules revision.",
             {{"id", text}, {"expected_revision", text}}, {"id", "expected_revision"}, false),
        tool("list_profiles", "List bundled templates and personal profiles, including revision hashes."),
        tool("read_profile", "Read a profile definition and its revision. Bundled templates are read-only.", {{"id", text}}, {"id"}),
        tool("export_profile", "Export an exact managed revision to a native .amgp file for application rules, without activating it.",
             {{"id", text}, {"expected_revision", text}}, {"id", "expected_revision"}, false),
        tool("validate_profile", "Validate a structured profile without saving or activating it.",
             {{"profile", QJsonObject{{"type", "object"}}}}, {"profile"}),
        tool("save_profile", "Create or update a personal profile. Use read_profile first, preserve its fields, and supply "
             "expected_revision for updates. To copy a template, change its id and name. For multiple layouts, keep top-level "
             "bindings empty and add layouts [{set:1..8,name,bindings}]; include set 1. Saving does not activate.",
             {{"profile", QJsonObject{{"type", "object"}}}, {"expected_revision", text}}, {"profile"}, false),
        tool("list_profile_revisions", "List preserved older revisions of a personal profile.", {{"id", text}}, {"id"}),
        tool("restore_profile_revision", "Restore an older personal profile revision, preserving the current revision. "
             "This does not change the active controller mapping.", {{"id", text}, {"revision", text}, {"expected_revision", text}},
             {"id", "revision", "expected_revision"}, false),
        tool("activate_profile", "Apply a managed profile to one SDL-mapped controller. Requires the exact controller_id "
             "from list_controllers and profile revision from read_profile. Refuses unsaved GUI changes.",
             {{"id", text}, {"controller_id", text}, {"expected_revision", text}}, {"id", "controller_id", "expected_revision"}, false),
        tool("unload_profile", "Clear mappings on one controller. Refuses unsaved GUI changes. Requires the exact "
             "controller_id from list_controllers.", {{"controller_id", text}}, {"controller_id"}, false)
    };
}
QJsonObject forward(const QString &name, const QJsonObject &arguments)
{
    QLocalSocket socket;
    socket.connectToServer(Troa::socketName());
    if (!socket.waitForConnected(2000)) return {{"error", "Open Bifrost Controller and enable Assistant access."}};
    socket.write(QJsonDocument(QJsonObject{{"method", name}, {"arguments", arguments}}).toJson(QJsonDocument::Compact) + '\n');
    if (socket.bytesToWrite() > 0 && !socket.waitForBytesWritten(2000)) return {{"error", "The request could not be delivered to the mapper."}};
    QByteArray response;
    while (!response.contains('\n')) {
        if (!socket.bytesAvailable() && !socket.waitForReadyRead(10000))
            return {{"error", "Mapper response timed out. Check its current state before retrying a change."}};
        response += socket.readAll();
        if (response.size() > 512 * 1024) return {{"error", "Mapper response exceeded the allowed size."}};
    }
    QJsonParseError error;
    const auto doc = QJsonDocument::fromJson(response.left(response.indexOf('\n')), &error);
    return error.error == QJsonParseError::NoError && doc.isObject() ? doc.object()
        : QJsonObject{{"error", "Mapper returned an invalid response."}};
}
void send(const QJsonObject &response)
{
    std::cout << QJsonDocument(response).toJson(QJsonDocument::Compact).constData() << '\n' << std::flush;
}
} // namespace

int main(int argc, char **argv)
{
    QCoreApplication application(argc, argv);
    QCoreApplication::setApplicationName(Troa::slug());
    bool initialized = false;
    std::string line;
    while (true) {
        line.clear();
        bool oversized = false;
        char character;
        while (std::cin.get(character) && character != '\n') {
            if (line.size() < 256 * 1024) line.push_back(character); else oversized = true;
        }
        if (line.empty() && !std::cin) break;
        if (oversized) { send(rpcError(QJsonValue::Null, -32600, "Request exceeds 256 KiB.")); continue; }
        QJsonParseError error;
        const auto doc = QJsonDocument::fromJson(QByteArray::fromStdString(line), &error);
        if (error.error != QJsonParseError::NoError) { send(rpcError(QJsonValue::Null, -32700, "Invalid JSON.")); continue; }
        if (!doc.isObject()) { send(rpcError(QJsonValue::Null, -32600, "Expected a JSON-RPC object.")); continue; }
        const auto request = doc.object();
        const auto id = request.value("id");
        const auto method = request.value("method").toString();
        if (request.value("jsonrpc") != "2.0" || method.isEmpty()) { send(rpcError(id.isUndefined() ? QJsonValue::Null : id, -32600, "Invalid JSON-RPC request.")); continue; }
        if (id.isUndefined()) continue; // Notifications do not have responses.
        QJsonObject result;
        if (method == "initialize") {
            initialized = true;
            result = {{"protocolVersion", "2025-11-25"}, {"capabilities", QJsonObject{{"tools", QJsonObject{}}}},
                {"serverInfo", QJsonObject{{"name", "bifrost-controller"}, {"version", "0.1.2"}}},
                {"instructions", "Manage controller profiles locally. Read before changing, save drafts before activation, "
                 "and use exact revision/controller ids. This server does not inject input or run scripts."}};
        } else if (method == "ping") result = {};
        else if (!initialized) { send(rpcError(id, -32002, "Initialize the server first.")); continue; }
        else if (method == "tools/list") result = {{"tools", tools()}};
        else if (method == "tools/call") {
            const auto params = request.value("params").toObject();
            const auto name = params.value("name").toString();
            bool found = false;
            for (const auto &value : tools()) if (value.toObject().value("name").toString() == name) found = true;
            if (!found) { send(rpcError(id, -32602, "Unknown tool.")); continue; }
            if (params.contains("arguments") && !params.value("arguments").isObject()) {
                send(rpcError(id, -32602, "arguments must be an object.")); continue;
            }
            const auto data = forward(name, params.value("arguments").toObject());
            result = {{"content", QJsonArray{QJsonObject{{"type", "text"},
                       {"text", QString::fromUtf8(QJsonDocument(data).toJson(QJsonDocument::Indented))}}}},
                      {"structuredContent", data}, {"isError", data.contains("error")}};
        } else { send(rpcError(id, -32601, "Method not found.")); continue; }
        send({{"jsonrpc", "2.0"}, {"id", id}, {"result", result}});
    }
    return 0;
}
