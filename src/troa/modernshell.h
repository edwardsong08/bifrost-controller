// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QWidget>
#include <QIcon>
#include <QJsonObject>
#include <functional>

class AntiMicroSettings;
class QComboBox;
class QLabel;
class QListWidget;
class QTextBrowser;

namespace Troa {
class LocalApi;
class ModernShell : public QWidget
{
  public:
    ModernShell(QWidget *mapping, QWidget *owner, LocalApi *api, AntiMicroSettings *settings,
                std::function<QJsonObject(const QJsonObject &)> handler);
    void refreshProfiles();
    void refreshControllers();
    static void applyAppearance(bool dark);
    static QIcon applicationIcon();

  private:
    QWidget *libraryPage();
    QWidget *assistantPage();
    void previewProfile();
    void applyProfile();
    void copyProfile();
    QString selectedId() const;
    LocalApi *m_api;
    AntiMicroSettings *m_settings;
    std::function<QJsonObject(const QJsonObject &)> m_handler;
    QListWidget *m_profiles = nullptr;
    QTextBrowser *m_preview = nullptr;
    QComboBox *m_controller = nullptr;
    QLabel *m_status = nullptr;
};
} // namespace Troa
