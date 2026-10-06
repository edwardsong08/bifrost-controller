// SPDX-License-Identifier: GPL-3.0-or-later
#include "modernshell.h"
#include "identity.h"
#include "localapi.h"
#include "profilestore.h"
#include "antimicrosettings.h"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QDesktopServices>
#include <QDir>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QFont>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QPlainTextEdit>
#include <QPixmap>
#include <QPalette>
#include <QVariant>
#include <QPushButton>
#include <QRegularExpression>
#include <QStackedWidget>
#include <QStyleFactory>
#include <QTextBrowser>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

namespace {
QLabel *label(const QString &text, const char *role = "body")
{
    auto result = new QLabel(text);
    result->setProperty("role", role);
    result->setWordWrap(true);
    return result;
}
QPushButton *button(const QString &text, bool primary = false)
{
    auto result = new QPushButton(text);
    result->setCursor(Qt::PointingHandCursor);
    if (primary) result->setProperty("role", "primary");
    return result;
}
void showError(QWidget *parent, const QJsonObject &result)
{
    if (result.contains("error")) QMessageBox::information(parent, "Profile library", result.value("error").toString());
}
} // namespace

namespace Troa {
QIcon ModernShell::applicationIcon()
{
    QIcon icon;
    for (const int size : {16, 32, 48, 64, 128, 256}) {
        QPixmap pixmap(size, size); pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap); painter.setRenderHint(QPainter::Antialiasing);
        painter.scale(size / 128.0, size / 128.0);
        painter.setPen(Qt::NoPen); painter.setBrush(QColor("#182635"));
        painter.drawRoundedRect(QRectF(4, 4, 120, 120), 28, 28);
        painter.setPen(QPen(QColor("#ddb975"), 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.setBrush(Qt::NoBrush);
        QPainterPath outline; outline.moveTo(37, 40); outline.lineTo(91, 40);
        outline.cubicTo(111, 40, 116, 98, 100, 99); outline.cubicTo(88, 99, 88, 80, 77, 79);
        outline.lineTo(51, 79); outline.cubicTo(40, 80, 40, 99, 28, 99);
        outline.cubicTo(12, 98, 17, 40, 37, 40); painter.drawPath(outline);
        painter.setPen(QPen(QColor("#f5f1e8"), 5, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(34, 57, 34, 73); painter.drawLine(26, 65, 42, 65);
        painter.setBrush(QColor("#f5f1e8")); painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPointF(86, 59), 4, 4); painter.drawEllipse(QPointF(96, 69), 4, 4);
        icon.addPixmap(pixmap);
    }
    return icon;
}
void ModernShell::applyAppearance(bool dark)
{
    const QString background = dark ? "#111820" : "#f2f4f7";
    const QString surface = dark ? "#1c2631" : "#ffffff";
    const QString text = dark ? "#edf1f5" : "#192736";
    const QString muted = dark ? "#aab6c3" : "#596b7e";
    const QString border = dark ? "#354350" : "#dbe2e9";
    const QString hover = dark ? "#293746" : "#eaf0f6";
    auto palette = qApp->palette();
    palette.setColor(QPalette::Window, QColor(background)); palette.setColor(QPalette::WindowText, QColor(text));
    palette.setColor(QPalette::Base, QColor(surface)); palette.setColor(QPalette::AlternateBase, QColor(background));
    palette.setColor(QPalette::Text, QColor(text)); palette.setColor(QPalette::Button, QColor(surface));
    palette.setColor(QPalette::ButtonText, QColor(text)); palette.setColor(QPalette::Highlight, QColor("#235f98"));
    palette.setColor(QPalette::HighlightedText, Qt::white);
    palette.setColor(QPalette::ToolTipBase, QColor(surface)); palette.setColor(QPalette::ToolTipText, QColor(text));
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor(muted));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(muted)); qApp->setPalette(palette);
    qApp->setStyleSheet(QString(R"(
        QWidget { color: %1; font-size: 13px; }
        QMainWindow, QWidget#troaShell { background: %2; }
        QLabel { background: transparent; }
        QLabel[role="title"] { font-size: 27px; font-weight: 600; }
        QLabel[role="section"] { font-size: 21px; font-weight: 600; }
        QLabel[role="eyebrow"] { color: %3; font-size: 11px; font-weight: 600; }
        QLabel[role="muted"] { color: %3; }
        QLabel[role="status"] { background: %4; padding: 9px 14px; border-radius: 8px; color: %3; }
        QWidget#troaSidebar { background: %4; border: 1px solid %5; border-radius: 12px; }
        QWidget[role="card"] { background: %4; border: 1px solid %5; border-radius: 12px; }
        QPushButton, QToolButton { background: %4; border: 1px solid %5; border-radius: 7px; padding: 8px 12px; min-height: 18px; }
        QPushButton:hover, QToolButton:hover { background: %6; border-color: #7794ae; }
        QPushButton:focus, QLineEdit:focus, QComboBox:focus { border: 2px solid #397bb2; }
        QPushButton:checked { background: %6; border-color: #397bb2; }
        QPushButton[role="primary"] { background: #235f98; color: white; border-color: #235f98; }
        QPushButton[role="primary"]:hover { background: #1b507f; }
        QPushButton[role="nav"] { text-align: left; border: none; padding: 13px; }
        QPushButton[role="nav"]:checked { color: #235f98; background: %6; font-weight: 600; }
        QPushButton:disabled { color: %3; background: %2; border-color: %5; }
        QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox { background: %4; border: 1px solid %5; border-radius: 6px; padding: 7px; min-height: 18px; }
        QListWidget, QTextBrowser, QPlainTextEdit, QScrollArea { background: %4; border: 1px solid %5; border-radius: 8px; }
        QListWidget::item { padding: 12px; border-bottom: 1px solid %5; }
        QListWidget::item:selected { background: %6; color: %1; border-left: 3px solid #235f98; }
        QTabWidget::pane { background: %4; border: 1px solid %5; border-radius: 8px; }
        QTabBar::tab { background: %2; border: none; padding: 10px 15px; color: %3; }
        QTabBar::tab:selected { background: %4; color: %1; border-bottom: 3px solid #235f98; }
        QGroupBox { border: 1px solid %5; border-radius: 8px; margin-top: 13px; padding-top: 15px; }
        QGroupBox::title { subcontrol-origin: margin; padding: 0 8px; }
        QMenu, QMenuBar { background: %4; }
        QMenu::item { padding: 8px 24px; }
        QMenu::item:selected { background: %6; }
        QScrollBar:vertical { background: %2; width: 12px; }
        QScrollBar::handle:vertical { background: %5; border-radius: 5px; min-height: 24px; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        FlashButtonWidget[isflashing="true"] { background: #235f98; color: white; border-color: #ddb975; }
        QPushButton[setActive="true"] { background: #235f98; color: white; }
        QPushButton[isDisplayingNames="true"] { border-color: #397bb2; }
    )").arg(text, background, muted, surface, border, hover));
}
ModernShell::ModernShell(QWidget *mapping, QWidget *owner, LocalApi *api, AntiMicroSettings *settings,
                         std::function<QJsonObject(const QJsonObject &)> handler)
    : QWidget(owner), m_api(api), m_settings(settings), m_handler(std::move(handler))
{
    setObjectName("troaShell");
    qApp->setStyle(QStyleFactory::create("Fusion"));
    QFont font = qApp->font(); font.setFamily("Segoe UI"); qApp->setFont(font);
    auto root = new QVBoxLayout(this); root->setContentsMargins(22, 18, 22, 22); root->setSpacing(20);
    auto header = new QHBoxLayout;
    auto icon = new QLabel; icon->setPixmap(applicationIcon().pixmap(52, 52)); header->addWidget(icon);
    auto titles = new QVBoxLayout; titles->setSpacing(3);
    titles->addWidget(label("TROA COMMUNITY", "eyebrow")); titles->addWidget(label("PC Controller Mapper", "title"));
    header->addLayout(titles); header->addStretch();
    m_status = label("Connect a controller", "status"); header->addWidget(m_status);
    auto appearance = new QCheckBox("Dark appearance");
    appearance->setChecked(settings->value("TROA/DarkAppearance", false).toBool());
    applyAppearance(appearance->isChecked());
    connect(appearance, &QCheckBox::toggled, this, [this](bool dark) {
        m_settings->setValue("TROA/DarkAppearance", dark); applyAppearance(dark);
    }); header->addWidget(appearance); root->addLayout(header);
    auto body = new QHBoxLayout; body->setSpacing(20);
    auto sidebar = new QWidget; sidebar->setObjectName("troaSidebar"); sidebar->setFixedWidth(190);
    auto navigation = new QVBoxLayout(sidebar); navigation->setContentsMargins(12, 18, 12, 18); navigation->setSpacing(8);
    navigation->addWidget(label("WORKSPACE", "eyebrow"));
    auto pages = new QStackedWidget;
    auto mappingPage = new QWidget; auto mappingLayout = new QVBoxLayout(mappingPage);
    mappingLayout->setContentsMargins(0, 0, 0, 0); mappingLayout->setSpacing(12);
    mappingLayout->addWidget(label("Controller mappings", "section"));
    mappingLayout->addWidget(label("Select a controller, then choose a button or stick to assign an action. Inputs highlight as you use them.", "muted"));
    auto shortcuts = new QHBoxLayout;
    const QList<QPair<QString, QString>> actions = {{"Rescan controllers", "actionUpdate_Joysticks"},
        {"Calibrate", "actionCalibration"}, {"Controller details", "actionProperties"}, {"Settings", "actionOptions"}};
    for (const auto &action : actions) {
        auto control = button(action.first); shortcuts->addWidget(control);
        connect(control, &QPushButton::clicked, owner, [owner, action]() {
            if (auto target = owner->findChild<QAction *>(action.second)) target->trigger();
        });
    }
    shortcuts->addStretch(); mappingLayout->addLayout(shortcuts);
    mapping->setProperty("role", "card"); mappingLayout->addWidget(mapping, 1); pages->addWidget(mappingPage);
    pages->addWidget(libraryPage()); pages->addWidget(assistantPage());
    const QStringList names = {"Controller mappings", "Profile library", "Assistant access"};
    for (int index = 0; index < names.size(); ++index) {
        auto control = button(names.at(index)); control->setProperty("role", "nav");
        control->setCheckable(true); control->setAutoExclusive(true); control->setChecked(index == 0);
        navigation->addWidget(control);
        connect(control, &QPushButton::clicked, this, [this, pages, index]() {
            pages->setCurrentIndex(index); if (index == 1) { refreshProfiles(); refreshControllers(); }
        });
    }
    navigation->addStretch(); navigation->addWidget(label("Windows first.\nBuilt for our community.", "muted"));
    auto support = button("Project & support"); navigation->addWidget(support);
    connect(support, &QPushButton::clicked, this, []() { QDesktopServices::openUrl(QUrl(projectUrl())); });
    navigation->addWidget(label("Based on AntiMicroX\nGPL open-source software", "muted"));
    body->addWidget(sidebar); body->addWidget(pages, 1); root->addLayout(body, 1);
    auto timer = new QTimer(this); connect(timer, &QTimer::timeout, this, &ModernShell::refreshControllers); timer->start(2000);
    refreshProfiles(); refreshControllers();
}
QWidget *ModernShell::libraryPage()
{
    auto page = new QWidget; auto root = new QVBoxLayout(page); root->setContentsMargins(0, 0, 0, 0); root->setSpacing(12);
    root->addWidget(label("Profile library", "section"));
    root->addWidget(label("Start with a community template or a personal profile. Updates replace bundled templates while keeping your personal copies.", "muted"));
    auto search = new QLineEdit; search->setPlaceholderText("Find a profile by name or category"); root->addWidget(search);
    auto split = new QHBoxLayout; m_profiles = new QListWidget; m_profiles->setMinimumWidth(220); split->addWidget(m_profiles, 1);
    m_preview = new QTextBrowser; m_preview->setOpenExternalLinks(false); split->addWidget(m_preview, 2); root->addLayout(split, 1);
    connect(m_profiles, &QListWidget::currentRowChanged, this, [this](int) { previewProfile(); });
    connect(search, &QLineEdit::textChanged, this, [this](const QString &query) {
        for (int row = 0; row < m_profiles->count(); ++row) {
            auto item = m_profiles->item(row); item->setHidden(!item->text().contains(query, Qt::CaseInsensitive));
        }
    });
    auto actions = new QHBoxLayout; m_controller = new QComboBox; m_controller->setMinimumWidth(220);
    m_controller->setAccessibleName("Controller to receive profile"); actions->addWidget(m_controller, 1);
    auto apply = button("Apply to controller", true); actions->addWidget(apply);
    auto copy = button("Copy to personal"); actions->addWidget(copy);
    auto refresh = button("Refresh library"); actions->addWidget(refresh); root->addLayout(actions);
    connect(apply, &QPushButton::clicked, this, &ModernShell::applyProfile);
    connect(copy, &QPushButton::clicked, this, &ModernShell::copyProfile);
    connect(refresh, &QPushButton::clicked, this, &ModernShell::refreshProfiles);
    root->addWidget(label("Templates use SDL's standard controller layout. Save your changes in Controller mappings before switching profiles.", "muted"));
    return page;
}
QWidget *ModernShell::assistantPage()
{
    auto page = new QWidget; auto root = new QVBoxLayout(page); root->setContentsMargins(0, 0, 0, 0); root->setSpacing(14);
    root->addWidget(label("Assistant access", "section"));
    root->addWidget(label("Connect a compatible MCP client to create profiles together, inspect assignments, and apply a profile to a specific controller.", "muted"));
    auto enabled = new QCheckBox("Enable local assistant access"); enabled->setChecked(m_api->isEnabled()); root->addWidget(enabled);
    auto status = label(m_api->isEnabled() ? "Ready for a local MCP connection." : "Assistant access is off.", "status"); root->addWidget(status);
    connect(enabled, &QCheckBox::toggled, this, [this, enabled, status](bool on) {
        const bool success = m_api->setEnabled(on);
        m_settings->setValue("TROA/AssistantAccess", on && success);
        status->setText(success ? (on ? "Ready for a local MCP connection." : "Assistant access is off.")
                               : "Could not open the local connection: " + m_api->errorString());
        if (!success) { enabled->blockSignals(true); enabled->setChecked(false); enabled->blockSignals(false); }
    });
    root->addWidget(label("Connection settings", "section"));
#ifdef Q_OS_WIN
    const auto executable = QDir::toNativeSeparators(QDir(QCoreApplication::applicationDirPath()).filePath("troa-controller-mcp.exe"));
#else
    const auto executable = QDir(QCoreApplication::applicationDirPath()).filePath("troa-controller-mcp");
#endif
    const QJsonObject config{{"mcpServers", QJsonObject{{"troa-controller-mapper", QJsonObject{{"command", executable}, {"args", QJsonArray{}}}}}}};
    const QString json = QString::fromUtf8(QJsonDocument(config).toJson(QJsonDocument::Indented));
    auto code = new QPlainTextEdit(json); code->setReadOnly(true); code->setMaximumHeight(180); root->addWidget(code);
    auto copy = button("Copy MCP connection settings", true); root->addWidget(copy, 0, Qt::AlignLeft);
    connect(copy, &QPushButton::clicked, this, [json]() { qApp->clipboard()->setText(json); });
    root->addWidget(label("Keep this app open while using the connection. Profiles can be drafted and saved without changing active mappings. The connection is restricted to your local user account.", "muted"));
    auto folder = button("Open personal profile folder"); root->addWidget(folder, 0, Qt::AlignLeft);
    connect(folder, &QPushButton::clicked, this, []() { QDir().mkpath(profileDirectory()); QDesktopServices::openUrl(QUrl::fromLocalFile(profileDirectory())); });
    root->addStretch(); return page;
}
QString ModernShell::selectedId() const
{
    return m_profiles && m_profiles->currentItem() ? m_profiles->currentItem()->data(Qt::UserRole).toString() : QString{};
}
void ModernShell::refreshProfiles()
{
    const auto selected = selectedId(); m_profiles->blockSignals(true); m_profiles->clear();
    int selectedRow = 0;
    for (const auto &value : ProfileStore().list()) {
        const auto profile = value.toObject();
        auto item = new QListWidgetItem(profile.value("name").toString() + "\n" + profile.value("category").toString()
            + (profile.value("bundled").toBool() ? " · Community template" : " · Personal"), m_profiles);
        item->setData(Qt::UserRole, profile.value("id"));
        if (profile.value("id").toString() == selected) selectedRow = m_profiles->count() - 1;
    }
    m_profiles->blockSignals(false); m_profiles->setCurrentRow(selectedRow); previewProfile();
}
void ModernShell::previewProfile()
{
    const auto item = ProfileStore().read(selectedId()); const auto profile = item.value("profile").toObject();
    if (item.contains("error")) { m_preview->setPlainText("Choose a profile to see its assignments."); return; }
    QString html = "<h2>" + profile.value("name").toString().toHtmlEscaped() + "</h2><p>" + profile.value("description").toString().toHtmlEscaped() + "</p><table cellspacing='8'>";
    for (const auto &value : profile.value("bindings").toArray()) {
        const auto binding = value.toObject(); QString action;
        if (binding.contains("keys")) {
            QStringList chord; for (const auto &key : binding.value("keys").toArray()) chord.append(key.toString()); action = chord.join(" + ");
        } else if (binding.contains("mouse_button")) {
            const QStringList mouse = {"", "Left click", "Middle click", "Right click", "Scroll up", "Scroll down", "Scroll left", "Scroll right", "Back", "Forward"};
            action = mouse.value(binding.value("mouse_button").toInt());
        } else action = "Move pointer " + binding.value("mouse_move").toString();
        html += "<tr><td><b>" + QString(binding.value("input").toString()).replace('_', ' ').toHtmlEscaped() + "</b></td><td>" + action.toHtmlEscaped() + "</td></tr>";
    }
    m_preview->setHtml(html + "</table>");
}
void ModernShell::refreshControllers()
{
    const auto response = m_handler({{"method", "list_controllers"}}); const auto devices = response.value("controllers").toArray();
    m_status->setText(devices.isEmpty() ? "Connect a controller" : QString("%1 controller%2 connected").arg(devices.size()).arg(devices.size() == 1 ? "" : "s"));
    const auto signature = QJsonDocument(devices).toJson(QJsonDocument::Compact);
    if (m_controller->property("devices").toByteArray() == signature) return;
    m_controller->setProperty("devices", signature);
    const auto selected = m_controller->currentData().toString(); m_controller->clear();
    for (const auto &value : devices) {
        const auto device = value.toObject();
        if (!device.value("standard_layout").toBool()) continue;
        m_controller->addItem(device.value("name").toString(), device.value("controller_id"));
    }
    const int index = m_controller->findData(selected); if (index >= 0) m_controller->setCurrentIndex(index);
    if (!m_controller->count()) m_controller->addItem("Connect an SDL-mapped controller", QString{});
}
void ModernShell::applyProfile()
{
    const auto item = ProfileStore().read(selectedId());
    if (item.contains("error")) { showError(this, item); return; }
    const auto result = m_handler({{"method", "activate_profile"}, {"arguments", QJsonObject{{"id", selectedId()},
        {"controller_id", m_controller->currentData().toString()}, {"expected_revision", item.value("revision")}}}});
    showError(this, result);
    if (!result.contains("error")) QMessageBox::information(this, "Profile applied", "Profile applied. Open Controller mappings to adjust assignments.");
}
void ModernShell::copyProfile()
{
    const auto item = ProfileStore().read(selectedId());
    if (item.contains("error")) { showError(this, item); return; }
    bool ok = false;
    const auto name = QInputDialog::getText(this, "Copy to personal profile", "Name your personal profile:", QLineEdit::Normal,
                                          item.value("profile").toObject().value("name").toString() + " (personal)", &ok).trimmed();
    if (!ok || name.isEmpty()) return;
    auto id = name.toLower(); id.replace(QRegularExpression("[^a-z0-9]+"), "-");
    id.remove(QRegularExpression("^-|-$")); id = id.left(48);
    if (id.isEmpty() || id.startsWith("builtin-")) id = "my-profile";
    const auto base = id; int suffix = 2;
    while (!ProfileStore().read(id).contains("error")) id = base + "-" + QString::number(suffix++);
    auto profile = item.value("profile").toObject(); profile.insert("id", id); profile.insert("name", name);
    const auto result = ProfileStore().save(profile, {}); showError(this, result); refreshProfiles();
}
} // namespace Troa
