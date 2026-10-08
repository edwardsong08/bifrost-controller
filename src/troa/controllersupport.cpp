// SPDX-License-Identifier: GPL-3.0-or-later
#include "controllersupport.h"
#include "gamecontroller/gamecontroller.h"
#include "joycontrolstick.h"
#include "joysensortype.h"
#include "setjoystick.h"
#include <QJsonObject>
#include <QMap>

namespace Troa {
QStringList buttonInputs()
{
    QStringList result = {"a",
                          "b",
                          "x",
                          "y",
                          "back",
                          "guide",
                          "start",
                          "left_stick_press",
                          "right_stick_press",
                          "left_shoulder",
                          "right_shoulder",
                          "dpad_up",
                          "dpad_down",
                          "dpad_left",
                          "dpad_right",
                          "misc1",
                          "paddle1",
                          "paddle2",
                          "paddle3",
                          "paddle4",
                          "touchpad_press"};
    for (int i = 0; i < 64; ++i)
        result.append(QString("raw_button_%1").arg(i));
    result.append({"left_touchpad_click", "right_touchpad_click"});
    return result;
}
QString buttonInput(int index) { return buttonInputs().value(index); }
int inputButton(const QString &input) { return buttonInputs().indexOf(input); }
QString controllerFamily(InputDevice *device)
{
    if (!device)
        return "generic";
    const auto name = device->getSDLName().toLower();
    if (name.contains("steam controller"))
    {
        auto pad = qobject_cast<GameController *>(device);
        const int buttons = pad ? SDL_JoystickNumButtons(SDL_GameControllerGetJoystick(pad->getController())) : 0;
        return buttons >= 22 ? "steam-2026" : "steam-2015";
    }
    if (name.contains("dualsense") || name.contains("dualshock") || name.contains("ps4") || name.contains("ps5"))
        return "playstation";
    if (name.contains("xbox") || name.contains("xinput"))
        return "xbox";
    return "generic";
}
QString inputName(const QString &input, const QString &family)
{
    const QMap<QString, QString> ps = {{"a", "Cross (×)"},         {"b", "Circle (○)"},         {"x", "Square (□)"},
                                       {"y", "Triangle (△)"},      {"back", "Create / Share"},  {"start", "Options"},
                                       {"guide", "PS button"},     {"left_shoulder", "L1"},     {"right_shoulder", "R1"},
                                       {"left_stick_press", "L3"}, {"right_stick_press", "R3"}, {"left_trigger", "L2"},
                                       {"right_trigger", "R2"}};
    const QMap<QString, QString> xbox = {{"back", "View / Back"}, {"start", "Menu / Start"}, {"guide", "Xbox button"},
                                         {"left_shoulder", "LB"}, {"right_shoulder", "RB"},  {"left_trigger", "LT"},
                                         {"right_trigger", "RT"}, {"misc1", "Share"}};
    const QMap<QString, QString> steam = {{"guide", "Steam button"},
                                          {"back", "Back / Menu"},
                                          {"start", "Start / View"},
                                          {"left_shoulder", "Left bumper"},
                                          {"right_shoulder", "Right bumper"},
                                          {"misc1", "Quick access"},
                                          {"paddle1", "Right upper grip (R4)"},
                                          {"paddle2", "Left upper grip (L4)"},
                                          {"paddle3", "Right lower grip (R5)"},
                                          {"paddle4", "Left lower grip (L5)"},
                                          {"raw_button_18", "Right stick touch"},
                                          {"raw_button_19", "Left stick touch"},
                                          {"raw_button_20", "Right grip touch"},
                                          {"raw_button_21", "Left grip touch"}};
    if (family == "playstation" && ps.contains(input))
        return ps.value(input);
    if (family == "xbox" && xbox.contains(input))
        return xbox.value(input);
    if (family.startsWith("steam-") && steam.contains(input))
    {
        if (family == "steam-2015" && input == "paddle1")
            return "Right grip";
        if (family == "steam-2015" && input == "paddle2")
            return "Left grip";
        return steam.value(input);
    }
    if (family == "steam-2015")
    {
        if (input.startsWith("right_stick_"))
            return inputName(QString(input).replace("right_stick_", "right_touchpad_"), "generic");
    }
    if (input == "a" || input == "b" || input == "x" || input == "y")
        return input.toUpper();
    QString name = input;
    name.replace("dpad_", "D-pad ");
    name.replace("_stick_press", " stick click");
    name.replace('_', ' ');
    if (!name.isEmpty())
        name[0] = name.at(0).toUpper();
    return name;
}
QStringList availableInputs(InputDevice *device)
{
    QStringList result;
    auto pad = qobject_cast<GameController *>(device);
    if (!pad)
        return result;
    for (int i = 0; i < pad->getNumberRawButtons(); ++i)
        if (pad->supportsButton(i))
            result.append(buttonInput(i));
    for (int i = 0; i < 2; ++i)
    {
        if (SDL_GameControllerHasAxis(pad->getController(), SDL_GameControllerAxis(SDL_CONTROLLER_AXIS_TRIGGERLEFT + i)))
            result.append(i == 0 ? "left_trigger" : "right_trigger");
        if (SDL_GameControllerHasAxis(pad->getController(), SDL_GameControllerAxis(i * 2)) &&
            SDL_GameControllerHasAxis(pad->getController(), SDL_GameControllerAxis(i * 2 + 1)))
            for (const auto &direction : {"up", "right", "down", "left"})
                result.append(QString(i == 0 ? "left_stick_" : "right_stick_") + direction);
    }
    for (int i = 0; i < pad->touchpadCount(); ++i)
        for (const auto &direction : {"up", "right", "down", "left"})
            result.append(QString(i == 0 ? "left_touchpad_" : "right_touchpad_") + direction);
    for (const auto &sensor : {QString("accelerometer"), QString("gyro")})
        if (device->hasRawSensor(sensor == "gyro" ? GYROSCOPE : ACCELEROMETER))
            for (const auto &direction : {"left", "right", "up", "down", "forward", "backward"})
                result.append(sensor + "_" + direction);
    return result;
}
QString profileCompatibility(InputDevice *device, const QJsonObject &profile)
{
    const auto wantedFamily = profile.value("controller_family").toString("generic");
    const auto family = controllerFamily(device);
    // Family-specific touchpad layouts must not silently become a different controller's mappings.
    if (wantedFamily.startsWith("steam-") && wantedFamily != family)
        return "Choose the matching physical Steam Controller model for this template.";
    auto layouts = profile.value("layouts").toArray();
    if (layouts.isEmpty())
        layouts.append(QJsonObject{{"bindings", profile.value("bindings")}});
    const auto available = availableInputs(device);
    QStringList missing;
    for (const auto &layout : layouts)
        for (const auto &value : layout.toObject().value("bindings").toArray())
        {
            const auto input = value.toObject().value("input").toString();
            if (!available.contains(input) && !missing.contains(input))
                missing.append(inputName(input, family));
        }
    return missing.isEmpty()
               ? QString{}
               : "This controller/driver does not expose: " + missing.join(", ") +
                     ". Keep the current profile and choose a compatible template or configure the controller layout.";
}
} // namespace Troa
