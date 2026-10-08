// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>

class InputDevice;
namespace Troa {
// SDL2's standardized indices stay unchanged for existing native profiles.
constexpr int RawButtonBase = 21;
constexpr int TouchpadClickBase = 85;
constexpr int TouchpadAxisBase = 6;
QString controllerFamily(InputDevice *device);
QString inputName(const QString &input, const QString &family);
QString buttonInput(int index);
int inputButton(const QString &input);
QStringList buttonInputs();
QStringList availableInputs(InputDevice *device);
QString profileCompatibility(InputDevice *device, const QJsonObject &profile);
} // namespace Troa
