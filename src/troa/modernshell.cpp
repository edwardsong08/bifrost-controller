// SPDX-License-Identifier: GPL-3.0-or-later
#include "modernshell.h"
#include "antimicrosettings.h"
#include "applicationcontext.h"
#include "applicationpage.h"
#include "controllersupport.h"
#include "identity.h"
#include "localapi.h"
#include "profilestore.h"

#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMap>
#include <QMenu>
#include <QMessageBox>
#include <QMutexLocker>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QPixmap>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QScrollArea>
#include <QStackedWidget>
#include <QStyle>
#include <QStyleFactory>
#include <QTableWidget>
#include <QTimer>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>
#include <QVariant>

namespace {
QLabel *label(const QString &text, const char *role = "body")
{
    auto result = new QLabel(text);
    result->setTextFormat(Qt::PlainText);
    result->setProperty("role", role);
    result->setWordWrap(true);
    result->setMinimumWidth(0);
    result->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    return result;
}
QPushButton *button(const QString &text, bool primary = false)
{
    auto result = new QPushButton(text);
    result->setCursor(Qt::PointingHandCursor);
    if (primary)
        result->setProperty("role", "primary");
    return result;
}
QWidget *card()
{
    auto result = new QWidget;
    result->setProperty("role", "card");
    return result;
}
QVBoxLayout *cardLayout(QWidget *widget)
{
    auto layout = new QVBoxLayout(widget);
    layout->setContentsMargins(20, 18, 20, 18);
    layout->setSpacing(12);
    return layout;
}
QWidget *scrollPage(QWidget *content)
{
    auto scroll = new QScrollArea;
    scroll->setObjectName("troaPageScroll");
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setWidget(content);
    return scroll;
}
QWidget *step(const QString &number, const QString &title, const QString &description)
{
    auto widget = new QWidget;
    auto layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 3, 0, 3);
    layout->setSpacing(14);
    auto badge = label(number, "step");
    badge->setAlignment(Qt::AlignCenter);
    badge->setFixedSize(30, 30);
    layout->addWidget(badge, 0, Qt::AlignTop);
    auto text = new QVBoxLayout;
    text->setSpacing(4);
    text->addWidget(label(title, "heading"));
    text->addWidget(label(description, "muted"));
    layout->addLayout(text, 1);
    return widget;
}
QString companionPath()
{
#ifdef Q_OS_WIN
    const QDir installRoot(QDir(QCoreApplication::applicationDirPath()).filePath("../../.."));
    if (QFileInfo::exists(installRoot.filePath("current.txt")) &&
        QFileInfo::exists(installRoot.filePath("bifrost-mcp-host.exe")))
        return QDir::toNativeSeparators(QDir::cleanPath(installRoot.filePath("bifrost-mcp-host.exe")));
    return QDir::toNativeSeparators(QDir(QCoreApplication::applicationDirPath()).filePath("bifrost-controller-mcp.exe"));
#else
    return QDir(QCoreApplication::applicationDirPath()).filePath("bifrost-controller-mcp");
#endif
}
QString connectionJson()
{
    const QJsonObject config{{"mcpServers", QJsonObject{{"bifrost-controller", QJsonObject{{"command", companionPath()},
                                                                                           {"args", QJsonArray{}}}}}}};
    return QString::fromUtf8(QJsonDocument(config).toJson(QJsonDocument::Indented));
}
QString actionName(const QJsonObject &binding)
{
    if (binding.contains("keys"))
    {
        QStringList chord;
        for (const auto &key : binding.value("keys").toArray())
            chord.append(key.toString());
        return chord.join(" + ");
    }
    if (binding.contains("mouse_button"))
    {
        const QStringList names = {"",          "Left click",  "Middle click", "Right click",
                                   "Scroll up", "Scroll down", "Scroll left",  "Scroll right",
                                   "Back",      "Forward"};
        return names.value(binding.value("mouse_button").toInt());
    }
    return "Move pointer " + binding.value("mouse_move").toString();
}
void feedback(QLabel *widget, const QString &text, bool error = false)
{
    widget->setProperty("role", error ? "error" : "success");
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->setText(text);
    widget->setVisible(!text.isEmpty());
}
} // namespace

