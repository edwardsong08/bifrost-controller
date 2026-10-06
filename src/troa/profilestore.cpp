// SPDX-License-Identifier: GPL-3.0-or-later
#include "profilestore.h"
#include "controllersupport.h"
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
const QStringList buttons = Troa::buttonInputs();
const QStringList directions = {"up", "right", "down", "left"};
const QMap<QString, int> keys = {{"Enter", Qt::Key_Return},
                                 {"Escape", Qt::Key_Escape},
                                 {"Space", Qt::Key_Space},
                                 {"Tab", Qt::Key_Tab},
                                 {"Backspace", Qt::Key_Backspace},
                                 {"Delete", Qt::Key_Delete},
                                 {"Insert", Qt::Key_Insert},
                                 {"Home", Qt::Key_Home},
                                 {"End", Qt::Key_End},
                                 {"PageUp", Qt::Key_PageUp},
                                 {"PageDown", Qt::Key_PageDown},
                                 {"Up", Qt::Key_Up},
                                 {"Down", Qt::Key_Down},
                                 {"Left", Qt::Key_Left},
                                 {"Right", Qt::Key_Right},
                                 {"Ctrl", Qt::Key_Control},
                                 {"Shift", Qt::Key_Shift},
                                 {"Alt", Qt::Key_Alt},
                                 {"Meta", Qt::Key_Meta},
                                 {"F1", Qt::Key_F1},
                                 {"F2", Qt::Key_F2},
                                 {"F3", Qt::Key_F3},
                                 {"F4", Qt::Key_F4},
                                 {"F5", Qt::Key_F5},
                                 {"F6", Qt::Key_F6},
                                 {"F7", Qt::Key_F7},
                                 {"F8", Qt::Key_F8},
                                 {"F9", Qt::Key_F9},
                                 {"F10", Qt::Key_F10},
                                 {"F11", Qt::Key_F11},
                                 {"F12", Qt::Key_F12},
                                 {"F13", Qt::Key_F13},
                                 {"F14", Qt::Key_F14},
                                 {"F15", Qt::Key_F15},
                                 {"F16", Qt::Key_F16},
                                 {"F17", Qt::Key_F17},
                                 {"F18", Qt::Key_F18},
                                 {"F19", Qt::Key_F19},
                                 {"F20", Qt::Key_F20},
                                 {"F21", Qt::Key_F21},
                                 {"F22", Qt::Key_F22},
                                 {"F23", Qt::Key_F23},
                                 {"F24", Qt::Key_F24},
                                 {"CapsLock", Qt::Key_CapsLock},
                                 {"NumLock", Qt::Key_NumLock},
                                 {"ScrollLock", Qt::Key_ScrollLock},
                                 {"PrintScreen", Qt::Key_Print},
                                 {"Pause", Qt::Key_Pause},
                                 {"Menu", Qt::Key_Menu}};
