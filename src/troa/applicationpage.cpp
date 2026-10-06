// SPDX-License-Identifier: GPL-3.0-or-later
#include "applicationpage.h"
#include "antimicrosettings.h"
#include "applicationcontext.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QKeySequenceEdit>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QTableWidget>
#include <QUuid>
#include <QVBoxLayout>

namespace {
QLabel *text(const QString &value, const char *role = "body")
{
    auto label = new QLabel(value);
    label->setTextFormat(Qt::PlainText);
    label->setWordWrap(true);
    label->setProperty("role", role);
    return label;
}
QTableWidget *table(const QStringList &columns)
{
    auto widget = new QTableWidget(0, columns.size());
    widget->setHorizontalHeaderLabels(columns);
    widget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    widget->horizontalHeader()->setStretchLastSection(true);
    widget->verticalHeader()->hide();
    widget->setSelectionBehavior(QAbstractItemView::SelectRows);
    widget->setSelectionMode(QAbstractItemView::SingleSelection);
    widget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    widget->setWordWrap(true);
    return widget;
}
void cell(QTableWidget *table, int row, int column, const QString &value)
{
    auto item = new QTableWidgetItem(value);
    item->setToolTip(value);
    table->setItem(row, column, item);
}
} // namespace
namespace Troa {
ApplicationPage::ApplicationPage(ApplicationContext *context, AntiMicroSettings *settings, QWidget *parent)
    : QWidget(parent)
    , m_context(context)
{
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(14);
    layout->addWidget(text("Applications & layouts", "section"));
    layout->addWidget(text("Pick a profile for each application. Within a game, switch named layouts such as Space and "
                           "Ground using a keyboard shortcut or an unused controller button.",
                           "muted"));
    m_focus = text("Waiting for application focus", "heading");
    layout->addWidget(m_focus);
    m_live = table({"Controller", "Assigned profile", "Active profile / layout", "Status"});
    m_live->setMinimumHeight(130);
    layout->addWidget(m_live);
    auto actions = new QHBoxLayout;
    auto add = new QPushButton("Add application");
    add->setProperty("role", "primary");
    m_edit = new QPushButton("Edit rule");
    m_remove = new QPushButton("Remove rule");
    actions->addWidget(add);
    actions->addWidget(m_edit);
    actions->addWidget(m_remove);
    actions->addStretch();
    layout->addLayout(actions);
    connect(add, &QPushButton::clicked, this, [this]() { editRule(false); });
    connect(m_edit, &QPushButton::clicked, this, [this]() { editRule(true); });
    connect(m_remove, &QPushButton::clicked, this, &ApplicationPage::removeRule);
    layout->addWidget(text("Application rules", "heading"));
    m_rules = table({"Application", "State", "Controller", "Profile", "Layouts", "Switch shortcut"});
    m_rules->setMinimumHeight(210);
    layout->addWidget(m_rules);
    connect(m_rules, &QTableWidget::itemSelectionChanged, this, [this]() {
        m_edit->setEnabled(m_rules->currentRow() >= 0);
        m_remove->setEnabled(m_rules->currentRow() >= 0);
    });
    m_feedback = text("", "success");
    m_feedback->hide();
    layout->addWidget(m_feedback);
    layout->addWidget(text("Layouts use the profile's eight mapping sets. Configure the actions under Controllers, then "
                           "save the .amgp profile. A switch button must be unassigned in every selected layout. "
                           "Applications with no rule keep the current profile; this is shown above.",
                           "muted"));
    auto notices = new QHBoxLayout;
    auto showNotice = new QCheckBox("Show profile / layout switch notices");
    showNotice->setChecked(settings->value("TROA/SwitchNotifications", true).toBool());
    auto display = new QComboBox;
    display->addItem("Focused application's screen", "focused");
    display->addItem("Primary screen", "primary");
    display->addItem("All screens", "all");
    display->setCurrentIndex(qMax(0, display->findData(settings->value("TROA/NotificationDisplay", "focused").toString())));
    connect(showNotice, &QCheckBox::toggled, this,
            [settings](bool enabled) { settings->setValue("TROA/SwitchNotifications", enabled); });
    connect(display, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [settings, display](int) { settings->setValue("TROA/NotificationDisplay", display->currentData().toString()); });
    notices->addWidget(showNotice);
    notices->addWidget(display);
    notices->addStretch();
    layout->addLayout(notices);
    layout->addWidget(text("Notices stay above desktop and borderless game windows without taking focus. Use "
                           "borderless/windowed mode if an exclusive fullscreen game hides desktop overlays.",
                           "muted"));
    connect(context, &ApplicationContext::changed, this, &ApplicationPage::refresh);
    refresh();
}
void ApplicationPage::refresh()
{
    const auto state = m_context->state();
    const auto executable = state.value("executable").toString();
    const auto application = QFileInfo(executable).fileName();
    m_focus->setText((state.value("mapper_focused").toBool() ? "Last focused application: " : "Focused application: ") +
                     (application.isEmpty() ? "No application detected" : application));
    m_focus->setToolTip(executable + "\n" + state.value("window_title").toString());
    const auto controllers = state.value("controllers").toArray();
    m_live->setRowCount(controllers.size());
    for (int i = 0; i < controllers.size(); ++i)
    {
        const auto device = controllers.at(i).toObject();
        cell(m_live, i, 0, device.value("controller").toString());
        cell(m_live, i, 1, device.value("assigned_profile").toString());
        cell(m_live, i, 2, device.value("active_profile").toString() + "\n" + device.value("active_mode").toString());
        const auto message = device.value("message").toString();
        cell(m_live, i, 3,
             message.isEmpty()
                 ? (device.value("assignment_active").toBool() ? "Using assigned profile" : "Current profile continues")
                 : message);
    }
    m_live->resizeRowsToContents();
    QString selected;
    if (m_rules->currentRow() >= 0 && m_rules->item(m_rules->currentRow(), 0))
        selected = m_rules->item(m_rules->currentRow(), 0)->data(Qt::UserRole).toString();
    m_rules->blockSignals(true);
    const auto rules = state.value("applications").toArray();
    m_rules->setRowCount(rules.size());
    int selectedRow = -1;
    for (int i = 0; i < rules.size(); ++i)
    {
        const auto rule = rules.at(i).toObject();
        cell(m_rules, i, 0, rule.value("name").toString());
        m_rules->item(i, 0)->setData(Qt::UserRole, rule.value("id").toString());
        if (selected == rule.value("id").toString())
            selectedRow = i;
        cell(m_rules, i, 1, rule.value("focus_state").toString());
        cell(m_rules, i, 2, rule.value("controller_name").toString());
        cell(m_rules, i, 3, QFileInfo(rule.value("profile_path").toString()).completeBaseName());
        QStringList modes;
        for (const auto &value : rule.value("modes").toArray())
            modes.append(value.toObject().value("name").toString());
        cell(m_rules, i, 4, modes.join(" → "));
        QStringList shortcuts;
        if (!rule.value("keyboard_shortcut").toString().isEmpty())
            shortcuts.append(rule.value("keyboard_shortcut").toString());
        if (!rule.value("controller_button_name").toString().isEmpty())
            shortcuts.append(rule.value("controller_button_name").toString());
        cell(m_rules, i, 5, shortcuts.isEmpty() ? "Manual layout buttons" : shortcuts.join(" / "));
    }
    if (selectedRow >= 0)
        m_rules->selectRow(selectedRow);
    m_rules->blockSignals(false);
    m_rules->resizeRowsToContents();
    m_edit->setEnabled(selectedRow >= 0);
    m_remove->setEnabled(selectedRow >= 0);
}
void ApplicationPage::removeRule()
{
    const int row = m_rules->currentRow();
    if (row < 0)
        return;
    const auto result = m_context->removeRule(m_rules->item(row, 0)->data(Qt::UserRole).toString(),
                                              m_context->rules().value("revision").toString());
    m_feedback->setText(result.contains("error") ? result.value("error").toString()
                                                 : "Application rule removed. Its profile file is kept.");
    m_feedback->show();
}
void ApplicationPage::editRule(bool existing)
{
    const auto snapshot = m_context->rules();
    QJsonObject original;
    if (existing && m_rules->currentRow() >= 0)
        for (const auto &value : snapshot.value("rules").toArray())
            if (value.toObject().value("id").toString() ==
                m_rules->item(m_rules->currentRow(), 0)->data(Qt::UserRole).toString())
                original = value.toObject();
    const auto state = m_context->state();
    auto dialog = new QDialog(this);
    dialog->setWindowTitle(existing ? "Edit application rule" : "Add application rule");
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setMinimumWidth(570);
    auto root = new QVBoxLayout(dialog);
    root->setSpacing(12);
    auto form = new QFormLayout;
    auto name = new QLineEdit(original.value("name").toString());
    auto exe = new QLineEdit(original.value("executable").toString(state.value("executable").toString()));
    if (name->text().isEmpty() && !exe->text().isEmpty())
        name->setText(QFileInfo(exe->text()).completeBaseName());
    auto exeRow = new QHBoxLayout;
    auto exeBrowse = new QPushButton("Browse…");
    exeRow->addWidget(exe);
    exeRow->addWidget(exeBrowse);
    connect(exeBrowse, &QPushButton::clicked, dialog, [dialog, exe]() {
        const auto path = QFileDialog::getOpenFileName(dialog, "Choose application", exe->text(), "Applications (*.exe)");
        if (!path.isEmpty())
            exe->setText(path);
    });
    auto controller = new QComboBox;
    const auto devices = state.value("controllers").toArray();
    for (const auto &value : devices)
    {
        const auto device = value.toObject();
        controller->addItem(device.value("controller").toString(), device.value("controller_id").toString());
    }
    if (existing)
        controller->setCurrentIndex(controller->findData(original.value("controller_id").toString()));
    auto path = new QLineEdit(original.value("profile_path").toString());
    auto pathRow = new QHBoxLayout;
    auto pathBrowse = new QPushButton("Browse…");
    auto useCurrent = new QPushButton("Use current");
    pathRow->addWidget(path);
    pathRow->addWidget(pathBrowse);
    pathRow->addWidget(useCurrent);
    connect(pathBrowse, &QPushButton::clicked, dialog, [dialog, path]() {
        const auto selected = QFileDialog::getOpenFileName(dialog, "Choose saved controller profile", path->text(),
                                                           "Controller profiles (*.amgp *.xml)");
        if (!selected.isEmpty())
            path->setText(selected);
    });
    connect(useCurrent, &QPushButton::clicked, dialog, [path, controller, devices]() {
        for (const auto &value : devices)
            if (value.toObject().value("controller_id").toString() == controller->currentData().toString())
                path->setText(value.toObject().value("profile_path").toString());
    });
    form->addRow("Application name", name);
    form->addRow("Application .exe", exeRow);
    form->addRow("Controller", controller);
    form->addRow("Saved profile", pathRow);
    root->addLayout(form);
    root->addWidget(text("Layouts to cycle through", "heading"));
    root->addWidget(text("Select the mapping sets you want and give them clear names. For example, set 1 = Space and set 2 "
                         "= Ground. The actions are edited under Controllers.",
                         "muted"));
    auto modesGrid = new QGridLayout;
    QList<QCheckBox *> enabled;
    QList<QLineEdit *> names;
    for (int i = 1; i <= 8; ++i)
    {
        auto check = new QCheckBox(QString("Set %1").arg(i));
        auto title = new QLineEdit(QString("Layout %1").arg(i));
        check->setChecked(!existing && i <= 2);
        for (const auto &value : original.value("modes").toArray())
            if (value.toObject().value("set").toInt() == i)
            {
                check->setChecked(true);
                title->setText(value.toObject().value("name").toString());
            }
        title->setMaxLength(60);
        title->setEnabled(check->isChecked());
        connect(check, &QCheckBox::toggled, title, &QLineEdit::setEnabled);
        modesGrid->addWidget(check, (i - 1) % 4, ((i - 1) / 4) * 2);
        modesGrid->addWidget(title, (i - 1) % 4, ((i - 1) / 4) * 2 + 1);
        enabled.append(check);
        names.append(title);
    }
    connect(path, &QLineEdit::textChanged, dialog, [names](const QString &value) {
        QString error;
        const auto labels = ApplicationContext::profileModes(value, &error);
        if (error.isEmpty())
            for (int i = 0; i < 8; ++i)
                names.at(i)->setText(labels.at(i));
    });
    root->addLayout(modesGrid);
    auto shortcuts = new QFormLayout;
    auto keyboard = new QKeySequenceEdit(
        QKeySequence::fromString(original.value("keyboard_shortcut").toString(), QKeySequence::PortableText));
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    keyboard->setMaximumSequenceLength(1);
#endif
    auto switchButton = new QComboBox;
    auto updateButtons = [controller, switchButton, devices, original]() {
        switchButton->clear();
        switchButton->addItem("None", -1);
        for (const auto &value : devices)
            if (value.toObject().value("controller_id").toString() == controller->currentData().toString())
                for (const auto &entry : value.toObject().value("controller_buttons").toArray())
                {
                    const auto button = entry.toObject();
                    switchButton->addItem(button.value("name").toString(), button.value("index").toInt());
                }
        switchButton->setCurrentIndex(qMax(0, switchButton->findData(original.value("controller_button").toInt(-1))));
    };
    updateButtons();
    connect(controller, QOverload<int>::of(&QComboBox::currentIndexChanged), dialog,
            [updateButtons](int) { updateButtons(); });
    shortcuts->addRow("Keyboard shortcut", keyboard);
    shortcuts->addRow("Controller switch button", switchButton);
    root->addLayout(shortcuts);
    root->addWidget(
        text("Shortcuts work while this application is focused. Controller switching occurs on release; leave that button "
             "unmapped in all selected sets. A–Z, 0–9 and F1–F11 are supported, with optional Ctrl/Alt/Shift.",
             "muted"));
    auto error = text("", "error");
    error->hide();
    root->addWidget(error);
    auto buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    root->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, dialog, [=]() {
        QJsonArray modes;
        for (int i = 0; i < 8; ++i)
            if (enabled.at(i)->isChecked())
                modes.append(QJsonObject{{"set", i + 1}, {"name", names.at(i)->text().trimmed()}});
        QJsonObject rule{
            {"id", original.value("id").toString("app-" + QUuid::createUuid().toString(QUuid::WithoutBraces))},
            {"name", name->text().trimmed()},
            {"executable", exe->text()},
            {"controller_id", controller->currentData().toString()},
            {"controller_name", controller->currentText()},
            {"profile_path", path->text()},
            {"modes", modes},
            {"keyboard_shortcut", keyboard->keySequence().toString(QKeySequence::PortableText)},
            {"controller_button", switchButton->currentData().toInt()},
            {"controller_button_name", switchButton->currentData().toInt() < 0 ? "" : switchButton->currentText()}};
        const auto result = m_context->saveRule(rule, snapshot.value("revision").toString());
        if (result.contains("error"))
        {
            error->setText(result.value("error").toString());
            error->show();
            return;
        }
        m_feedback->setText("Application rule saved. Focus that app to use its profile and switch layouts.");
        m_feedback->show();
        dialog->accept();
    });
    dialog->open();
}
} // namespace Troa
