// SPDX-License-Identifier: GPL-3.0-or-later
#include "profilestore.h"
#include "identity.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QLockFile>
#include <QMap>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>
#include <QUuid>
#include <QXmlStreamWriter>
#include <Qt>

namespace {
const QStringList buttons = {"a", "b", "x", "y", "back", "guide", "start", "left_stick_press",
                             "right_stick_press", "left_shoulder", "right_shoulder", "dpad_up", "dpad_down",
                             "dpad_left", "dpad_right"};
const QStringList directions = {"up", "right", "down", "left"};
const QMap<QString, int> keys = {{"Enter", Qt::Key_Return}, {"Escape", Qt::Key_Escape}, {"Space", Qt::Key_Space},
    {"Tab", Qt::Key_Tab}, {"Backspace", Qt::Key_Backspace}, {"Delete", Qt::Key_Delete}, {"Insert", Qt::Key_Insert},
    {"Home", Qt::Key_Home}, {"End", Qt::Key_End}, {"PageUp", Qt::Key_PageUp}, {"PageDown", Qt::Key_PageDown},
    {"Up", Qt::Key_Up}, {"Down", Qt::Key_Down}, {"Left", Qt::Key_Left}, {"Right", Qt::Key_Right},
    {"Ctrl", Qt::Key_Control}, {"Shift", Qt::Key_Shift}, {"Alt", Qt::Key_Alt}, {"Meta", Qt::Key_Meta},
    {"F1", Qt::Key_F1}, {"F2", Qt::Key_F2}, {"F3", Qt::Key_F3}, {"F4", Qt::Key_F4}, {"F5", Qt::Key_F5},
    {"F6", Qt::Key_F6}, {"F7", Qt::Key_F7}, {"F8", Qt::Key_F8}, {"F9", Qt::Key_F9}, {"F10", Qt::Key_F10},
    {"F11", Qt::Key_F11}, {"F12", Qt::Key_F12}};
int keyCode(const QString &key)
{
    if (keys.contains(key)) return keys.value(key);
    if (key.size() == 1 && key.at(0).unicode() >= 32 && key.at(0).unicode() <= 126)
        return key.toUpper().at(0).unicode();
    return 0;
}
QJsonObject readFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly) || file.size() > 256 * 1024) return {};
    QJsonParseError error;
    const auto doc = QJsonDocument::fromJson(file.readAll(), &error);
    return error.error == QJsonParseError::NoError && doc.isObject() ? doc.object() : QJsonObject{};
}
bool writeFile(const QString &filePath, const QJsonObject &value)
{
    QSaveFile file(filePath);
    const auto bytes = QJsonDocument(value).toJson(QJsonDocument::Indented);
    return file.open(QIODevice::WriteOnly) && file.write(bytes) == bytes.size() && file.commit();
}
void slot(QXmlStreamWriter &xml, const QString &mode, int code)
{
    xml.writeStartElement("slot");
    xml.writeTextElement("code", QString::number(code));
    xml.writeTextElement("mode", mode);
    xml.writeEndElement();
}
void bindingXml(QXmlStreamWriter &xml, const QJsonObject &binding, const QString &tag, int index)
{
    xml.writeStartElement(tag);
    xml.writeAttribute("index", QString::number(index));
    xml.writeTextElement("actionname", binding.value("label").toString());
    xml.writeStartElement("slots");
    const auto chord = binding.value("keys").toArray();
    if (chord.size() > 1) { xml.writeStartElement("slot"); xml.writeStartElement("slots"); }
    for (const auto &key : chord) slot(xml, "keyboard", keyCode(key.toString()));
    if (chord.size() > 1) { xml.writeEndElement(); xml.writeTextElement("mode", "mix"); xml.writeEndElement(); }
    if (binding.contains("mouse_button")) slot(xml, "mousebutton", binding.value("mouse_button").toInt());
    if (binding.contains("mouse_move")) {
        const QMap<QString, int> mouse = {{"up", 1}, {"down", 2}, {"left", 3}, {"right", 4}};
        slot(xml, "mousemovement", mouse.value(binding.value("mouse_move").toString()));
    }
    xml.writeEndElement();
    xml.writeEndElement();
}
} // namespace

