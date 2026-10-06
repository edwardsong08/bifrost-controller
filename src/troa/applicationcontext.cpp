// SPDX-License-Identifier: GPL-3.0-or-later
#include "applicationcontext.h"
#include "antimicrosettings.h"
#include "gui/joytabwidget.h"
#include "identity.h"
#include "inputdevice.h"
#include "joybuttontypes/joybutton.h"
#include "profilestore.h"
#include "setjoystick.h"

#include <QApplication>
#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QKeySequence>
#include <QLabel>
#include <QRegularExpression>
#include <QScreen>
#include <QThread>
#include <QTimer>
#include <QVBoxLayout>
#include <QXmlStreamReader>
#include <memory>
#ifdef Q_OS_WIN
    #include "winextras.h"
    #include <windows.h>
#endif

namespace {
QString canonical(const QString &path)
{
    return QDir::cleanPath(QDir::fromNativeSeparators(QFileInfo(path).absoluteFilePath())).toCaseFolded();
}
QString foregroundExecutable()
{
#ifdef Q_OS_WIN
    return WinExtras::getForegroundWindowExePath();
#else
    return {};
#endif
}
QString modeName(InputDevice *device, const QJsonObject &rule, int index)
{
    for (const auto &value : rule.value("modes").toArray())
    {
        const auto mode = value.toObject();
        if (mode.value("set").toInt() == index + 1)
            return mode.value("name").toString();
    }
    const auto name = device->getSetJoystick(index)->getName();
    return name.isEmpty() ? QString("Layout %1").arg(index + 1) : name;
}
bool unusedButton(InputDevice *device, const QJsonObject &rule)
{
    const int index = rule.value("controller_button").toInt(-1);
    if (index < 0)
        return false;
    // Never steal an input already mapped by the profile, including virtual D-pad use.
    for (const auto &value : rule.value("modes").toArray())
    {
        auto button = device->getSetJoystick(value.toObject().value("set").toInt() - 1)->getJoyButton(index);
        if (!button || !button->getAssignedSlots()->isEmpty() || button->getVDPad() ||
            button->getChangeSetCondition() != JoyButton::SetChangeDisabled)
            return false;
    }
    return true;
}
#ifdef Q_OS_WIN
BOOL CALLBACK collectWindows(HWND window, LPARAM parameter)
{
    if (!IsWindowVisible(window) || GetWindow(window, GW_OWNER) || GetWindowTextLengthW(window) == 0)
        return TRUE;
    DWORD pid = 0;
    GetWindowThreadProcessId(window, &pid);
    if (pid == GetCurrentProcessId())
        return TRUE;
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!process)
        return TRUE;
    wchar_t path[32768];
    DWORD length = 32768;
    if (QueryFullProcessImageNameW(process, 0, path, &length))
    {
        auto paths = reinterpret_cast<QSet<QString> *>(parameter);
        paths->insert(canonical(QString::fromWCharArray(path, int(length))));
    }
    CloseHandle(process);
    return TRUE;
}
bool shortcutParts(const QString &text, UINT *modifiers, UINT *key)
{
    if (text.isEmpty())
        return true;
    // A single chord, no system-reserved Windows modifier or F12 key.
    const auto sequence = QKeySequence::fromString(text, QKeySequence::PortableText);
    if (sequence.count() != 1)
        return false;
    #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const int combined = sequence[0].toCombined();
    #else
    const int combined = sequence[0];
    #endif
    const int flags = combined & int(Qt::KeyboardModifierMask);
    if (flags & ~(int(Qt::ControlModifier) | int(Qt::AltModifier) | int(Qt::ShiftModifier)))
        return false;
    const int code = combined & ~int(Qt::KeyboardModifierMask);
    if (code >= Qt::Key_A && code <= Qt::Key_Z)
        *key = UINT('A' + code - Qt::Key_A);
    else if (code >= Qt::Key_0 && code <= Qt::Key_9)
        *key = UINT('0' + code - Qt::Key_0);
    else if (code >= Qt::Key_F1 && code <= Qt::Key_F11)
        *key = UINT(VK_F1 + code - Qt::Key_F1);
    else
        return false;
    *modifiers = MOD_NOREPEAT;
    if (flags & Qt::ControlModifier)
        *modifiers |= MOD_CONTROL;
    if (flags & Qt::AltModifier)
        *modifiers |= MOD_ALT;
    if (flags & Qt::ShiftModifier)
        *modifiers |= MOD_SHIFT;
    return true;
}
#endif
} // namespace

