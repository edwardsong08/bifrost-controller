// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QWidget>
class QLabel;
class QTableWidget;
class QPushButton;
class AntiMicroSettings;
namespace Troa {
class ApplicationContext;
class ApplicationPage : public QWidget
{
  public:
    ApplicationPage(ApplicationContext *context, AntiMicroSettings *settings, QWidget *parent = nullptr);
    void refresh();

  private:
    void editRule(bool existing);
    void removeRule();
    ApplicationContext *m_context;
    QLabel *m_focus, *m_feedback;
    QTableWidget *m_live, *m_rules;
    QPushButton *m_edit, *m_remove;
};
} // namespace Troa