int keyCode(const QString &key)
{
    if (keys.contains(key))
        return keys.value(key);
    if (key.size() == 1 && key.at(0).unicode() >= 32 && key.at(0).unicode() <= 126)
        return key.toUpper().at(0).unicode();
    return 0;
}
QJsonObject readFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly) || file.size() > 256 * 1024)
        return {};
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
    if (chord.size() > 1)
    {
        xml.writeStartElement("slot");
        xml.writeStartElement("slots");
    }
    for (const auto &key : chord)
        slot(xml, "keyboard", keyCode(key.toString()));
    if (chord.size() > 1)
    {
        xml.writeEndElement();
        xml.writeTextElement("mode", "mix");
        xml.writeEndElement();
    }
    if (binding.contains("mouse_button"))
        slot(xml, "mousebutton", binding.value("mouse_button").toInt());
    if (binding.contains("mouse_move"))
    {
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
    return QString::fromLatin1(
        QCryptographicHash::hash(QJsonDocument(profile).toJson(QJsonDocument::Compact), QCryptographicHash::Sha256).toHex());
}
QJsonArray ProfileStore::catalog()
{
    QFile file(":/troa/catalog.json");
    QJsonArray result;
    if (file.open(QIODevice::ReadOnly))
        result = QJsonDocument::fromJson(file.readAll()).array();
    QFile cache(QDir(dataDirectory()).filePath("community-catalog-v2.json"));
    if (!cache.open(QIODevice::ReadOnly) || cache.size() > 256 * 1024)
        return result;
    const auto downloaded = QJsonDocument::fromJson(cache.readAll()).array();
    for (const auto &value : downloaded)
    {
        const auto profile = value.toObject();
        if (!profile.value("id").toString().startsWith("builtin-") || !validate(profile).isEmpty())
            continue;
        int index = -1;
        for (int i = 0; i < result.size(); ++i)
            if (result.at(i).toObject().value("id") == profile.value("id"))
                index = i;
        if (index >= 0)
            result.replace(index, profile);
        else
            result.append(profile);
    }
    return result;
}
QJsonObject ProfileStore::installCatalog(const QByteArray &bytes)
{
    if (bytes.size() > 256 * 1024)
        return failure("Community download is too large. Existing profiles were kept.");
    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(bytes, &error);
    if (error.error != QJsonParseError::NoError || !document.isArray() || document.array().isEmpty() ||
        document.array().size() > 100)
        return failure("Invalid community download. Existing profiles were kept.");
    QSet<QString> ids;
    for (const auto &value : document.array())
    {
        const auto profile = value.toObject();
        const auto id = profile.value("id").toString();
        const auto validation = validate(profile);
        if (!id.startsWith("builtin-"))
            return failure("Community template IDs must start with builtin-. Existing profiles were kept.");
        if (ids.contains(id))
            return failure("Duplicate community template: " + id + ". Existing profiles were kept.");
        if (!validation.isEmpty())
            return failure("Community template " + id + ": " + validation + " Existing profiles were kept.");
        ids.insert(id);
    }
    if (!QDir().mkpath(dataDirectory()))
        return failure("Could not create the community cache folder.");
    const auto path = QDir(dataDirectory()).filePath("community-catalog-v2.json");
    QFile existing(path);
    const auto normalized = document.toJson(QJsonDocument::Indented);
    if (normalized.size() > 256 * 1024)
        return failure("Community download is too large. Existing profiles were kept.");
    const bool unchanged = existing.open(QIODevice::ReadOnly) && existing.readAll() == normalized;
    existing.close();
    QSaveFile cache(path);
    if (!unchanged && (!cache.open(QIODevice::WriteOnly) || cache.write(normalized) != normalized.size() || !cache.commit()))
        return failure("Could not save community templates. Existing profiles were kept.");
    return {{"count", document.array().size()}, {"changed", !unchanged}};
}
QStringList ProfileStore::inputs()
{
    auto result = buttons;
    result.append({"left_trigger", "right_trigger"});
    for (const auto &stick :
         {QString("left_stick"), QString("right_stick"), QString("left_touchpad"), QString("right_touchpad")})
        for (const auto &direction : directions)
            result.append(stick + "_" + direction);
    for (const auto &sensor : {QString("accelerometer"), QString("gyro")})
        for (const auto &direction : {"left", "right", "up", "down", "forward", "backward"})
            result.append(sensor + "_" + direction);
    return result;
}
QStringList ProfileStore::namedKeys() { return keys.keys(); }
QString ProfileStore::validate(const QJsonObject &profile)
{
    if (!validId(profile.value("id").toString()))
        return "id must be a lowercase slug, at most 64 characters.";
    const auto name = profile.value("name").toString().trimmed();
    if (name.isEmpty() || name.size() > 120)
        return "name must contain 1 to 120 characters.";
    if (profile.value("schema_version").toInt() != 1)
        return "schema_version must be 1.";
    if (profile.value("controller").toString() != "sdl-gamecontroller")
        return "Use controller: sdl-gamecontroller.";
    if (profile.contains("controller_family") &&
        !QStringList{"generic", "playstation", "xbox", "steam-2015", "steam-2026"}.contains(
            profile.value("controller_family").toString()))
        return "controller_family must be generic, playstation, xbox, steam-2015, or steam-2026.";
    if (profile.value("description").toString().size() > 2000)
        return "description is too long.";
    if (!QStringList{"desktop", "browser", "game", "custom"}.contains(profile.value("category").toString()))
        return "category must be desktop, browser, game, or custom.";
    const int deadZone = profile.value("dead_zone").toInt(8000);
    if (deadZone < 1000 || deadZone > 30000)
        return "dead_zone must be between 1000 and 30000.";
    if (profile.contains("layouts"))
    {
        const auto layouts = profile.value("layouts").toArray();
        if (!profile.value("layouts").isArray() || layouts.isEmpty() || layouts.size() > 8)
            return "layouts must contain 1 to 8 named mapping sets.";
        if (!profile.value("bindings").isArray() || !profile.value("bindings").toArray().isEmpty())
            return "For a multi-layout profile, use empty top-level bindings and put bindings inside each layout.";
        QSet<int> sets;
        for (const auto &value : layouts)
        {
            const auto layout = value.toObject();
            const auto set = layout.value("set");
            if (!set.isDouble() || set.toDouble() != set.toInt() || set.toInt() < 1 || set.toInt() > 8 ||
                sets.contains(set.toInt()))
                return "Layout set numbers must be unique integers from 1 to 8.";
            if (layout.value("name").toString().trimmed().isEmpty() || layout.value("name").toString().size() > 30)
                return "Give each layout a name of 1 to 30 characters.";
            sets.insert(set.toInt());
            for (const auto &field : layout.keys())
                if (!QStringList{"set", "name", "bindings"}.contains(field))
                    return "Unknown layout field: " + field;
            auto single = profile;
            single.remove("layouts");
            single["bindings"] = layout.value("bindings");
            const auto error = validate(single);
            if (!error.isEmpty())
                return layout.value("name").toString() + ": " + error;
        }
        if (!sets.contains(1))
            return "Include set 1 as the initial layout.";
    }
    if (!profile.value("bindings").isArray() || profile.value("bindings").toArray().size() > inputs().size())
        return "bindings must be an array with at most one entry per supported input.";
    QSet<QString> seen;
    for (const auto &value : profile.value("bindings").toArray())
    {
        if (!value.isObject())
            return "Each binding must be an object.";
        const auto binding = value.toObject();
        const auto input = binding.value("input").toString();
        if (!inputs().contains(input) || seen.contains(input))
            return "Unsupported or duplicate input: " + input;
        seen.insert(input);
        if (binding.value("label").toString().size() > 120)
            return "Binding labels must be at most 120 characters.";
        int actions = 0;
        if (binding.contains("keys"))
        {
            ++actions;
            const auto chord = binding.value("keys").toArray();
            if (!binding.value("keys").isArray() || chord.isEmpty() || chord.size() > 4)
                return "keys must contain 1 to 4 keys, pressed together.";
            QSet<QString> chordSeen;
            for (const auto &key : chord)
            {
                if (!key.isString() || !keyCode(key.toString()) || chordSeen.contains(key.toString()))
                    return "Unsupported or duplicate keyboard key.";
                chordSeen.insert(key.toString());
            }
        }
        if (binding.contains("mouse_button"))
        {
            ++actions;
            const auto code = binding.value("mouse_button");
            if (!code.isDouble() || code.toDouble() != code.toInt() || code.toInt() < 1 || code.toInt() > 9)
                return "mouse_button must be an integer from 1 to 9.";
        }
        if (binding.contains("mouse_move"))
        {
            ++actions;
            if (!directions.contains(binding.value("mouse_move").toString()))
                return "Invalid mouse movement direction.";
        }
        if (actions != 1)
            return "Each binding needs exactly one of keys, mouse_button, or mouse_move.";
        const QSet<QString> fields = {"input", "label", "keys", "mouse_button", "mouse_move"};
        for (const auto &field : binding.keys())
            if (!fields.contains(field))
                return "Unknown binding field: " + field;
    }
    const QSet<QString> fields = {
        "id",        "name",     "description", "category", "controller", "controller_family", "schema_version",
        "dead_zone", "bindings", "layouts"};
    for (const auto &field : profile.keys())
        if (!fields.contains(field))
            return "Unknown profile field: " + field;
    return {};
}
QJsonObject ProfileStore::read(const QString &id) const
{
    if (!validId(id))
        return failure("Invalid profile id.");
    QJsonObject profile;
    bool bundled = id.startsWith("builtin-");
    if (bundled)
    {
        for (const auto &value : catalog())
            if (value.toObject().value("id").toString() == id)
                profile = value.toObject();
    } else
        profile = readFile(path(id));
    if (profile.isEmpty())
        return failure("Profile was not found.");
    const auto error = validate(profile);
    if (!error.isEmpty())
        return failure("Profile is invalid: " + error);
    return {{"profile", profile}, {"revision", revision(profile)}, {"bundled", bundled}};
}
QJsonArray ProfileStore::list() const
{
    QJsonArray result;
    QStringList ids;
    for (const auto &value : catalog())
        ids.append(value.toObject().value("id").toString());
    for (const auto &file : QDir(profileDirectory()).entryList({"*.json"}, QDir::Files))
        if (!file.startsWith("builtin-"))
            ids.append(QFileInfo(file).completeBaseName());
    for (const auto &id : ids)
    {
        const auto item = read(id);
        if (item.contains("error"))
            continue;
        auto summary = item.value("profile").toObject();
        int bindingCount = summary.value("bindings").toArray().size();
        for (const auto &value : summary.value("layouts").toArray())
            bindingCount += value.toObject().value("bindings").toArray().size();
        summary.insert("layout_count", summary.contains("layouts") ? summary.value("layouts").toArray().size() : 1);
        summary.remove("bindings");
        summary.remove("layouts");
        summary.insert("binding_count", bindingCount);
        summary.insert("revision", item.value("revision"));
        summary.insert("bundled", item.value("bundled"));
        result.append(summary);
    }
    return result;
}
QJsonObject ProfileStore::save(const QJsonObject &profile, const QString &expectedRevision) const
{
    const auto error = validate(profile);
    if (!error.isEmpty())
        return failure(error);
    const auto id = profile.value("id").toString();
    if (id.startsWith("builtin-"))
        return failure("Bundled profiles are read-only. Copy to a new personal id first.");
    if (!QDir().mkpath(profileDirectory()))
        return failure("Cannot create the profile library.");
    QLockFile lock(QDir(profileDirectory()).filePath("library.lock"));
    if (!lock.tryLock(0))
        return failure("Profile library is busy; retry shortly.");
    const bool exists = QFileInfo::exists(path(id));
    if (exists)
    {
        const auto current = read(id);
        if (current.contains("error"))
            return current;
        if (expectedRevision.isEmpty() || expectedRevision != current.value("revision").toString())
            return failure("Profile changed or expected_revision is missing. Read it again before saving.");
        const auto directory = QDir(profileDirectory()).filePath("revisions/" + id);
        if (!QDir().mkpath(directory) ||
            !writeFile(QDir(directory).filePath(expectedRevision + ".json"), current.value("profile").toObject()))
            return failure("Cannot preserve the previous revision; profile was not changed.");
    } else if (!expectedRevision.isEmpty())
        return failure("Profile no longer exists; expected_revision must be empty for creation.");
    if (!writeFile(path(id), profile))
        return failure("Could not atomically save the profile.");
    return read(id);
}
QJsonArray ProfileStore::revisions(const QString &id) const
{
    QJsonArray result;
    if (!validId(id))
        return result;
    const QDir directory(QDir(profileDirectory()).filePath("revisions/" + id));
    for (const auto &file : directory.entryInfoList({"*.json"}, QDir::Files, QDir::Time))
        result.append(QJsonObject{{"revision", file.completeBaseName()},
                                  {"saved_at", file.lastModified().toUTC().toString(Qt::ISODate)}});
    return result;
}
QJsonObject ProfileStore::restore(const QString &id, const QString &hash, const QString &expectedRevision) const
{
    if (!validId(id) || !QRegularExpression("^[a-f0-9]{64}$").match(hash).hasMatch())
        return failure("Invalid id or revision.");
    const auto old = readFile(QDir(profileDirectory()).filePath("revisions/" + id + "/" + hash + ".json"));
    if (old.isEmpty() || old.value("id").toString() != id || revision(old) != hash)
        return failure("Revision was not found or is damaged.");
    return save(old, expectedRevision);
}
QString ProfileStore::exportMapping(const QString &id) const
{
    const auto item = read(id);
    if (item.contains("error"))
        return {};
    const auto profile = item.value("profile").toObject();
    const QString directory = QDir(profileDirectory()).filePath("compiled/" + id);
    if (!QDir().mkpath(directory))
        return {};
    QByteArray bytes;
    QXmlStreamWriter xml(&bytes);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement("gamecontroller");
    xml.writeAttribute("configversion", "19");
    xml.writeAttribute("appversion", "3.6.1");
    xml.writeTextElement("profilename", profile.value("name").toString());
    xml.writeStartElement("sets");
    auto layouts = profile.value("layouts").toArray();
    if (layouts.isEmpty())
        layouts.append(QJsonObject{{"set", 1}, {"name", ""}, {"bindings", profile.value("bindings")}});
    for (const auto &layoutValue : layouts)
    {
        const auto layout = layoutValue.toObject();
        xml.writeStartElement("set");
        xml.writeAttribute("index", QString::number(layout.value("set").toInt()));
        if (!layout.value("name").toString().isEmpty())
            xml.writeTextElement("name", layout.value("name").toString());
        const auto bindings = layout.value("bindings").toArray();
        for (const auto &value : bindings)
        {
            const auto binding = value.toObject();
            const int index = buttons.indexOf(binding.value("input").toString());
            if (index >= 0 && (index < 11 || index >= 15))
                bindingXml(xml, binding, "button", index + 1);
        }
        xml.writeStartElement("dpad");
        xml.writeAttribute("index", "1");
        for (int direction = 0; direction < directions.size(); ++direction)
        {
            const auto input = "dpad_" + directions.at(direction);
            for (const auto &value : bindings)
                if (value.toObject().value("input").toString() == input)
                    bindingXml(xml, value.toObject(), "dpadbutton", 1 << direction);
        }
        xml.writeEndElement();
        for (int trigger = 0; trigger < 2; ++trigger)
        {
            const auto input = QString(trigger == 0 ? "left_trigger" : "right_trigger");
            for (const auto &value : bindings)
                if (value.toObject().value("input").toString() == input)
                {
                    xml.writeStartElement("trigger");
                    xml.writeAttribute("index", QString::number(trigger + 5));
                    xml.writeTextElement("deadZone", "8000");
                    xml.writeTextElement("throttle", "positivehalf");
                    bindingXml(xml, value.toObject(), "triggerbutton", 2);
                    xml.writeEndElement();
                }
        }
        for (int stick = 0; stick < 4; ++stick)
        {
            const QStringList prefixes = {"left_stick_", "right_stick_", "left_touchpad_", "right_touchpad_"};
            bool assigned = false;
            for (const auto &value : bindings)
                if (value.toObject().value("input").toString().startsWith(prefixes.at(stick)))
                    assigned = true;
            if (stick >= 2 && !assigned)
                continue;
            xml.writeStartElement("stick");
            xml.writeAttribute("index", QString::number(stick + 1));
            xml.writeTextElement("deadZone", QString::number(profile.value("dead_zone").toInt(8000)));
            for (int direction = 0; direction < directions.size(); ++direction)
            {
                const auto input = prefixes.at(stick) + directions.at(direction);
                for (const auto &value : bindings)
                    if (value.toObject().value("input").toString() == input)
                        bindingXml(xml, value.toObject(), "stickbutton", 1 << direction);
            }
            xml.writeEndElement();
        }
        for (int sensor = 0; sensor < 2; ++sensor)
        {
            const auto prefix = QString(sensor == 0 ? "accelerometer_" : "gyro_");
            const QStringList sensorDirections = {"left", "right", "up", "down", "forward", "backward"};
            bool assigned = false;
            for (const auto &value : bindings)
                if (value.toObject().value("input").toString().startsWith(prefix))
                    assigned = true;
            if (!assigned)
                continue;
            xml.writeStartElement("sensor");
            xml.writeAttribute("type", QString::number(sensor));
            for (int direction = 0; direction < sensorDirections.size(); ++direction)
                for (const auto &value : bindings)
                    if (value.toObject().value("input").toString() == prefix + sensorDirections.at(direction))
                        bindingXml(xml, value.toObject(), "sensorbutton", 1 << direction);
            xml.writeEndElement();
        }
        xml.writeEndElement();
    }
    xml.writeEndElement();
    xml.writeEndElement();
    xml.writeEndDocument();
    if (xml.hasError())
        return {};
    // Include the compiler output hash. Keep every exported revision immutable, including old manually edited files.
    const auto hash = QString::fromLatin1(QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex());
    const auto filePath = QDir(directory).filePath(item.value("revision").toString() + "-" + hash + ".amgp");
    QFile existing(filePath);
    if (existing.exists())
        return existing.open(QIODevice::ReadOnly) && existing.readAll() == bytes ? filePath : QString{};
    QSaveFile file(filePath);
    return file.open(QIODevice::WriteOnly) && file.write(bytes) == bytes.size() && file.commit() ? filePath : QString{};
}
} // namespace Troa