namespace Troa {
QJsonObject failure(const QString &message) { return {{"error", message}}; }
bool ProfileStore::validId(const QString &id)
{
    return QRegularExpression(QStringLiteral("^[a-z0-9][a-z0-9-]{0,63}$")).match(id).hasMatch();
}
QString ProfileStore::path(const QString &id)
{
    return validId(id) ? QDir(profileDirectory()).filePath(id + ".json") : QString{};
}
QString ProfileStore::revision(const QJsonObject &profile)
{
    return QString::fromLatin1(QCryptographicHash::hash(QJsonDocument(profile).toJson(QJsonDocument::Compact),
                                                     QCryptographicHash::Sha256).toHex());
}
QJsonArray ProfileStore::catalog()
{
    QFile file(":/troa/catalog.json");
    if (!file.open(QIODevice::ReadOnly)) return {};
    return QJsonDocument::fromJson(file.readAll()).array();
}
QStringList ProfileStore::inputs()
{
    auto result = buttons;
    for (const auto &stick : {QString("left_stick"), QString("right_stick")})
        for (const auto &direction : directions) result.append(stick + "_" + direction);
    return result;
}
QStringList ProfileStore::namedKeys() { return keys.keys(); }
QString ProfileStore::validate(const QJsonObject &profile)
{
    if (!validId(profile.value("id").toString())) return "id must be a lowercase slug, at most 64 characters.";
    const auto name = profile.value("name").toString().trimmed();
    if (name.isEmpty() || name.size() > 120) return "name must contain 1 to 120 characters.";
    if (profile.value("schema_version").toInt() != 1) return "schema_version must be 1.";
    if (profile.value("controller").toString() != "sdl-gamecontroller") return "Use controller: sdl-gamecontroller.";
    if (profile.value("description").toString().size() > 2000) return "description is too long.";
    if (!QStringList{"desktop", "browser", "game", "custom"}.contains(profile.value("category").toString()))
        return "category must be desktop, browser, game, or custom.";
    const int deadZone = profile.value("dead_zone").toInt(8000);
    if (deadZone < 1000 || deadZone > 30000) return "dead_zone must be between 1000 and 30000.";
    if (!profile.value("bindings").isArray() || profile.value("bindings").toArray().size() > inputs().size())
        return "bindings must be an array with at most one entry per supported input.";
    QSet<QString> seen;
    for (const auto &value : profile.value("bindings").toArray()) {
        if (!value.isObject()) return "Each binding must be an object.";
        const auto binding = value.toObject();
        const auto input = binding.value("input").toString();
        if (!inputs().contains(input) || seen.contains(input)) return "Unsupported or duplicate input: " + input;
        seen.insert(input);
        if (binding.value("label").toString().size() > 120) return "Binding labels must be at most 120 characters.";
        int actions = 0;
        if (binding.contains("keys")) {
            ++actions;
            const auto chord = binding.value("keys").toArray();
            if (!binding.value("keys").isArray() || chord.isEmpty() || chord.size() > 4)
                return "keys must contain 1 to 4 keys, pressed together.";
            QSet<QString> chordSeen;
            for (const auto &key : chord) {
                if (!key.isString() || !keyCode(key.toString()) || chordSeen.contains(key.toString()))
                    return "Unsupported or duplicate keyboard key.";
                chordSeen.insert(key.toString());
            }
        }
        if (binding.contains("mouse_button")) {
            ++actions;
            const auto code = binding.value("mouse_button");
            if (!code.isDouble() || code.toDouble() != code.toInt() || code.toInt() < 1 || code.toInt() > 9)
                return "mouse_button must be an integer from 1 to 9.";
        }
        if (binding.contains("mouse_move")) {
            ++actions;
            if (!directions.contains(binding.value("mouse_move").toString())) return "Invalid mouse movement direction.";
        }
        if (actions != 1) return "Each binding needs exactly one of keys, mouse_button, or mouse_move.";
        const QSet<QString> fields = {"input", "label", "keys", "mouse_button", "mouse_move"};
        for (const auto &field : binding.keys()) if (!fields.contains(field)) return "Unknown binding field: " + field;
    }
    const QSet<QString> fields = {"id", "name", "description", "category", "controller", "schema_version", "dead_zone", "bindings"};
    for (const auto &field : profile.keys()) if (!fields.contains(field)) return "Unknown profile field: " + field;
    return {};
}
QJsonObject ProfileStore::read(const QString &id) const
{
    if (!validId(id)) return failure("Invalid profile id.");
    QJsonObject profile;
    bool bundled = id.startsWith("builtin-");
    if (bundled) {
        for (const auto &value : catalog()) if (value.toObject().value("id").toString() == id) profile = value.toObject();
    } else profile = readFile(path(id));
    if (profile.isEmpty()) return failure("Profile was not found.");
    const auto error = validate(profile);
    if (!error.isEmpty()) return failure("Profile is invalid: " + error);
    return {{"profile", profile}, {"revision", revision(profile)}, {"bundled", bundled}};
}
QJsonArray ProfileStore::list() const
{
    QJsonArray result;
    QStringList ids;
    for (const auto &value : catalog()) ids.append(value.toObject().value("id").toString());
    for (const auto &file : QDir(profileDirectory()).entryList({"*.json"}, QDir::Files))
        if (!file.startsWith("builtin-")) ids.append(QFileInfo(file).completeBaseName());
    for (const auto &id : ids) {
        const auto item = read(id);
        if (item.contains("error")) continue;
        auto summary = item.value("profile").toObject();
        summary.remove("bindings");
        summary.insert("binding_count", item.value("profile").toObject().value("bindings").toArray().size());
        summary.insert("revision", item.value("revision"));
        summary.insert("bundled", item.value("bundled"));
        result.append(summary);
    }
    return result;
}
QJsonObject ProfileStore::save(const QJsonObject &profile, const QString &expectedRevision) const
{
    const auto error = validate(profile);
    if (!error.isEmpty()) return failure(error);
    const auto id = profile.value("id").toString();
    if (id.startsWith("builtin-")) return failure("Bundled profiles are read-only. Copy to a new personal id first.");
    if (!QDir().mkpath(profileDirectory())) return failure("Cannot create the profile library.");
    QLockFile lock(QDir(profileDirectory()).filePath("library.lock"));
    if (!lock.tryLock(0)) return failure("Profile library is busy; retry shortly.");
    const bool exists = QFileInfo::exists(path(id));
    if (exists) {
        const auto current = read(id);
        if (current.contains("error")) return current;
        if (expectedRevision.isEmpty() || expectedRevision != current.value("revision").toString())
            return failure("Profile changed or expected_revision is missing. Read it again before saving.");
        const auto directory = QDir(profileDirectory()).filePath("revisions/" + id);
        if (!QDir().mkpath(directory) || !writeFile(QDir(directory).filePath(expectedRevision + ".json"),
                                                 current.value("profile").toObject()))
            return failure("Cannot preserve the previous revision; profile was not changed.");
    } else if (!expectedRevision.isEmpty()) return failure("Profile no longer exists; expected_revision must be empty for creation.");
    if (!writeFile(path(id), profile)) return failure("Could not atomically save the profile.");
    return read(id);
}
QJsonArray ProfileStore::revisions(const QString &id) const
{
    QJsonArray result;
    if (!validId(id)) return result;
    const QDir directory(QDir(profileDirectory()).filePath("revisions/" + id));
    for (const auto &file : directory.entryInfoList({"*.json"}, QDir::Files, QDir::Time))
        result.append(QJsonObject{{"revision", file.completeBaseName()}, {"saved_at", file.lastModified().toUTC().toString(Qt::ISODate)}});
    return result;
}
QJsonObject ProfileStore::restore(const QString &id, const QString &hash, const QString &expectedRevision) const
{
    if (!validId(id) || !QRegularExpression("^[a-f0-9]{64}$").match(hash).hasMatch()) return failure("Invalid id or revision.");
    const auto old = readFile(QDir(profileDirectory()).filePath("revisions/" + id + "/" + hash + ".json"));
    if (old.isEmpty() || old.value("id").toString() != id || revision(old) != hash) return failure("Revision was not found or is damaged.");
    return save(old, expectedRevision);
}
QString ProfileStore::exportMapping(const QString &id) const
{
    const auto item = read(id);
    if (item.contains("error")) return {};
    const auto profile = item.value("profile").toObject();
    const QString directory = QDir(profileDirectory()).filePath("compiled/" + id);
    if (!QDir().mkpath(directory)) return {};
    const auto filePath = QDir(directory).filePath(item.value("revision").toString() + ".amgp");
    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) return {};
    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement("gamecontroller");
    xml.writeAttribute("configversion", "19");
    xml.writeAttribute("appversion", "3.6.1");
    xml.writeTextElement("profilename", profile.value("name").toString());
    xml.writeStartElement("sets"); xml.writeStartElement("set"); xml.writeAttribute("index", "1");
    const auto bindings = profile.value("bindings").toArray();
    for (const auto &value : bindings) {
        const auto binding = value.toObject();
        const int index = buttons.indexOf(binding.value("input").toString());
        if (index >= 0 && index < 11) bindingXml(xml, binding, "button", index + 1);
    }
    xml.writeStartElement("dpad"); xml.writeAttribute("index", "1");
    for (int direction = 0; direction < directions.size(); ++direction) {
        const auto input = "dpad_" + directions.at(direction);
        for (const auto &value : bindings)
            if (value.toObject().value("input").toString() == input)
                bindingXml(xml, value.toObject(), "dpadbutton", 1 << direction);
    }
    xml.writeEndElement();
    for (int stick = 0; stick < 2; ++stick) {
        xml.writeStartElement("stick"); xml.writeAttribute("index", QString::number(stick + 1));
        xml.writeTextElement("deadZone", QString::number(profile.value("dead_zone").toInt(8000)));
        for (int direction = 0; direction < directions.size(); ++direction) {
            const auto input = QString(stick == 0 ? "left_stick_" : "right_stick_") + directions.at(direction);
            for (const auto &value : bindings)
                if (value.toObject().value("input").toString() == input)
                    bindingXml(xml, value.toObject(), "stickbutton", 1 << direction);
        }
        xml.writeEndElement();
    }
    xml.writeEndElement(); xml.writeEndElement(); xml.writeEndElement(); xml.writeEndDocument();
    return !xml.hasError() && file.commit() ? filePath : QString{};
}
} // namespace Troa