namespace Troa {
ApplicationContext::ApplicationContext(AntiMicroSettings *settings, std::function<QList<JoyTabWidget *>()> tabs,
                                       QObject *parent)
    : QObject(parent)
    , m_settings(settings)
    , m_tabs(std::move(tabs))
{
    const auto saved = QJsonDocument::fromJson(settings->value("TROA/ApplicationRules").toByteArray()).array();
    for (const auto &value : saved)
    {
        const auto rule = value.toObject();
        const auto modes = rule.value("modes").toArray();
        bool valid = !rule.value("id").toString().isEmpty() && !rule.value("executable").toString().isEmpty() &&
                     !rule.value("controller_id").toString().isEmpty() && !modes.isEmpty() && modes.size() <= 8;
        for (const auto &mode : modes)
            valid = valid && mode.toObject().value("set").toInt() >= 1 && mode.toObject().value("set").toInt() <= 8;
        if (valid)
            m_rules.append(rule);
    }
    qApp->installNativeEventFilter(this);
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &ApplicationContext::poll);
    timer->start(500);
}
ApplicationContext::~ApplicationContext()
{
    qApp->removeNativeEventFilter(this);
#ifdef Q_OS_WIN
    for (const auto id : m_hotkeys.keys())
        UnregisterHotKey(nullptr, id);
#endif
    for (auto widget : m_notices)
        if (widget)
            delete widget;
}
QString ApplicationContext::revision() const
{
    return QString::fromLatin1(
        QCryptographicHash::hash(QJsonDocument(m_rules).toJson(QJsonDocument::Compact), QCryptographicHash::Sha256).toHex());
}
QJsonObject ApplicationContext::rules() const { return {{"rules", m_rules}, {"revision", revision()}}; }
void ApplicationContext::persist()
{
    m_settings->setValue("TROA/ApplicationRules", QJsonDocument(m_rules).toJson(QJsonDocument::Compact));
    m_settings->sync();
    m_lastApplied.clear();
    m_rememberedModes.clear();
    ++m_focusGeneration;
    registerShortcuts();
    poll();
    emit changed();
}
QStringList ApplicationContext::profileModes(const QString &path, QString *error)
{
    QStringList modes;
    for (int i = 1; i <= 8; ++i)
        modes.append(QString("Layout %1").arg(i));
    QFile file(path);
    const QString suffix = QFileInfo(path).suffix().toLower();
    if ((suffix != "amgp" && suffix != "xml") || file.size() > 8 * 1024 * 1024 || !file.open(QIODevice::ReadOnly))
    {
        if (error)
            *error = "Choose an existing .amgp or .xml controller profile (up to 8 MiB).";
        return modes;
    }
    QXmlStreamReader reader(&file);
    if (!reader.readNextStartElement() ||
        (reader.name() != QStringLiteral("gamecontroller") && reader.name() != QStringLiteral("joystick")))
    {
        if (error)
            *error = "This file is not a native controller profile.";
        return modes;
    }
    while (reader.readNextStartElement())
    {
        if (reader.name() != QStringLiteral("sets"))
        {
            reader.skipCurrentElement();
            continue;
        }
        while (reader.readNextStartElement())
        {
            if (reader.name() != QStringLiteral("set"))
            {
                reader.skipCurrentElement();
                continue;
            }
            const int set = reader.attributes().value("index").toInt() - 1;
            while (reader.readNextStartElement())
            {
                if (reader.name() == QStringLiteral("name") && set >= 0 && set < 8)
                    modes[set] = reader.readElementText();
                else
                    reader.skipCurrentElement();
            }
        }
    }
    while (!reader.atEnd())
        reader.readNext();
    if (reader.hasError() && error)
        *error = "The controller profile contains invalid XML: " + reader.errorString();
    return modes;
}
QJsonObject ApplicationContext::saveRule(QJsonObject rule, const QString &expectedRevision)
{
#ifndef Q_OS_WIN
    return failure("Application rules and keyboard shortcuts are currently available on Windows only.");
#else
    if (expectedRevision != revision())
        return failure("Application rules changed. Read them again before saving.");
    const auto id = rule.value("id").toString();
    if (!QRegularExpression("^[a-z0-9][a-z0-9-]{0,63}$").match(id).hasMatch())
        return failure("Use a short lowercase id containing letters, numbers, and hyphens.");
    if (rule.value("name").toString().trimmed().isEmpty() || rule.value("name").toString().size() > 120)
        return failure("Give this application a name (up to 120 characters).");
    const auto exe = QFileInfo(rule.value("executable").toString());
    if (!exe.isFile() || exe.suffix().compare("exe", Qt::CaseInsensitive) != 0)
        return failure("Choose the application's executable .exe file.");
    const auto controller = rule.value("controller_id").toString();
    JoyTabWidget *target = nullptr;
    for (auto tab : m_tabs())
        if (tab->getJoystick()->getStringIdentifier() == controller)
            target = tab;
    if (!target)
        return failure("Connect and choose the controller for this application.");
    QString error;
    profileModes(rule.value("profile_path").toString(), &error);
    if (!error.isEmpty())
        return failure(error);
    const auto modes = rule.value("modes").toArray();
    QSet<int> sets;
    if (modes.isEmpty() || modes.size() > 8)
        return failure("Choose between one and eight layouts.");
    for (const auto &value : modes)
    {
        const auto mode = value.toObject();
        const int set = mode.value("set").toInt();
        if (set < 1 || set > 8 || sets.contains(set) || mode.value("name").toString().trimmed().isEmpty() ||
            mode.value("name").toString().size() > 60)
            return failure("Each layout needs a unique set number (1–8) and a name.");
        sets.insert(set);
    }
    const int button = rule.value("controller_button").toInt(-1);
    if (button < -1 || button >= target->getJoystick()->getNumberButtons())
        return failure("Choose an available controller button.");
    if (button >= 0 && !target->getJoystick()->isGameController())
        return failure("Controller switching currently needs an SDL standard controller layout.");
    UINT modifiers = 0, key = 0;
    if (!shortcutParts(rule.value("keyboard_shortcut").toString(), &modifiers, &key))
        return failure("Use a single A–Z, 0–9, or F1–F11 shortcut, optionally with Ctrl, Alt, or Shift.");
    for (const auto &value : m_rules)
    {
        const auto other = value.toObject();
        if (other.value("id").toString() != id && other.value("controller_id").toString() == controller &&
            canonical(other.value("executable").toString()) == canonical(exe.absoluteFilePath()))
            return failure("This controller already has a rule for this application. Edit that rule instead.");
    }
    rule["executable"] = exe.absoluteFilePath();
    rule["profile_path"] = QFileInfo(rule.value("profile_path").toString()).absoluteFilePath();
    rule["keyboard_shortcut"] =
        QKeySequence::fromString(rule.value("keyboard_shortcut").toString(), QKeySequence::PortableText)
            .toString(QKeySequence::PortableText);
    bool replaced = false;
    for (int i = 0; i < m_rules.size(); ++i)
        if (m_rules.at(i).toObject().value("id").toString() == id)
        {
            m_rules.replace(i, rule);
            replaced = true;
            break;
        }
    if (!replaced)
    {
        if (m_rules.size() >= 100)
            return failure("The application rule limit is 100.");
        m_rules.append(rule);
    }
    persist();
    return rules();
#endif
}
QJsonObject ApplicationContext::removeRule(const QString &id, const QString &expectedRevision)
{
    if (expectedRevision != revision())
        return failure("Application rules changed. Read them again before removing a rule.");
    for (int i = 0; i < m_rules.size(); ++i)
        if (m_rules.at(i).toObject().value("id").toString() == id)
        {
            m_rules.removeAt(i);
            persist();
            return rules();
        }
    return failure("Application rule not found.");
}
QJsonObject ApplicationContext::matchingRule(const QString &controller) const
{
    if (m_executable.isEmpty())
        return {};
    for (const auto &value : m_rules)
    {
        const auto rule = value.toObject();
        if (rule.value("controller_id").toString() == controller &&
            canonical(rule.value("executable").toString()) == canonical(m_executable))
            return rule;
    }
    return {};
}
bool ApplicationContext::ownsController(const QString &controller) const
{
    // Read the actual foreground at dispatch time, not the previous polling result.
    const auto exe = foregroundExecutable();
    if (exe.isEmpty())
        return false;
    for (const auto &value : m_rules)
    {
        const auto rule = value.toObject();
        if (rule.value("controller_id").toString() == controller &&
            canonical(rule.value("executable").toString()) == canonical(exe))
            return true;
    }
    return false;
}
void ApplicationContext::registerShortcuts()
{
#ifdef Q_OS_WIN
    for (const auto id : m_hotkeys.keys())
        UnregisterHotKey(nullptr, id);
    m_hotkeys.clear();
    m_shortcutErrors.clear();
    if (m_mapperFocused)
        return;
    QSet<QString> seen;
    int id = 0x5450;
    for (const auto &value : m_rules)
    {
        const auto rule = value.toObject();
        const auto text = rule.value("keyboard_shortcut").toString();
        if (text.isEmpty() || canonical(rule.value("executable").toString()) != canonical(m_executable) ||
            seen.contains(text))
            continue;
        seen.insert(text);
        UINT modifiers = 0, key = 0;
        if (!shortcutParts(text, &modifiers, &key) || !RegisterHotKey(nullptr, id, modifiers, key))
            m_shortcutErrors[text] = "Shortcut unavailable: another app or Windows may be using it.";
        else
            m_hotkeys[id++] = text;
    }
#endif
}
void ApplicationContext::watch(JoyTabWidget *tab)
{
    auto device = tab->getJoystick();
    if (!m_devices.contains(device))
    {
        m_devices.insert(device);
        connect(device, &QObject::destroyed, this, [this, device]() {
            m_devices.remove(device);
            m_lastApplied.clear();
        });
    }
    // Use normalized SDL buttons from the engine, not hardware-specific raw button numbers.
    for (int set = 0; set < 8; ++set)
        for (auto button : device->getSetJoystick(set)->getButtons())
        {
            if (m_buttons.contains(button))
                continue;
            m_buttons.insert(button);
            auto pressedRule = std::make_shared<QString>();
            auto pressedGeneration = std::make_shared<int>(-1);
            const QPointer<InputDevice> guarded(device);
            connect(button, &QObject::destroyed, this, [this, button]() { m_buttons.remove(button); });
            connect(
                button, &JoyButton::clicked, this,
                [this, guarded, set, pressedRule, pressedGeneration](int index) {
                    pressedRule->clear();
                    if (!guarded || m_loading || m_mapperFocused || guarded->getActiveSetNumber() != set)
                        return;
                    const auto rule = matchingRule(guarded->getStringIdentifier());
                    if (rule.value("controller_button").toInt(-1) == index && unusedButton(guarded, rule) &&
                        ownsController(guarded->getStringIdentifier()))
                    {
                        *pressedRule = rule.value("id").toString();
                        *pressedGeneration = m_focusGeneration;
                    }
                },
                Qt::QueuedConnection);
            connect(
                button, &JoyButton::released, this,
                [this, guarded, set, pressedRule, pressedGeneration](int) {
                    const auto id = *pressedRule;
                    pressedRule->clear();
                    if (id.isEmpty() || !guarded || *pressedGeneration != m_focusGeneration ||
                        guarded->getActiveSetNumber() != set)
                        return;
                    const auto rule = matchingRule(guarded->getStringIdentifier());
                    if (rule.value("id").toString() == id)
                        cycle(guarded, rule, true);
                },
                Qt::QueuedConnection);
        }
}
QString ApplicationContext::cycle(InputDevice *device, const QJsonObject &rule, bool controllerButton)
{
    if (m_loading || rule.isEmpty() || m_mapperFocused || !ownsController(device->getStringIdentifier()))
        return {};
    JoyTabWidget *tab = nullptr;
    for (auto candidate : m_tabs())
        if (candidate->getJoystick() == device)
            tab = candidate;
    if (!tab || canonical(tab->currentProfilePath()) != canonical(rule.value("profile_path").toString()))
        return {};
    if (controllerButton && !unusedButton(device, rule))
        return "Switch button is already assigned. Clear its actions in every selected layout.";
    const auto modes = rule.value("modes").toArray();
    if (modes.size() < 2)
        return {};
    int next = 0;
    for (int i = 0; i < modes.size(); ++i)
        if (modes.at(i).toObject().value("set").toInt() == device->getActiveSetNumber() + 1)
            next = (i + 1) % modes.size();
    const int set = modes.at(next).toObject().value("set").toInt() - 1;
    // The device owns release/held-input handling. Dispatch in its thread like other engine operations.
    QMetaObject::invokeMethod(
        device, "setActiveSetNumber",
        device->thread() == QThread::currentThread() ? Qt::DirectConnection : Qt::BlockingQueuedConnection, Q_ARG(int, set));
    poll();
    return {};
}
void ApplicationContext::poll()
{
#ifdef Q_OS_WIN
    DWORD pid = 0;
    GetWindowThreadProcessId(GetForegroundWindow(), &pid);
    const bool mapperFocused = pid == GetCurrentProcessId();
    const auto exe = foregroundExecutable();
    const bool focusChanged = mapperFocused != m_mapperFocused || (!mapperFocused && exe != m_executable);
    m_mapperFocused = mapperFocused;
    if (!mapperFocused)
    {
        m_executable = exe;
        m_title = WinExtras::getCurrentWindowText();
    }
    if (focusChanged)
    {
        ++m_focusGeneration;
        m_lastApplied.clear();
        registerShortcuts();
    }
#endif
    for (auto tab : m_tabs())
    {
        auto device = tab->getJoystick();
        watch(tab);
        const auto controller = device->getStringIdentifier();
        const auto rule = matchingRule(controller);
        const auto id = rule.value("id").toString();
        const auto path = rule.value("profile_path").toString();
        if (!m_mapperFocused && !rule.isEmpty() && m_lastApplied.value(controller) != id)
        {
            if (device->isDeviceEdited())
                m_messages[controller] = "Switch paused: save or revert your controller edits.";
            else if (!QFileInfo::exists(path))
                m_messages[controller] = "Assigned profile is missing. Edit this application rule.";
            else
            {
                QString error;
                profileModes(path, &error);
                if (!error.isEmpty())
                    m_messages[controller] = error;
                else
                {
                    m_loading = true;
                    if (canonical(tab->currentProfilePath()) != canonical(path))
                        tab->loadConfigFile(path);
                    m_loading = false;
                    if (canonical(tab->currentProfilePath()) != canonical(path))
                        m_messages[controller] = "Profile could not be loaded.";
                    else
                    {
                        m_lastApplied[controller] = id;
                        m_messages.remove(controller);
                        const int set = m_rememberedModes.value(
                            id, rule.value("modes").toArray().first().toObject().value("set").toInt() - 1);
                        QMetaObject::invokeMethod(device, "setActiveSetNumber",
                                                  device->thread() == QThread::currentThread()
                                                      ? Qt::DirectConnection
                                                      : Qt::BlockingQueuedConnection,
                                                  Q_ARG(int, set));
                    }
                }
            }
        }
        const int set = device->getActiveSetNumber();
        const auto observed = id + ":" + tab->currentProfilePath() + ":" + QString::number(set);
        if (!m_mapperFocused && !rule.isEmpty() && canonical(tab->currentProfilePath()) == canonical(path))
        {
            if (m_observed.value(controller) != observed)
                notice(rule.value("name").toString() + " · " + modeName(device, rule, set),
                       device->getSDLName() + "  •  " + tab->getCurrentConfigName());
            m_rememberedModes[id] = set;
        }
        m_observed[controller] = observed;
    }
    emit changed();
}
QJsonObject ApplicationContext::state() const
{
    QJsonArray controllers, applications;
    QSet<QString> openPaths;
#ifdef Q_OS_WIN
    EnumWindows(collectWindows, reinterpret_cast<LPARAM>(&openPaths));
#endif
    for (auto tab : m_tabs())
    {
        auto device = tab->getJoystick();
        const auto controller = device->getStringIdentifier();
        const auto rule = matchingRule(controller);
        QJsonArray buttons;
        if (device->isGameController())
            for (auto button : device->getActiveSetJoystick()->getButtons())
                buttons.append(
                    QJsonObject{{"index", button->getJoyNumber()}, {"name", button->getPartialName(true, false)}});
        const bool matches =
            !rule.isEmpty() && canonical(tab->currentProfilePath()) == canonical(rule.value("profile_path").toString());
        QString message =
            rule.isEmpty() ? "No application rule; the current controller profile continues." : m_messages.value(controller);
        if (matches && rule.value("controller_button").toInt(-1) >= 0 && !unusedButton(device, rule))
            message = "Controller shortcut paused: its button has an action in a selected layout. Clear those actions to "
                      "reserve it.";
        if (!rule.value("keyboard_shortcut").toString().isEmpty() &&
            m_shortcutErrors.contains(rule.value("keyboard_shortcut").toString()))
            message += " " + m_shortcutErrors.value(rule.value("keyboard_shortcut").toString());
        controllers.append(QJsonObject{
            {"controller_id", controller},
            {"controller", device->getSDLName()},
            {"application", rule.value("name").toString(QFileInfo(m_executable).fileName())},
            {"assigned_profile",
             rule.isEmpty() ? "No rule" : QFileInfo(rule.value("profile_path").toString()).completeBaseName()},
            {"active_profile", tab->currentProfilePath().isEmpty() ? "No saved profile" : tab->getCurrentConfigName()},
            {"profile_path", tab->currentProfilePath()},
            {"controller_buttons", buttons},
            {"active_mode", modeName(device, matches ? rule : QJsonObject(), device->getActiveSetNumber())},
            {"active_set", device->getActiveSetNumber() + 1},
            {"assignment_active", matches},
            {"message", message.trimmed()}});
    }
    for (const auto &value : m_rules)
    {
        auto rule = value.toObject();
        const auto path = canonical(rule.value("executable").toString());
        rule["focus_state"] = path == canonical(m_executable) ? (m_mapperFocused ? "Last focused" : "Focused")
                                                              : (openPaths.contains(path) ? "Open" : "Closed");
        applications.append(rule);
    }
    return {{"executable", m_executable},
            {"window_title", m_title},
            {"mapper_focused", m_mapperFocused},
            {"controllers", controllers},
            {"applications", applications}};
}
void ApplicationContext::notice(const QString &title, const QString &detail)
{
    if (!m_settings->value("TROA/SwitchNotifications", true).toBool())
        return;
    const auto placement = m_settings->value("TROA/NotificationDisplay", "focused").toString();
    QList<QScreen *> screens;
    QScreen *screen = qApp->primaryScreen();
#ifdef Q_OS_WIN
    MONITORINFOEXW info{};
    info.cbSize = sizeof(info);
    const auto monitor = MonitorFromWindow(GetForegroundWindow(), MONITOR_DEFAULTTOPRIMARY);
    if (GetMonitorInfoW(monitor, reinterpret_cast<LPMONITORINFO>(&info)))
        for (auto candidate : qApp->screens())
            if (candidate->name().compare(QString::fromWCharArray(info.szDevice), Qt::CaseInsensitive) == 0)
                screen = candidate;
#endif
    if (placement == "all")
        screens = qApp->screens();
    else if (placement == "primary")
        screens.append(qApp->primaryScreen());
    else
        screens.append(screen);
    for (auto widget : m_notices)
        if (widget)
            delete widget;
    m_notices.clear();
    for (auto display : screens)
    {
        if (!display)
            continue;
        auto widget = new QWidget(nullptr, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
                                               Qt::WindowDoesNotAcceptFocus | Qt::WindowTransparentForInput);
        widget->setAttribute(Qt::WA_ShowWithoutActivating);
        widget->setAttribute(Qt::WA_DeleteOnClose);
        widget->setObjectName("troaSwitchNotice");
        widget->setStyleSheet(
            "QWidget#troaSwitchNotice { background: #111722; border: 2px solid #d4a84f; border-radius: 12px; }"
            "QLabel { color: #f4f0e8; background: transparent; font-size: 14px; }"
            "QLabel#noticeTitle { color: #ffda83; font-size: 20px; font-weight: 600; }");
        auto layout = new QVBoxLayout(widget);
        layout->setContentsMargins(22, 16, 22, 16);
        auto heading = new QLabel(title);
        heading->setObjectName("noticeTitle");
        heading->setTextFormat(Qt::PlainText);
        heading->setWordWrap(true);
        auto description = new QLabel(detail);
        description->setTextFormat(Qt::PlainText);
        description->setWordWrap(true);
        layout->addWidget(heading);
        layout->addWidget(description);
        const auto available = display->availableGeometry();
        widget->setFixedWidth(qMin(420, available.width() - 32));
        widget->adjustSize();
        widget->move(available.right() - widget->width() - 20, available.top() + 24);
        widget->show();
        m_notices.append(widget);
        QTimer::singleShot(4500, widget, &QWidget::close);
    }
}
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
bool ApplicationContext::nativeEventFilter(const QByteArray &, void *message, qintptr *)
#else
bool ApplicationContext::nativeEventFilter(const QByteArray &, void *message, long *)
#endif
{
#ifdef Q_OS_WIN
    const auto event = static_cast<MSG *>(message);
    if (event->message == WM_HOTKEY && m_hotkeys.contains(int(event->wParam)))
    {
        const auto shortcut = m_hotkeys.value(int(event->wParam));
        for (auto tab : m_tabs())
        {
            const auto rule = matchingRule(tab->getJoystick()->getStringIdentifier());
            if (rule.value("keyboard_shortcut").toString() == shortcut)
                cycle(tab->getJoystick(), rule, false);
        }
        return true;
    }
#else
    Q_UNUSED(message)
#endif
    return false;
}
} // namespace Troa
