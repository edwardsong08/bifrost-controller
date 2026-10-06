// SPDX-License-Identifier: GPL-3.0-or-later
#include "modernshell.h"
#include "antimicrosettings.h"
#include "applicationcontext.h"
#include "applicationpage.h"
#include "identity.h"
#include "localapi.h"
#include "profilestore.h"

#include <QAction>
#include <QApplication>
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
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QPixmap>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
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
    return QDir::toNativeSeparators(QDir(QCoreApplication::applicationDirPath()).filePath("troa-controller-mcp.exe"));
#else
    return QDir(QCoreApplication::applicationDirPath()).filePath("troa-controller-mcp");
#endif
}
QString connectionJson()
{
    const QJsonObject config{{"mcpServers", QJsonObject{{"troa-controller-mapper", QJsonObject{{"command", companionPath()},
                                                                                               {"args", QJsonArray{}}}}}}};
    return QString::fromUtf8(QJsonDocument(config).toJson(QJsonDocument::Indented));
}
QString inputName(QString input, bool playStation)
{
    if (playStation)
    {
        const QMap<QString, QString> names = {
            {"a", "Cross (×)"},         {"b", "Circle (○)"},        {"x", "Square (□)"},        {"y", "Triangle (△)"},
            {"back", "Create / Share"}, {"start", "Options"},       {"guide", "PS button"},     {"left_shoulder", "L1"},
            {"right_shoulder", "R1"},   {"left_stick_press", "L3"}, {"right_stick_press", "R3"}};
        if (names.contains(input))
            return names.value(input);
    }
    if (input == "a" || input == "b" || input == "x" || input == "y")
        return input.toUpper() + " button";
    input.replace("dpad_", "D-pad ");
    input.replace("left_stick_press", "Left stick click");
    input.replace("right_stick_press", "Right stick click");
    input.replace("left_stick_", "Left stick ");
    input.replace("right_stick_", "Right stick ");
    input.replace("left_shoulder", "Left shoulder");
    input.replace("right_shoulder", "Right shoulder");
    input.replace('_', ' ');
    if (!input.isEmpty())
        input[0] = input.at(0).toUpper();
    return input;
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
        QMenu, QMenuBar { background: %4; }
        QMenu::item { padding: 8px 24px; }
        QMenu::item:selected { background: %7; }
        QScrollBar:vertical { background: transparent; width: 10px; }
        QScrollBar::handle:vertical { background: %5; border-radius: 5px; min-height: 24px; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        FlashButtonWidget[isflashing="true"] { background: %9; color: %10; border-color: %9; }
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
    titles->addWidget(label("TROA", "eyebrow"));
    titles->addWidget(label("PC Controller Mapper", "title"));
    header->addLayout(titles);
    header->addStretch();
    m_status = label("No controller connected", "status");
    header->addWidget(m_status);
    auto mcp = button("MCP setup");
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
    mappingLayout->addWidget(label("Controllers", "section"));
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
    for (const auto &name : {"actionCalibration", "actionProperties", "actionKeyValue", "actionOptions"})
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

    const QStringList names = {"Get started", "Controllers", "Profiles", "MCP & AI setup", "Applications"};
    for (int index = 0; index < names.size(); ++index)
    {
        auto control = button(names.at(index));
        control->setProperty("role", "nav");
        control->setCheckable(true);
        control->setAutoExclusive(true);
        control->setMinimumHeight(24);
        if (index == 1)
            control->setIcon(controllerIcon());
        navigation->addWidget(control);
        m_navigation.append(control);
        connect(control, &QPushButton::clicked, this, [this, index]() { navigate(index); });
    }
    navigation->addStretch();
    auto options = button("Settings");
    connect(options, &QPushButton::clicked, owner, [owner]() {
        if (auto action = owner->findChild<QAction *>("actionOptions"))
            action->trigger();
    });
    navigation->addWidget(options);
    navigation->addWidget(label("APPEARANCE", "eyebrow"));
    auto appearance = new QComboBox;
    appearance->addItems({"Light", "Dark"});
    appearance->setAccessibleName("Appearance");
    appearance->setCurrentIndex(settings->value("TROA/DarkAppearance", false).toBool() ? 1 : 0);
    applyAppearance(appearance->currentIndex() == 1);
    connect(appearance, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        m_settings->setValue("TROA/DarkAppearance", index == 1);
        applyAppearance(index == 1);
    });
    navigation->addWidget(appearance);
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
    navigate(0);
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
    root->addWidget(label("Make your controller feel at home.", "section"));
    root->addWidget(label("Map controller inputs to the keys and mouse actions you use on your PC.", "muted"));
    auto connection = card();
    auto row = new QHBoxLayout(connection);
    row->setContentsMargins(20, 18, 20, 18);
    row->setSpacing(16);
    auto icon = new QLabel;
    icon->setPixmap(controllerIcon().pixmap(56, 56));
    row->addWidget(icon);
    auto details = new QVBoxLayout;
    m_startStatus = label("Connect your controller", "heading");
    m_startDetail = label("Plug it in with USB, or pair it in Windows Bluetooth settings.", "muted");
    details->addWidget(m_startStatus);
    details->addWidget(m_startDetail);
    row->addLayout(details, 1);
    auto open = button("Open controllers");
    connect(open, &QPushButton::clicked, this, [this]() { navigate(1); });
    row->addWidget(open);
    root->addWidget(connection);
    auto guide = card();
    auto steps = cardLayout(guide);
    steps->addWidget(step("1", "Connect", "Your controller appears in Controllers. If it is missing, choose Rescan."));
    steps->addWidget(step("2", "Choose a starting profile",
                          "Use a desktop or browser template, or open one of your existing mapping files."));
    steps->addWidget(
        step("3", "Make it yours", "Click a controller input to change its action. Save a copy to keep your own setup."));
    root->addWidget(guide);
    auto starts = new QHBoxLayout;
    for (const auto &entry : QList<QPair<QString, QString>>{{"Desktop", "builtin-desktop"}, {"Browser", "builtin-browser"}})
    {
        auto tile = card();
        auto layout = cardLayout(tile);
        layout->addWidget(label(entry.first, "heading"));
        layout->addWidget(label(entry.first == "Desktop" ? "Pointer control, clicks, scrolling, and everyday shortcuts."
                                                         : "Tabs, the address bar, navigation, and scrolling.",
                                "muted"));
        auto choose = button("View " + entry.first.toLower() + " profile");
        connect(choose, &QPushButton::clicked, this, [this, entry]() { chooseTemplate(entry.second); });
        layout->addWidget(choose);
        starts->addWidget(tile, 1);
    }
    root->addLayout(starts);
    auto assistant = card();
    auto assistantLayout = cardLayout(assistant);
    assistantLayout->addWidget(label("Create profiles with your AI assistant", "heading"));
    assistantLayout->addWidget(label("MCP lets a compatible AI app read your controller setup and help you build profiles. "
                                     "Connect it once in MCP & AI setup.",
                                     "muted"));
    auto setup = button("Set up MCP", true);
    connect(setup, &QPushButton::clicked, this, &ModernShell::showAssistant);
    assistantLayout->addWidget(setup, 0, Qt::AlignLeft);
    root->addWidget(assistant);
    root->addStretch();
    return page;
}
QWidget *ModernShell::libraryPage()
{
    auto page = new QWidget;
    auto root = new QVBoxLayout(page);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(12);
    root->addWidget(label("Profiles", "section"));
    root->addWidget(
        label("Start with a community template. Make a personal copy when you want to keep your own version.", "muted"));
    m_search = new QLineEdit;
    m_search->setPlaceholderText("Search profiles");
    m_search->setAccessibleName("Search profiles by name or category");
    root->addWidget(m_search);
    auto split = new QHBoxLayout;
    split->setSpacing(14);
    m_profiles = new QListWidget;
    m_profiles->setAccessibleName("Profile library");
    m_profiles->setMinimumWidth(190);
    m_profiles->setMinimumHeight(230);
    split->addWidget(m_profiles, 1);
    auto detail = card();
    auto layout = cardLayout(detail);
    m_profileName = label("Choose a profile", "heading");
    m_profileDescription = label("", "muted");
    m_profileMeta = label("", "muted");
    layout->addWidget(m_profileName);
    layout->addWidget(m_profileDescription);
    layout->addWidget(m_profileMeta);
    m_layout = new QComboBox;
    m_layout->setAccessibleName("Profile layout to preview");
    layout->addWidget(m_layout);
    connect(m_layout, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) { previewProfile(); });
    m_bindings = new QTableWidget(0, 3);
    m_bindings->setHorizontalHeaderLabels({"Control", "Action", "Purpose"});
    m_bindings->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_bindings->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_bindings->setShowGrid(false);
    m_bindings->verticalHeader()->hide();
    m_bindings->verticalHeader()->setDefaultSectionSize(36);
    m_bindings->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_bindings->setMinimumHeight(210);
    m_bindings->setAccessibleName("Profile controller assignments");
    layout->addWidget(m_bindings, 1);
    split->addWidget(detail, 3);
    root->addLayout(split, 1);
    connect(m_profiles, &QListWidget::currentRowChanged, this, [this](int) { previewProfile(); });
    connect(m_search, &QLineEdit::textChanged, this, [this]() { filterProfiles(); });
    m_controllerHelp = label("", "muted");
    root->addWidget(m_controllerHelp);
    auto actions = new QHBoxLayout;
    m_controller = new QComboBox;
    m_controller->setAccessibleName("Controller to receive profile");
    actions->addWidget(m_controller, 1);
    m_apply = button("Use this profile", true);
    m_copy = button("Make a personal copy");
    actions->addWidget(m_apply);
    actions->addWidget(m_copy);
    root->addLayout(actions);
    connect(m_controller, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { previewProfile(); });
    connect(m_apply, &QPushButton::clicked, this, &ModernShell::applyProfile);
    connect(m_copy, &QPushButton::clicked, this, &ModernShell::copyProfile);
    m_libraryFeedback = label("");
    m_libraryFeedback->hide();
    root->addWidget(m_libraryFeedback);
    root->addWidget(label("Using a profile changes this controller's assignments. Save any work in Controllers first. "
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
    root->addWidget(label("MCP & AI setup", "section"));
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
        m_configText->setPlainText("[mcp_servers.troa-controller-mapper]\ncommand = \"" + escaped + "\"\nargs = []\n");
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
        const bool playStation = m_controller && (m_controller->currentText().contains("DualSense", Qt::CaseInsensitive) ||
                                                  m_controller->currentText().contains("DualShock", Qt::CaseInsensitive) ||
                                                  m_controller->currentText().contains("PS4", Qt::CaseInsensitive) ||
                                                  m_controller->currentText().contains("PS5", Qt::CaseInsensitive));
        m_profileMeta->setText(QString("%1 assignments · %2")
                                   .arg(bindings.size())
                                   .arg(item.value("bundled").toBool() ? "Community template" : "Personal profile"));
        for (const auto &value : bindings)
        {
            const auto binding = value.toObject();
            const int row = m_bindings->rowCount();
            m_bindings->insertRow(row);
            m_bindings->setItem(row, 0, new QTableWidgetItem(inputName(binding.value("input").toString(), playStation)));
            m_bindings->setItem(row, 1, new QTableWidgetItem(actionName(binding)));
            m_bindings->setItem(row, 2, new QTableWidgetItem(binding.value("label").toString()));
        }
    }
    updateProfileActions();
}
void ModernShell::updateProfileActions()
{
    const bool selected = !selectedId().isEmpty();
    if (m_copy)
        m_copy->setEnabled(selected);
    if (m_apply)
        m_apply->setEnabled(selected && m_controller && !m_controller->currentData().toString().isEmpty());
}
void ModernShell::refreshControllers()
{
    const auto response = m_handler({{"method", "list_controllers"}});
    const auto devices = response.value("controllers").toArray();
    m_status->setText(devices.isEmpty()
                          ? "No controller connected"
                          : QString("%1 controller%2 connected").arg(devices.size()).arg(devices.size() == 1 ? "" : "s"));
    m_startStatus->setText(devices.isEmpty() ? "Connect your controller" : "Your controller is ready to configure");
    m_startDetail->setText(devices.isEmpty() ? "Plug it in with USB, or pair it in Windows Bluetooth settings."
                                             : "Choose a profile or open Controllers to customize its buttons and sticks.");
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
        }
        const int index = m_controller->findData(selected);
        if (index >= 0)
            m_controller->setCurrentIndex(index);
        if (!m_controller->count())
            m_controller->addItem("No compatible controller connected", QString{});
        m_controller->blockSignals(false);
        if (selected != m_controller->currentData().toString())
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
                                                             {"expected_revision", item.value("revision")}}}});
    if (result.contains("error"))
        libraryFeedback(result.value("error").toString(), true);
    else
        libraryFeedback("Profile applied to " + m_controller->currentText() +
                        ". Open Controllers to view or customize its assignments.");
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