namespace Troa {
QIcon ModernShell::applicationIcon() { return QIcon(":/troa/logo.png"); }
QIcon ModernShell::controllerIcon()
{
    QIcon icon;
    for (const int size : {16, 32, 48, 64, 128, 256})
    {
        QPixmap pixmap(size, size);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.scale(size / 128.0, size / 128.0);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#182635"));
        painter.drawRoundedRect(QRectF(4, 4, 120, 120), 28, 28);
        painter.setPen(QPen(QColor("#ddb975"), 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.setBrush(Qt::NoBrush);
        QPainterPath outline;
        outline.moveTo(37, 40);
        outline.lineTo(91, 40);
        outline.cubicTo(111, 40, 116, 98, 100, 99);
        outline.cubicTo(88, 99, 88, 80, 77, 79);
        outline.lineTo(51, 79);
        outline.cubicTo(40, 80, 40, 99, 28, 99);
        outline.cubicTo(12, 98, 17, 40, 37, 40);
        painter.drawPath(outline);
        painter.setPen(QPen(QColor("#f5f1e8"), 5, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(34, 57, 34, 73);
        painter.drawLine(26, 65, 42, 65);
        painter.setBrush(QColor("#f5f1e8"));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPointF(86, 59), 4, 4);
        painter.drawEllipse(QPointF(96, 69), 4, 4);
        icon.addPixmap(pixmap);
    }
    return icon;
}

void ModernShell::applyAppearance(bool dark)
{
    // TROA's deployed ivory/charcoal/gold palette, with Bifrost's quiet navy surfaces.
    const QString background = dark ? "#0d0e0f" : "#f4f0e8";
    const QString surface = dark ? "#171c25" : "#fffaf1";
    const QString text = dark ? "#f4f0e8" : "#252019";
    const QString muted = dark ? "#bfb8ad" : "#665c50";
    const QString border = dark ? "#3f3c31" : "#d4c5a8";
    const QString hover = dark ? "#242c39" : "#eee4d1";
    const QString selection = dark ? "#342c1c" : "#f2e3bc";
    const QString accentText = dark ? "#ffda83" : "#775606";
    const QString primary = dark ? "#d4a84f" : "#e3b545";
    const QString primaryText = "#241a09";
    const QString primaryHover = dark ? "#eac477" : "#efc464";
    auto palette = qApp->palette();
    palette.setColor(QPalette::Window, QColor(background));
    palette.setColor(QPalette::WindowText, QColor(text));
    palette.setColor(QPalette::Base, QColor(surface));
    palette.setColor(QPalette::AlternateBase, QColor(background));
    palette.setColor(QPalette::Text, QColor(text));
    palette.setColor(QPalette::Button, QColor(surface));
    palette.setColor(QPalette::ButtonText, QColor(text));
    palette.setColor(QPalette::Highlight, QColor(primary));
    palette.setColor(QPalette::HighlightedText, QColor(primaryText));
    palette.setColor(QPalette::ToolTipBase, QColor(surface));
    palette.setColor(QPalette::ToolTipText, QColor(text));
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor(muted));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(muted));
    qApp->setPalette(palette);
    qApp->setStyleSheet(QString(R"(
        QWidget { color: %1; font-size: 13px; }
        QMainWindow, QWidget#troaShell { background: %2; }
        QLabel { background: transparent; }
        QLabel[role="title"] { font-size: 22px; font-weight: 600; }
        QLabel[role="section"] { font-size: 25px; font-weight: 600; }
        QLabel[role="heading"] { font-size: 15px; font-weight: 600; }
        QLabel[role="eyebrow"] { color: %3; font-size: 11px; font-weight: 600; }
        QLabel[role="muted"] { color: %3; }
        QLabel[role="status"] { color: %3; padding: 6px 10px; }
        QLabel[role="step"] { background: %7; color: %8; border-radius: 15px; font-weight: 600; }
        QLabel[role="success"] { color: %8; background: %7; border-radius: 8px; padding: 10px; }
        QLabel[role="error"] { color: %1; border: 1px solid #b36537; border-radius: 8px; padding: 10px; }
        QWidget#troaSidebar { background: %4; border: 1px solid %5; border-radius: 12px; }
        QWidget[role="card"] { background: %4; border: 1px solid %5; border-radius: 12px; }
        QScrollArea#troaPageScroll, QScrollArea#troaPageScroll > QWidget > QWidget { background: %2; border: none; }
        QPushButton, QToolButton { background: %4; border: 1px solid %5; border-radius: 8px; padding: 8px 12px; min-height: 18px; }
        QPushButton:hover, QToolButton:hover { background: %6; }
        QPushButton:focus, QToolButton:focus, QLineEdit:focus, QComboBox:focus, QCheckBox:focus {
            border: 2px solid %9;
        }
        QPushButton:checked { background: %7; border-color: %9; }
        QPushButton[role="primary"] { background: %9; color: %10; border-color: %9; }
        QPushButton[role="primary"]:hover { background: %11; }
        QPushButton[role="nav"] { text-align: left; border: none; padding: 10px; background: transparent; }
        QPushButton[role="nav"]:checked { color: %8; background: %7; font-weight: 600; }
        QPushButton:disabled { color: %3; background: %2; border-color: %5; }
        QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox { background: %4; border: 1px solid %5; border-radius: 7px; padding: 7px; min-height: 18px; }
        QListWidget, QTableWidget, QTextBrowser, QPlainTextEdit, QScrollArea { background: %4; border: 1px solid %5; border-radius: 9px; }
        QListWidget::item { padding: 14px 12px; border-bottom: 1px solid %5; }
        QListWidget::item:selected { background: %7; color: %1; }
        QTableWidget { gridline-color: %5; }
        QHeaderView::section { background: %2; color: %3; border: none; border-bottom: 1px solid %5; padding: 9px; font-weight: 600; }
        QTableWidget::item { padding: 6px; border-bottom: 1px solid %5; }
        QTabWidget::pane { background: %4; border: 1px solid %5; border-radius: 9px; }
        QTabBar::tab { background: %2; border: none; padding: 9px 14px; color: %3; }
        QTabBar::tab:selected { background: %4; color: %1; border-bottom: 2px solid %9; }
        QGroupBox { border: 1px solid %5; border-radius: 9px; margin-top: 13px; padding-top: 15px; }
        QGroupBox::title { subcontrol-origin: margin; padding: 0 8px; }
        QDialog { background: %2; }
        QMenuBar { background: %4; border-bottom: 1px solid %5; padding: 4px 8px; }
        QMenuBar::item { background: transparent; padding: 7px 12px; margin: 2px; border-radius: 6px; }
        QMenuBar::item:selected, QMenuBar::item:pressed { background: %7; color: %8; }
        QMenu { background: %4; border: 1px solid %5; border-radius: 9px; padding: 6px; }
        QMenu::item { padding: 8px 30px 8px 12px; border-radius: 5px; }
        QMenu::item:selected { background: %7; color: %8; }
        QMenu::item:disabled { color: %3; }
        QMenu::separator { height: 1px; background: %5; margin: 6px 8px; }
        QComboBox { padding-right: 28px; }
        QComboBox::drop-down { subcontrol-origin: padding; subcontrol-position: top right; width: 26px;
                              border-left: 1px solid %5; }
        QComboBox QAbstractItemView { background: %4; color: %1; border: 1px solid %5;
                                    selection-background-color: %7; selection-color: %8; padding: 4px; }
        QDialogButtonBox QPushButton { min-width: 68px; }
        QScrollBar:vertical { background: transparent; width: 10px; }
        QScrollBar::handle:vertical { background: %5; border-radius: 5px; min-height: 24px; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        JoyButtonStatusBox[isflashing="true"], FlashButtonWidget[isflashing="true"] { background: %9; color: %10; border-color: %9; }
        QPushButton[setActive="true"] { background: %9; color: %10; }
        QPushButton[isDisplayingNames="true"] { border-color: %9; }
        QPushButton#setPushButton1, QPushButton#setPushButton2, QPushButton#setPushButton3, QPushButton#setPushButton4,
        QPushButton#setPushButton5, QPushButton#setPushButton6, QPushButton#setPushButton7, QPushButton#setPushButton8 {
            padding: 5px 7px; min-width: 18px;
        }
    )")
                            .arg(text, background, muted, surface, border, hover, selection, accentText)
                            .arg(primary)
                            .arg(primaryText)
                            .arg(primaryHover));
}
ModernShell::ModernShell(QWidget *mapping, QWidget *owner, LocalApi *api, AntiMicroSettings *settings,
                         ApplicationContext *context, std::function<QJsonObject(const QJsonObject &)> handler)
    : QWidget(owner)
    , m_api(api)
    , m_settings(settings)
    , m_handler(std::move(handler))
{
    setObjectName("troaShell");
    qApp->setStyle(QStyleFactory::create("Fusion"));
    QFont font = qApp->font();
    font.setFamily("Segoe UI");
    qApp->setFont(font);
    auto root = new QVBoxLayout(this);
    root->setContentsMargins(18, 16, 18, 18);
    root->setSpacing(16);
    auto header = new QHBoxLayout;
    header->setSpacing(12);
    auto logo = new QLabel;
    logo->setPixmap(applicationIcon().pixmap(48, 48));
    logo->setAccessibleName("TROA logo");
    header->addWidget(logo);
    auto titles = new QVBoxLayout;
    titles->setSpacing(2);
    titles->addWidget(label("TROA GAMING SOFTWARE", "eyebrow"));
    titles->addWidget(label("Bifrost Controller", "title"));
    header->addLayout(titles, 1);
    m_status = label("No controller connected", "status");
    m_status->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_status->setMaximumWidth(210);
    header->addWidget(m_status);
    auto mcp = button("Assistant · MCP");
    mcp->setToolTip("Connect an AI app to this mapper. Shortcut: Ctrl+Shift+M");
    connect(mcp, &QPushButton::clicked, this, &ModernShell::showAssistant);
    header->addWidget(mcp);
    root->addLayout(header);

    auto body = new QHBoxLayout;
    body->setSpacing(16);
    auto sidebar = new QWidget;
    sidebar->setObjectName("troaSidebar");
    sidebar->setFixedWidth(176);
    auto navigation = new QVBoxLayout(sidebar);
    navigation->setContentsMargins(10, 16, 10, 14);
    navigation->setSpacing(7);
    navigation->addWidget(label("WORKSPACE", "eyebrow"));
    m_pages = new QStackedWidget;
    m_pages->addWidget(scrollPage(startPage()));

    auto mappingPage = new QWidget;
    auto mappingLayout = new QVBoxLayout(mappingPage);
    mappingLayout->setContentsMargins(0, 0, 0, 0);
    mappingLayout->setSpacing(12);
    mappingLayout->addWidget(label("Map controls", "section"));
    mappingLayout->addWidget(label(
        "Choose a controller tab, then click an input to assign an action. Save your profile when you're done.", "muted"));
    auto applicationStatus = label("Application context", "heading");
    mappingLayout->addWidget(applicationStatus);
    connect(context, &ApplicationContext::changed, this, [context, applicationStatus]() {
        const auto state = context->state();
        QStringList lines;
        const auto app = QFileInfo(state.value("executable").toString()).fileName();
        lines.append((state.value("mapper_focused").toBool() ? "Last focused: " : "Focused: ") +
                     (app.isEmpty() ? "No application detected" : app));
        for (const auto &value : state.value("controllers").toArray())
        {
            const auto device = value.toObject();
            lines.append(device.value("controller").toString() + " · " + device.value("active_profile").toString() + " · " +
                         device.value("active_mode").toString());
        }
        applicationStatus->setText(lines.join("\n"));
    });
    auto shortcuts = new QHBoxLayout;
    auto profiles = button("Browse profiles", true);
    connect(profiles, &QPushButton::clicked, this, [this]() { navigate(2); });
    shortcuts->addWidget(profiles);
    auto applications = button("Application rules");
    connect(applications, &QPushButton::clicked, this, [this]() { navigate(4); });
    shortcuts->addWidget(applications);
    auto rescan = button("Rescan");
    connect(rescan, &QPushButton::clicked, owner, [owner]() {
        if (auto action = owner->findChild<QAction *>("actionUpdate_Joysticks"))
            action->trigger();
    });
    shortcuts->addWidget(rescan);
    auto tools = new QToolButton;
    tools->setText("Controller tools");
    tools->setPopupMode(QToolButton::InstantPopup);
    auto menu = new QMenu(tools);
    for (const auto &name : {"actionCalibration", "actionProperties", "actionKeyValue"})
        if (auto action = owner->findChild<QAction *>(name))
            menu->addAction(action);
    tools->setMenu(menu);
    shortcuts->addWidget(tools);
    shortcuts->addStretch();
    mappingLayout->addLayout(shortcuts);
    mapping->setProperty("role", "card");
    mappingLayout->addWidget(mapping, 1);
    m_pages->addWidget(mappingPage);
    m_pages->addWidget(scrollPage(libraryPage()));
    m_pages->addWidget(scrollPage(assistantPage()));
    m_pages->addWidget(scrollPage(new ApplicationPage(context, settings)));

    const QStringList names = {"Overview", "Map controls", "Profile library", "Assistant · MCP", "App rules"};
    for (int index = 0; index < names.size(); ++index)
    {
        auto control = button(names.at(index));
        control->setProperty("role", "nav");
        control->setCheckable(true);
        control->setAutoExclusive(true);
        control->setMinimumHeight(24);
        if (index == 1)
            control->setIcon(controllerIcon());
        m_navigation.append(control);
        connect(control, &QPushButton::clicked, this, [this, index]() { navigate(index); });
    }
    for (int index : {0, 1, 2, 4, 3})
        navigation->addWidget(m_navigation.at(index));
    navigation->addStretch();
    auto options = button("Settings");
    connect(options, &QPushButton::clicked, owner, [owner]() {
        if (auto action = owner->findChild<QAction *>("actionOptions"))
            action->trigger();
    });
    navigation->addWidget(options);
    navigation->addWidget(label("APPEARANCE", "eyebrow"));
    auto appearance = new QHBoxLayout;
    appearance->setSpacing(6);
    auto appearanceGroup = new QButtonGroup(this);
    const bool dark = settings->value("TROA/DarkAppearance", false).toBool();
    for (int index = 0; index < 2; ++index)
    {
        auto mode = button(index == 0 ? "Light" : "Dark");
        mode->setCheckable(true);
        mode->setChecked(index == (dark ? 1 : 0));
        mode->setAccessibleName(index == 0 ? "Use light appearance" : "Use dark appearance");
        appearanceGroup->addButton(mode, index);
        appearance->addWidget(mode);
        connect(mode, &QPushButton::clicked, this, [this, index]() {
            {
                QMutexLocker lock(m_settings->getLock());
                m_settings->setValue("TROA/DarkAppearance", index == 1);
                m_settings->sync();
            }
            applyAppearance(index == 1);
        });
    }
    applyAppearance(dark);
    navigation->addLayout(appearance);
    navigation->addWidget(label("Preview " + QCoreApplication::applicationVersion(), "muted"));
    body->addWidget(sidebar);
    body->addWidget(m_pages, 1);
    root->addLayout(body, 1);
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        refreshControllers();
        refreshAssistantStatus();
    });
    timer->start(2000);
    refreshProfiles();
    refreshControllers();
    refreshAssistantStatus();
    auto updateOverview = [this, context]() {
        const auto state = context->state();
        const auto executable = state.value("executable").toString();
        const auto application = QFileInfo(executable).fileName();
        m_homeFocus->setText(
            (state.value("mapper_focused").toBool() ? "Last focused application: " : "Focused application: ") +
            (application.isEmpty() ? "No application detected" : application));
        m_homeFocus->setToolTip(executable);
        const auto devices = state.value("controllers").toArray();
        const auto signature = QJsonDocument(devices).toJson(QJsonDocument::Compact);
        if (m_homeControllers->property("state").toByteArray() == signature)
            return;
        m_homeControllers->setProperty("state", signature);
        m_homeControllers->setRowCount(devices.size());
        for (int i = 0; i < devices.size(); ++i)
        {
            const auto device = devices.at(i).toObject();
            const QStringList values = {device.value("controller").toString(),
                                        device.value("active_profile").toString() + " · " +
                                            device.value("active_mode").toString(),
                                        device.value("unsaved_changes").toBool() ? "Unsaved edits — save your mapping"
                                                                                 : device.value("message").toString()};
            for (int column = 0; column < values.size(); ++column)
            {
                auto item = new QTableWidgetItem(values.at(column));
                item->setToolTip(values.at(column));
                m_homeControllers->setItem(i, column, item);
            }
        }
        m_homeControllers->resizeRowsToContents();
        m_homeControllers->setVisible(!devices.isEmpty());
    };
    connect(context, &ApplicationContext::changed, this, updateOverview);
    updateOverview();
    navigate(0);
    m_catalogNetwork = new QNetworkAccessManager(this);
    if (m_settings->value("TROA/UpdateCommunityProfiles", true).toBool())
        QTimer::singleShot(0, this, [this]() { updateCommunityProfiles(); });
}
void ModernShell::navigate(int index)
{
    if (!m_pages || index < 0 || index >= m_pages->count())
        return;
    m_pages->setCurrentIndex(index);
    if (index < m_navigation.size())
        m_navigation.at(index)->setChecked(true);
    if (index == 2)
    {
        refreshProfiles();
        refreshControllers();
    }
    if (index == 3)
        refreshAssistantStatus();
}
void ModernShell::showAssistant() { navigate(3); }
QWidget *ModernShell::startPage()
{
    auto page = new QWidget;
    auto root = new QVBoxLayout(page);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(16);
    root->addWidget(label("Your controller workspace", "section"));
    root->addWidget(label("See what is active, then choose the task you want to work on.", "muted"));
    auto connection = card();
    auto connectionLayout = cardLayout(connection);
    auto row = new QHBoxLayout;
    auto icon = new QLabel;
    icon->setPixmap(controllerIcon().pixmap(40, 40));
    row->addWidget(icon);
    auto details = new QVBoxLayout;
    m_startStatus = label("Connect a controller", "heading");
    m_startDetail = label("Plug it in with USB, or pair it in Windows Bluetooth settings.", "muted");
    details->addWidget(m_startStatus);
    details->addWidget(m_startDetail);
    row->addLayout(details, 1);
    auto rescan = button("Rescan");
    connect(rescan, &QPushButton::clicked, this, [this]() {
        if (auto action = window()->findChild<QAction *>("actionUpdate_Joysticks"))
            action->trigger();
    });
    row->addWidget(rescan);
    connectionLayout->addLayout(row);
    m_homeFocus = label("Focused application", "muted");
    connectionLayout->addWidget(m_homeFocus);
    m_homeControllers = new QTableWidget(0, 3);
    m_homeControllers->setHorizontalHeaderLabels({"Controller", "Active profile / layout", "Status"});
    m_homeControllers->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_homeControllers->setSelectionMode(QAbstractItemView::NoSelection);
    m_homeControllers->setShowGrid(false);
    m_homeControllers->verticalHeader()->hide();
    m_homeControllers->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_homeControllers->setMinimumHeight(145);
    m_homeControllers->setWordWrap(true);
    m_homeControllers->setAccessibleName("Actual active profiles for connected controllers");
    connectionLayout->addWidget(m_homeControllers);
    root->addWidget(connection);
    const QList<QPair<QString, QString>> tasks = {
        {"Choose a profile", "Browse desktop, browser, and game templates. Review the inputs and layout before using one."},
        {"Customize controls", "Edit button, stick, trigger, touchpad, and motion mappings exposed by your controller."},
        {"Set profiles for apps",
         "Choose the profile used when an app is focused, and a shortcut to switch layouts within it."}};
    const QList<int> targets = {2, 1, 4};
    for (int i = 0; i < tasks.size(); ++i)
    {
        auto tile = card();
        auto layout = new QHBoxLayout(tile);
        layout->setContentsMargins(18, 14, 18, 14);
        auto descriptions = new QVBoxLayout;
        descriptions->addWidget(label(tasks.at(i).first, "heading"));
        descriptions->addWidget(label(tasks.at(i).second, "muted"));
        layout->addLayout(descriptions, 1);
        auto open = button(i == 0 ? "Browse profiles" : i == 1 ? "Map controls" : "App rules", i == 0);
        connect(open, &QPushButton::clicked, this, [this, targets, i]() { navigate(targets.at(i)); });
        layout->addWidget(open);
        root->addWidget(tile);
    }
    auto footer = new QHBoxLayout;
    auto help = button("How to get started");
    connect(help, &QPushButton::clicked, this, [this]() {
        QMessageBox::information(
            this, "Get started",
            "1. Connect your controller, then choose a template in Profile library.\n\n"
            "2. Review its layouts and assignments, choose your controller, then press Use this profile.\n\n"
            "3. Customize inputs in Map controls. Save a copy to keep a personal mapping file.\n\n"
            "4. Add an App rule using that saved file to switch profiles automatically and cycle layouts.\n\n"
            "Assistant · MCP connects an AI app to help create and edit library profiles. App updates prompt you separately "
            "from community-template updates.");
    });
    footer->addWidget(help);
    auto assistant = button("Connect an assistant · MCP");
    connect(assistant, &QPushButton::clicked, this, &ModernShell::showAssistant);
    footer->addWidget(assistant);
    footer->addStretch();
    root->addLayout(footer);
    root->addStretch();
    return page;
}
QWidget *ModernShell::libraryPage()
{
    auto page = new QWidget;
    auto root = new QVBoxLayout(page);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(12);
    root->addWidget(label("Profile library", "section"));
    root->addWidget(label("Review a template and its layouts, then choose the controller that will use it. Library copies "
                          "update separately from saved mapping files.",
                          "muted"));
    auto updates = new QHBoxLayout;
    m_updateProfiles = button("Update profiles");
    connect(m_updateProfiles, &QPushButton::clicked, this, &ModernShell::updateCommunityProfiles);
    updates->addWidget(m_updateProfiles);
    auto startup = new QCheckBox("Refresh templates at startup");
    startup->setChecked(m_settings->value("TROA/UpdateCommunityProfiles", true).toBool());
    connect(startup, &QCheckBox::toggled, this, [this](bool on) {
        QMutexLocker lock(m_settings->getLock());
        m_settings->setValue("TROA/UpdateCommunityProfiles", on);
        m_settings->sync();
    });
    updates->addWidget(startup);
    updates->addStretch();
    root->addLayout(updates);
    m_catalogStatus =
        label("Templates update independently of the app. Personal copies and active mappings are kept.", "muted");
    root->addWidget(m_catalogStatus);
    m_search = new QLineEdit;
    m_search->setPlaceholderText("Search profiles");
    m_search->setAccessibleName("Search profiles by name or category");
    root->addWidget(m_search);
    auto split = new QBoxLayout(QBoxLayout::LeftToRight);
    m_librarySplit = split;
    split->setSpacing(14);
    m_profiles = new QListWidget;
    m_profiles->setAccessibleName("Profile library");
    m_profiles->setMinimumWidth(190);
    m_profiles->setMinimumHeight(230);
    split->addWidget(m_profiles, 1);
    auto detail = card();
    detail->setMinimumWidth(0);
    auto layout = cardLayout(detail);
    m_profileName = label("Choose a profile", "heading");
    m_profileDescription = label("", "muted");
    m_profileMeta = label("", "muted");
    layout->addWidget(m_profileName);
    layout->addWidget(m_profileDescription);
    layout->addWidget(m_profileMeta);
    m_layout = new QComboBox;
    m_layout->setAccessibleName("Layout to preview and apply");
    m_layout->setToolTip("This layout is selected when you press Use this profile.");
    layout->addWidget(m_layout);
    connect(m_layout, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) { previewProfile(); });
    m_bindings = new QTableWidget(0, 3);
    m_bindings->setHorizontalHeaderLabels({"Control", "Action", "Purpose"});
    m_bindings->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_bindings->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_bindings->setShowGrid(false);
    m_bindings->verticalHeader()->hide();
    m_bindings->verticalHeader()->setDefaultSectionSize(36);
    m_bindings->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_bindings->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_bindings->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_bindings->horizontalHeader()->setMaximumSectionSize(240);
    m_bindings->setMinimumWidth(0);
    m_bindings->setMinimumHeight(210);
    m_bindings->setAccessibleName("Profile controller assignments");
    layout->addWidget(m_bindings, 1);
    split->addWidget(detail, 3);
    root->addLayout(split, 1);
    connect(m_profiles, &QListWidget::currentRowChanged, this, [this](int) { previewProfile(); });
    connect(m_search, &QLineEdit::textChanged, this, [this]() { filterProfiles(); });
    m_controllerHelp = label("", "muted");
    root->addWidget(m_controllerHelp);
    auto actions = new QVBoxLayout;
    m_controller = new QComboBox;
    m_controller->setAccessibleName("Controller to receive profile");
    m_controller->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_controller->setMinimumContentsLength(18);
    actions->addWidget(m_controller);
    m_apply = button("Use this profile", true);
    m_copy = button("Copy to my library");
    m_copy->setToolTip("Keep an independent library definition for editing with your assistant. Use Map controls > Save a "
                       "copy for a native mapping file.");
    auto actionButtons = new QHBoxLayout;
    actionButtons->addWidget(m_apply);
    actionButtons->addWidget(m_copy);
    actionButtons->addStretch();
    actions->addLayout(actionButtons);
    root->addLayout(actions);
    connect(m_controller, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { previewProfile(); });
    connect(m_apply, &QPushButton::clicked, this, &ModernShell::applyProfile);
    connect(m_copy, &QPushButton::clicked, this, &ModernShell::copyProfile);
    m_libraryFeedback = label("");
    m_libraryFeedback->hide();
    root->addWidget(m_libraryFeedback);
    root->addWidget(label("Using a profile changes this controller's assignments. Save any work in Map controls first. "
                          "Personal copies are kept when community templates update.",
                          "muted"));
    return page;
}
QWidget *ModernShell::assistantPage()
{
    auto page = new QWidget;
    auto root = new QVBoxLayout(page);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(16);
    root->addWidget(label("Assistant · MCP", "section"));
    root->addWidget(label("Connect your AI app to this mapper so you can create controller profiles together.", "muted"));
    auto access = card();
    auto accessLayout = cardLayout(access);
    auto enabled = new QCheckBox("Enable local MCP access");
    enabled->setChecked(m_api->isEnabled());
    accessLayout->addWidget(enabled);
    m_mcpStatus = label("", "heading");
    m_mcpDetail = label("", "muted");
    accessLayout->addWidget(m_mcpStatus);
    accessLayout->addWidget(m_mcpDetail);
    accessLayout->addWidget(label("Access is limited to your local user account. Turn it off here at any time.", "muted"));
    connect(enabled, &QCheckBox::toggled, this, [this, enabled](bool on) {
        const bool success = m_api->setEnabled(on);
        m_settings->setValue("TROA/AssistantAccess", on && success);
        if (!success)
        {
            enabled->blockSignals(true);
            enabled->setChecked(false);
            enabled->blockSignals(false);
            feedback(m_setupFeedback, "Could not enable local MCP access: " + m_api->errorString(), true);
        }
        refreshAssistantStatus();
    });
    root->addWidget(access);

    auto connection = card();
    auto layout = cardLayout(connection);
    layout->addWidget(
        step("1", "Add the mapper in your AI app",
             "Add a local stdio MCP server using the settings below. Local access alone does not connect an AI app."));
    auto pathRow = new QHBoxLayout;
    auto path = new QLineEdit(companionPath());
    path->setReadOnly(true);
    path->setAccessibleName("MCP server executable");
    pathRow->addWidget(path, 1);
    auto copyPath = button("Copy path");
    connect(copyPath, &QPushButton::clicked, this, [this]() {
        qApp->clipboard()->setText(companionPath());
        feedback(m_setupFeedback, "Server path copied.");
    });
    pathRow->addWidget(copyPath);
    layout->addLayout(pathRow);
    m_configFormat = new QComboBox;
    m_configFormat->addItems({"JSON · General MCP clients", "TOML · Codex"});
    m_configFormat->setAccessibleName("MCP configuration format");
    layout->addWidget(m_configFormat);
    m_configText = new QPlainTextEdit;
    m_configText->setReadOnly(true);
    m_configText->setAccessibleName("MCP connection settings");
    m_configText->setMinimumHeight(130);
    m_configText->setMaximumHeight(170);
    layout->addWidget(m_configText);
    connect(m_configFormat, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this]() { updateConnectionSettings(); });
    updateConnectionSettings();
    auto copy = button("Copy connection settings", true);
    connect(copy, &QPushButton::clicked, this, &ModernShell::copyConnectionSettings);
    layout->addWidget(copy, 0, Qt::AlignLeft);
    layout->addWidget(step("2", "Reload your AI app's MCP connection",
                           "Keep this mapper open. Reload the MCP server in your AI app after adding its settings."));
    layout->addWidget(step("3", "Create a profile together",
                           "Ask your assistant to list your controllers and profiles, then help you save a personal "
                           "profile. Review its assignments before applying it."));
    auto prompt = button("Copy a starter request");
    connect(prompt, &QPushButton::clicked, this, [this]() {
        qApp->clipboard()->setText(
            "Use the TROA controller mapper tools to list my controllers and profiles. Help me create a personal desktop or "
            "game profile. Save the draft and show me its assignments before applying it.");
        feedback(m_setupFeedback, "Starter request copied. Paste it into your connected AI app.");
    });
    layout->addWidget(prompt, 0, Qt::AlignLeft);
    root->addWidget(connection);
    m_setupFeedback = label("");
    m_setupFeedback->hide();
    root->addWidget(m_setupFeedback);
    auto files = button("Open personal profile folder");
    connect(files, &QPushButton::clicked, this, []() {
        QDir().mkpath(profileDirectory());
        QDesktopServices::openUrl(QUrl::fromLocalFile(profileDirectory()));
    });
    root->addWidget(files, 0, Qt::AlignLeft);
    root->addWidget(label("MCP can read, create, save, restore, and apply profiles. Saving a draft leaves your controller's "
                          "active mapping as it is.",
                          "muted"));
    root->addStretch();
    return page;
}
void ModernShell::updateConnectionSettings()
{
    if (m_configFormat->currentIndex() == 0)
        m_configText->setPlainText(connectionJson());
    else
    {
        QString escaped = companionPath();
        escaped.replace("\\", "\\\\");
        escaped.replace("\"", "\\\"");
        m_configText->setPlainText("[mcp_servers.bifrost-controller]\ncommand = \"" + escaped + "\"\nargs = []\n");
    }
}
void ModernShell::copyConnectionSettings()
{
    qApp->clipboard()->setText(m_configText ? m_configText->toPlainText() : connectionJson());
    if (m_setupFeedback)
        feedback(m_setupFeedback, "Connection settings copied. Paste them into your AI app's MCP configuration.");
}
void ModernShell::refreshAssistantStatus()
{
    if (!m_mcpStatus)
        return;
    if (!QFileInfo::exists(companionPath()))
    {
        m_mcpStatus->setText("MCP companion is missing");
        m_mcpDetail->setText("Install the full TROA package to include its MCP server.");
    } else if (!m_api->isEnabled())
    {
        m_mcpStatus->setText("Local MCP access is off");
        m_mcpDetail->setText("Enable it above when you want an AI app to access your profiles.");
    } else if (m_api->lastRequestAt().isValid())
    {
        m_mcpStatus->setText("MCP requests received");
        m_mcpDetail->setText("Last request at " + m_api->lastRequestAt().toString("h:mm AP") +
                             ". Keep the mapper open while using your AI app.");
    } else
    {
        m_mcpStatus->setText("Available locally · waiting for an AI app");
        m_mcpDetail->setText(
            "No MCP request has been received in this session. Add the server to your AI app using the steps below.");
    }
}
QString ModernShell::selectedId() const
{
    auto item = m_profiles ? m_profiles->currentItem() : nullptr;
    return item && !item->isHidden() ? item->data(Qt::UserRole).toString() : QString{};
}
void ModernShell::updateCommunityProfiles()
{
    if (!m_catalogNetwork || m_catalogReply)
        return;
    m_updateProfiles->setEnabled(false);
    m_catalogStatus->setText("Checking for community profiles…");
    const QUrl url("https://raw.githubusercontent.com/edwardsong08/bifrost-controller/"
                   "codex/troa-controller-mapper/profiles/catalog-v2.json");
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);
    request.setRawHeader("User-Agent", "Bifrost-Controller-Community-Profiles");
    request.setRawHeader("Cache-Control", "no-cache");
    auto reply = m_catalogNetwork->get(request);
    m_catalogReply = reply;
    reply->setReadBufferSize(256 * 1024 + 1);
    auto timeout = new QTimer(reply);
    timeout->setSingleShot(true);
    connect(timeout, &QTimer::timeout, reply, &QNetworkReply::abort);
    timeout->start(15000);
    connect(reply, &QNetworkReply::readyRead, this, [reply]() {
        auto bytes = reply->property("catalog_bytes").toByteArray();
        bytes += reply->readAll();
        reply->setProperty("catalog_bytes", bytes);
        if (bytes.size() > 256 * 1024)
            reply->abort();
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply, timeout, url]() {
        timeout->stop();
        auto bytes = reply->property("catalog_bytes").toByteArray();
        bytes += reply->readAll();
        const bool delivered = reply->error() == QNetworkReply::NoError && reply->url() == url &&
                               reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200;
        if (!delivered)
            m_catalogStatus->setText(
                "Could not reach community updates. Saved templates remain available; try Update profiles later.");
        else
        {
            const auto result = ProfileStore::installCatalog(bytes);
            if (result.contains("error"))
                m_catalogStatus->setText(result.value("error").toString());
            else
            {
                refreshProfiles();
                m_catalogStatus->setText(
                    QString("%1 community templates %2. Your personal profiles and active mappings were kept.")
                        .arg(result.value("count").toInt())
                        .arg(result.value("changed").toBool() ? "downloaded" : "are up to date"));
            }
        }
        m_catalogReply = nullptr;
        m_updateProfiles->setEnabled(true);
        reply->deleteLater();
    });
}
void ModernShell::refreshProfiles()
{
    const auto selected = selectedId();
    m_profiles->blockSignals(true);
    m_profiles->clear();
    int selectedRow = -1;
    for (const auto &value : ProfileStore().list())
    {
        const auto profile = value.toObject();
        QString category = profile.value("category").toString();
        if (!category.isEmpty())
            category[0] = category.at(0).toUpper();
        auto item = new QListWidgetItem(profile.value("name").toString() + "\n" + category +
                                            (profile.value("bundled").toBool() ? " · Community" : " · Personal"),
                                        m_profiles);
        item->setData(Qt::UserRole, profile.value("id"));
        if (profile.value("id").toString() == selected)
            selectedRow = m_profiles->count() - 1;
    }
    m_profiles->setCurrentRow(selectedRow);
    m_profiles->blockSignals(false);
    filterProfiles();
}
void ModernShell::filterProfiles()
{
    m_profiles->blockSignals(true);
    int firstVisible = -1;
    for (int row = 0; row < m_profiles->count(); ++row)
    {
        auto item = m_profiles->item(row);
        item->setHidden(!item->text().contains(m_search->text(), Qt::CaseInsensitive));
        if (!item->isHidden() && firstVisible < 0)
            firstVisible = row;
    }
    if (selectedId().isEmpty())
        m_profiles->setCurrentRow(firstVisible);
    m_profiles->blockSignals(false);
    previewProfile();
}
void ModernShell::chooseTemplate(const QString &id)
{
    m_search->clear();
    navigate(2);
    for (int row = 0; row < m_profiles->count(); ++row)
        if (m_profiles->item(row)->data(Qt::UserRole).toString() == id)
            m_profiles->setCurrentRow(row);
}
void ModernShell::previewProfile()
{
    const auto item = ProfileStore().read(selectedId());
    const auto profile = item.value("profile").toObject();
    m_bindings->setRowCount(0);
    if (item.contains("error"))
    {
        m_profileName->setText("Choose a profile");
        m_profileDescription->setText(m_search->text().isEmpty() ? "Select a profile to see its assignments."
                                                                 : "No profiles match this search.");
        m_profileMeta->clear();
    } else
    {
        m_profileName->setText(profile.value("name").toString());
        m_profileDescription->setText(profile.value("description").toString());
        auto bindings = profile.value("bindings").toArray();
        const auto layouts = profile.value("layouts").toArray();
        const int selectedSet =
            m_layout->property("profile_id").toString() == selectedId() ? m_layout->currentData().toInt() : 1;
        m_layout->blockSignals(true);
        m_layout->clear();
        if (layouts.isEmpty())
            m_layout->addItem("Layout 1", 1);
        else
            for (const auto &value : layouts)
            {
                const auto entry = value.toObject();
                m_layout->addItem(entry.value("name").toString(), entry.value("set").toInt());
            }
        m_layout->setCurrentIndex(qMax(0, m_layout->findData(selectedSet)));
        m_layout->setProperty("profile_id", selectedId());
        m_layout->setVisible(!layouts.isEmpty());
        m_layout->blockSignals(false);
        for (const auto &value : layouts)
            if (value.toObject().value("set").toInt() == m_layout->currentData().toInt())
                bindings = value.toObject().value("bindings").toArray();
        const auto family = m_controller && !m_controller->currentData().toString().isEmpty()
                                ? m_controller->currentData(Qt::UserRole + 1).toString()
                                : profile.value("controller_family").toString("generic");
        m_profileMeta->setText(QString("%1 assignments · %2")
                                   .arg(bindings.size())
                                   .arg(item.value("bundled").toBool() ? "Community template" : "Personal profile"));
        for (const auto &value : bindings)
        {
            const auto binding = value.toObject();
            const int row = m_bindings->rowCount();
            m_bindings->insertRow(row);
            m_bindings->setItem(row, 0, new QTableWidgetItem(Troa::inputName(binding.value("input").toString(), family)));
            m_bindings->setItem(row, 1, new QTableWidgetItem(actionName(binding)));
            m_bindings->setItem(row, 2, new QTableWidgetItem(binding.value("label").toString()));
        }
    }
    updateProfileActions();
}
void ModernShell::updateProfileActions()
{
    const auto item = ProfileStore().read(selectedId());
    const bool selected = !item.contains("error") && !selectedId().isEmpty();
    m_copy->setEnabled(selected);
    const auto device = m_controller->currentData(Qt::UserRole + 2).toJsonObject();
    QString reason;
    if (device.isEmpty())
        reason = "Connect a controller with a standard layout to use this profile.";
    else if (selected)
    {
        const auto profile = item.value("profile").toObject();
        const auto wanted = profile.value("controller_family").toString("generic");
        if (wanted.startsWith("steam-") && wanted != device.value("controller_family").toString())
            reason = "Choose the matching physical Steam Controller model for this template.";
        else
        {
            auto layouts = profile.value("layouts").toArray();
            if (layouts.isEmpty())
                layouts.append(QJsonObject{{"bindings", profile.value("bindings")}});
            const auto available = device.value("available_inputs").toArray();
            QStringList missing;
            for (const auto &layout : layouts)
                for (const auto &value : layout.toObject().value("bindings").toArray())
                {
                    const auto input = value.toObject().value("input").toString();
                    if (!available.contains(input) && !missing.contains(input))
                        missing.append(input);
                }
            if (!missing.isEmpty())
                reason = "The connected controller does not expose: " + missing.join(", ") +
                         ". Choose a compatible template or configure its controller layout.";
        }
    }
    m_apply->setEnabled(selected && reason.isEmpty());
    m_apply->setToolTip(reason);
    m_controllerHelp->setText(reason.isEmpty() ? "Use this profile applies the selected layout to the controller below."
                                               : reason);
}
void ModernShell::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (!m_librarySplit || !m_profiles)
        return;
    const bool compact = width() < 1100;
    m_librarySplit->setDirection(compact ? QBoxLayout::TopToBottom : QBoxLayout::LeftToRight);
    m_profiles->setMinimumHeight(compact ? 130 : 230);
    m_profiles->setMaximumHeight(compact ? 180 : QWIDGETSIZE_MAX);
    m_profiles->setMaximumWidth(compact ? QWIDGETSIZE_MAX : 280);
}
void ModernShell::refreshControllers()
{
    const auto response = m_handler({{"method", "list_controllers"}});
    const auto devices = response.value("controllers").toArray();
    if (auto owner = window())
        for (const auto &name : {"actionProperties", "actionCalibration"})
            if (auto action = owner->findChild<QAction *>(name))
                action->setEnabled(!devices.isEmpty());
    m_status->setText(devices.isEmpty()
                          ? "No controller connected"
                          : QString("%1 controller%2 connected").arg(devices.size()).arg(devices.size() == 1 ? "" : "s"));
    m_startStatus->setText(devices.isEmpty() ? "Connect a controller" : "Connected controllers");
    m_startDetail->setText(devices.isEmpty()
                               ? "Plug it in with USB, or pair it in Windows Bluetooth settings."
                               : "Active mappings are shown below. Save unsaved edits before changing profiles.");
    const auto signature = QJsonDocument(devices).toJson(QJsonDocument::Compact);
    if (m_controller->property("devices").toByteArray() != signature)
    {
        m_controller->setProperty("devices", signature);
        const auto selected = m_controller->currentData().toString();
        m_controller->blockSignals(true);
        m_controller->clear();
        for (const auto &value : devices)
        {
            const auto device = value.toObject();
            if (!device.value("standard_layout").toBool())
                continue;
            m_controller->addItem(device.value("name").toString(), device.value("controller_id"));
            m_controller->setItemData(m_controller->count() - 1, device.value("controller_family"), Qt::UserRole + 1);
            m_controller->setItemData(m_controller->count() - 1, device, Qt::UserRole + 2);
        }
        const int index = m_controller->findData(selected);
        if (index >= 0)
            m_controller->setCurrentIndex(index);
        if (!m_controller->count())
            m_controller->addItem("No compatible controller connected", QString{});
        m_controller->blockSignals(false);
        previewProfile();
    }
    m_controllerHelp->setText(
        !m_controller->currentData().toString().isEmpty() ? "Choose which controller will use this profile."
        : devices.isEmpty()
            ? "Connect a controller to use a profile. You can browse and copy profiles now."
            : "This controller needs a standard layout. Open Controllers and choose Controller layout to configure it.");
    updateProfileActions();
}
void ModernShell::libraryFeedback(const QString &text, bool error) { feedback(m_libraryFeedback, text, error); }
void ModernShell::applyProfile()
{
    const auto item = ProfileStore().read(selectedId());
    if (item.contains("error"))
    {
        libraryFeedback(item.value("error").toString(), true);
        return;
    }
    const auto result = m_handler({{"method", "activate_profile"},
                                   {"arguments", QJsonObject{{"id", selectedId()},
                                                             {"controller_id", m_controller->currentData().toString()},
                                                             {"set", m_layout->currentData().toInt()},
                                                             {"expected_revision", item.value("revision")}}}});
    if (result.contains("error"))
        libraryFeedback(result.value("error").toString(), true);
    else
        libraryFeedback("Profile applied to " + m_controller->currentText() + " · " + m_layout->currentText() +
                        ". Open Map controls to view or customize its assignments. Use its layout buttons to view Space, "
                        "Ground, or Menus; the library layout selector previews a template until you apply it.");
}
void ModernShell::copyProfile()
{
    const auto item = ProfileStore().read(selectedId());
    if (item.contains("error"))
    {
        libraryFeedback(item.value("error").toString(), true);
        return;
    }
    bool ok = false;
    const auto name = QInputDialog::getText(this, "Make a personal copy", "Profile name:", QLineEdit::Normal,
                                            item.value("profile").toObject().value("name").toString() + " (personal)", &ok)
                          .trimmed();
    if (!ok || name.isEmpty())
        return;
    auto id = name.toLower();
    id.replace(QRegularExpression("[^a-z0-9]+"), "-");
    id.remove(QRegularExpression("^-|-$"));
    id = id.left(48);
    if (id.isEmpty() || id.startsWith("builtin-"))
        id = "my-profile";
    const auto base = id;
    int suffix = 2;
    while (!ProfileStore().read(id).contains("error"))
        id = base + "-" + QString::number(suffix++);
    auto profile = item.value("profile").toObject();
    profile.insert("id", id);
    profile.insert("name", name);
    const auto result = ProfileStore().save(profile, {});
    if (result.contains("error"))
    {
        libraryFeedback(result.value("error").toString(), true);
        return;
    }
    m_search->clear();
    refreshProfiles();
    for (int row = 0; row < m_profiles->count(); ++row)
        if (m_profiles->item(row)->data(Qt::UserRole).toString() == id)
            m_profiles->setCurrentRow(row);
    libraryFeedback("Personal copy created. Use this profile, then open Controllers to customize and save a mapping file. "
                    "To update this library definition, use your connected AI app.");
}
} // namespace Troa
