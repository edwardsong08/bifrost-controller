// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QIcon>
#include <QJsonObject>
#include <QList>
#include <QWidget>
#include <functional>

class AntiMicroSettings;
class QComboBox;
class QLabel;
class QListWidget;
class QLineEdit;
class QPushButton;
class QPlainTextEdit;
class QStackedWidget;
class QTableWidget;

namespace Troa {
class LocalApi;
class ApplicationContext;
class ModernShell : public QWidget
{
  public:
    ModernShell(QWidget *mapping, QWidget *owner, LocalApi *api, AntiMicroSettings *settings, ApplicationContext *context,
                std::function<QJsonObject(const QJsonObject &)> handler);
    void refreshProfiles();
    void refreshControllers();
    void showAssistant();
    void copyConnectionSettings();
    static void applyAppearance(bool dark);
    static QIcon applicationIcon();
    static QIcon controllerIcon();

  private:
    QWidget *startPage();
    QWidget *libraryPage();
    QWidget *assistantPage();
    void navigate(int index);
    void chooseTemplate(const QString &id);
    void filterProfiles();
    void updateProfileActions();
    void refreshAssistantStatus();
    void updateConnectionSettings();
    void libraryFeedback(const QString &text, bool error = false);
    void previewProfile();
    void applyProfile();
    void copyProfile();
    QString selectedId() const;
    LocalApi *m_api;
    AntiMicroSettings *m_settings;
    std::function<QJsonObject(const QJsonObject &)> m_handler;
    QListWidget *m_profiles = nullptr;
    QStackedWidget *m_pages = nullptr;
    QList<QPushButton *> m_navigation;
    QLineEdit *m_search = nullptr;
    QLabel *m_profileName = nullptr;
    QLabel *m_profileDescription = nullptr;
    QLabel *m_profileMeta = nullptr;
    QTableWidget *m_bindings = nullptr;
    QPushButton *m_apply = nullptr;
    QPushButton *m_copy = nullptr;
    QLabel *m_libraryFeedback = nullptr;
    QLabel *m_controllerHelp = nullptr;
    QComboBox *m_controller = nullptr;
    QComboBox *m_layout = nullptr;
    QLabel *m_status = nullptr;
    QLabel *m_startStatus = nullptr;
    QLabel *m_startDetail = nullptr;
    QLabel *m_mcpStatus = nullptr;
    QLabel *m_mcpDetail = nullptr;
    QLabel *m_setupFeedback = nullptr;
    QComboBox *m_configFormat = nullptr;
    QPlainTextEdit *m_configText = nullptr;
};
} // namespace Troa
