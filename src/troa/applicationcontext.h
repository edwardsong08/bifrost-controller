// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QAbstractNativeEventFilter>
#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QSet>
#include <functional>

class AntiMicroSettings;
class JoyTabWidget;
class InputDevice;
class JoyButton;
class QWidget;

namespace Troa {
// Application rules select a native profile; modes select its existing mapping sets.
class ApplicationContext : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
  public:
    ApplicationContext(AntiMicroSettings *settings, std::function<QList<JoyTabWidget *>()> tabs, QObject *parent);
    ~ApplicationContext() override;
    QJsonObject state() const;
    QJsonObject rules() const;
    QJsonObject saveRule(QJsonObject rule, const QString &expectedRevision);
    QJsonObject removeRule(const QString &id, const QString &expectedRevision);
    bool ownsController(const QString &controller) const;
    void poll();
    static QStringList profileModes(const QString &path, QString *error = nullptr);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    bool nativeEventFilter(const QByteArray &type, void *message, qintptr *result) override;
#else
    bool nativeEventFilter(const QByteArray &type, void *message, long *result) override;
#endif
  signals:
    void changed();

  private:
    QJsonObject matchingRule(const QString &controller) const;
    QString revision() const;
    bool persist();
    void registerShortcuts();
    QString cycle(InputDevice *device, const QJsonObject &rule, bool controllerButton);
    void watch(JoyTabWidget *tab);
    void notice(const QString &title, const QString &detail);
    AntiMicroSettings *m_settings;
    std::function<QList<JoyTabWidget *>()> m_tabs;
    QJsonArray m_rules;
    QString m_executable, m_title;
    bool m_mapperFocused = false;
    QMap<int, QString> m_hotkeys;
    QMap<QString, QString> m_shortcutErrors, m_messages, m_lastApplied, m_observed;
    QMap<QString, int> m_rememberedModes;
    QSet<InputDevice *> m_devices;
    QSet<JoyButton *> m_buttons;
    QList<QPointer<QWidget>> m_notices;
    bool m_loading = false;
    int m_focusGeneration = 0;
    int m_windowScanTick = 0;
    QSet<QString> m_openExecutables;
};
} // namespace Troa
