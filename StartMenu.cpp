#include "StartMenu.h"
#include "YGOSearch.h"
#include "PTCGSearch.h"
#include "GenshinSearch.h"
#include <QMenuBar>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QDialog>
#include <QLabel>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QTextEdit>
#include <QHBoxLayout>
#include <QToolButton>
#include <QSizePolicy>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QCursor>
#include <QPixmap>
#include <QColor>

class ImageCardButton : public QToolButton {
public:
    explicit ImageCardButton(const QString &imagePath, QWidget *parent = nullptr)
        : QToolButton(parent), background(imagePath) {
        if (background.isNull()) {
            qWarning() << "无法加载图鉴卡片背景图片:" << imagePath;
        }
        setAutoRaise(true);
        setCursor(Qt::PointingHandCursor);
        setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        {
            QPainter painter(this);
            painter.setRenderHint(QPainter::Antialiasing);

            QPainterPath clipPath;
            clipPath.addRoundedRect(rect(), 16, 16);
            painter.setClipPath(clipPath);

            const QPixmap scaled = background.scaled(
                size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            const QRect source((scaled.width() - width()) / 2,
                               (scaled.height() - height()) / 2,
                               width(),
                               height());
            painter.drawPixmap(rect(), scaled, source);
            painter.fillRect(rect(), QColor(5, 12, 24, 115));
        }

        QToolButton::paintEvent(event);
    }

private:
    QPixmap background;
};

StartMenu::StartMenu(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("集换式卡牌（TCG）搜索工具");
    resize(1040, 700);
    setMinimumSize(820, 600);
    setStyleSheet(R"(
        QMainWindow { background-color: #0b1120; }
        QMenuBar {
            color: #cbd5e1; background-color: #111827; border-bottom: 1px solid #263449;
            padding: 5px;
        }
        QMenuBar::item { padding: 7px 12px; background: transparent; border-radius: 5px; }
        QMenuBar::item:selected { color: #f8fafc; background-color: #1e293b; }
        QMenu { color: #e2e8f0; background-color: #111827; border: 1px solid #334155; }
        QMenu::item { padding: 7px 24px; }
        QMenu::item:selected { background-color: #2563eb; }
        QToolButton {
            color: #f8fafc; background: transparent;
            border: 1px solid rgba(255, 255, 255, 90); border-radius: 16px;
            padding: 22px 12px; font-size: 15px; font-weight: 700;
            text-align: center;
        }
        QToolButton:hover { border: 2px solid #60a5fa; }
    )");

    // 添加工具栏
    QMenuBar *menuBar = new QMenuBar(this);

    QMenu *helpMenu = menuBar->addMenu("帮助");
    QAction *ygoHelp = new QAction("游戏王", this);
    QAction *ptcgHelp = new QAction("宝可梦TCG", this);
    QAction *genshinHelp = new QAction("原神七圣召唤", this);

    helpMenu->addAction(ygoHelp);
    helpMenu->addAction(ptcgHelp);
    helpMenu->addAction(genshinHelp);

    connect(ygoHelp, &QAction::triggered, this, &StartMenu::showYGOHelp);
    connect(ptcgHelp, &QAction::triggered, this, &StartMenu::showPTCGHelp);
    connect(genshinHelp, &QAction::triggered, this, &StartMenu::showGenshinHelp);

    setMenuBar(menuBar);
    addToolMenu();

    // 关于菜单
    QMenu *aboutMenu = menuBar->addMenu("关于");
    QAction *aboutAction = new QAction("关于本软件", this);
    QAction *aboutAuthorAction = new QAction("关于作者", this);
    aboutMenu->addAction(aboutAction);
    aboutMenu->addAction(aboutAuthorAction);

    connect(aboutAction, &QAction::triggered, this, &StartMenu::showAboutDialog);
    connect(aboutAuthorAction, &QAction::triggered, this, &StartMenu::showAuthorInfo);

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setObjectName("landingPage");
    centralWidget->setStyleSheet("#landingPage { background-color: #0b1120; }");
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    layout->setContentsMargins(36, 32, 36, 28);
    layout->setSpacing(18);

    QLabel *eyebrowLabel = new QLabel("TCG SEARCHER  /  CARD LIBRARY", centralWidget);
    eyebrowLabel->setStyleSheet("color: #60a5fa; font-size: 10pt; font-weight: 700; letter-spacing: 2px;");
    layout->addWidget(eyebrowLabel);

    QLabel *titleLabel = new QLabel("集换式卡牌检索", centralWidget);
    titleLabel->setStyleSheet("color: #f8fafc; font-size: 28pt; font-weight: 700;");
    layout->addWidget(titleLabel);

    QLabel *subtitleLabel = new QLabel(
        "选择一个图鉴，搜索最新卡牌资料。在线数据持续更新，卡片详情双击查看。",
        centralWidget);
    subtitleLabel->setStyleSheet("color: #94a3b8; font-size: 11pt;");
    layout->addWidget(subtitleLabel);

    QFrame *line = new QFrame(centralWidget);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("color: #263449; background-color: #263449; max-height: 1px;");
    layout->addWidget(line);

    QHBoxLayout *cardsLayout = new QHBoxLayout;
    cardsLayout->setSpacing(18);

    ImageCardButton *ygoButton = new ImageCardButton(":/ygo_background.jpg", centralWidget);
    ygoButton->setObjectName("ygoCard");
    ygoButton->setText("游戏王\n实时卡查 · 卡片详情");
    QPixmap ygoPixmap(":/ygo_logo.png");
    ygoButton->setIcon(QIcon(ygoPixmap));
    ygoButton->setIconSize(QSize(150, 76));
    ygoButton->setMinimumHeight(270);
    ygoButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    cardsLayout->addWidget(ygoButton);

    ImageCardButton *ptcgButton = new ImageCardButton(":/ptcg_background.jpg", centralWidget);
    ptcgButton->setObjectName("ptcgCard");
    ptcgButton->setText("宝可梦 TCG\n实时图鉴 · 按系列检索");
    QPixmap ptcgPixmap(":/ptcg_logo.jpg");
    ptcgButton->setIcon(QIcon(ptcgPixmap));
    ptcgButton->setIconSize(QSize(150, 76));
    ptcgButton->setMinimumHeight(270);
    ptcgButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    cardsLayout->addWidget(ptcgButton);

    ImageCardButton *genshinButton = new ImageCardButton(":/genshin_background.jpeg", centralWidget);
    genshinButton->setObjectName("genshinCard");
    genshinButton->setText("原神 · 七圣召唤\n角色卡与行动卡");
    QPixmap genshinPixmap(":/genshin_logo.png");
    genshinButton->setIcon(QIcon(genshinPixmap));
    genshinButton->setIconSize(QSize(150, 76));
    genshinButton->setMinimumHeight(270);
    genshinButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    cardsLayout->addWidget(genshinButton);
    layout->addLayout(cardsLayout, 1);

    QLabel *footerLabel = new QLabel("三大卡牌图鉴  ·  HTTPS 在线数据  ·  双击搜索结果查看详情", centralWidget);
    footerLabel->setAlignment(Qt::AlignCenter);
    footerLabel->setStyleSheet("color: #64748b; font-size: 9pt;");
    layout->addWidget(footerLabel);

    connect(ygoButton, &QToolButton::clicked, this, &StartMenu::openYGOSearch);
    connect(ptcgButton, &QToolButton::clicked, this, &StartMenu::openPTCGSearch);
    connect(genshinButton, &QToolButton::clicked, this, &StartMenu::openGenshinSearch);

    setCentralWidget(centralWidget);
}

void StartMenu::addToolMenu() {
    QMenu *toolMenu = menuBar()->addMenu("工具");

    QAction *clearAction = new QAction("清空搜索结果", this);
    connect(clearAction, &QAction::triggered, this, &StartMenu::clearSearchResults);

    QAction *resetAction = new QAction("重置窗口布局", this);
    connect(resetAction, &QAction::triggered, this, &StartMenu::resetWindowLayout);

    toolMenu->addAction(clearAction);
    toolMenu->addAction(resetAction);
}

void StartMenu::clearSearchResults() {
    QMessageBox::information(this, "提示", "搜索结果已清空！");
}

void StartMenu::resetWindowLayout() {
    QMessageBox::information(this, "提示", "窗口布局已重置！");
}

// 显示“关于”对话框
void StartMenu::showAboutDialog() {
    // 创建对话框
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("关于本软件");
    dialog->resize(400, 300);

    // 创建不可编辑但可复制的文本框
    QTextEdit *textEdit = new QTextEdit(dialog);
    textEdit->setReadOnly(true); // 设置为只读
    textEdit->setText(
        "软件名称: TCG 搜索工具\n"
        "版本: 1.0.0\n"
        "描述: 这是一款用于搜索游戏王、宝可梦、原神卡片的工具。\n\n"
        "目前程序仍处于测试阶段，存在以下bug：\n"
        "    ——部分规则文本无法正确显示。\n"
        "    ——宝可梦TCG卡片信息显示比较混乱。\n"
        "    ——原神七圣召唤卡片信息颜色影响观感。\n"
        "敬请谅解！感谢您的支持与使用！\n"
        "您可以复制以上内容，但无法编辑。"
        );

    // 添加右键菜单来支持复制
    textEdit->setContextMenuPolicy(Qt::ActionsContextMenu); // 启用自定义右键菜单
    QAction *copyAction = new QAction("复制", textEdit); // 创建“复制”动作
    connect(copyAction, &QAction::triggered, textEdit, &QTextEdit::copy); // 连接复制动作
    textEdit->addAction(copyAction); // 将“复制”动作添加到右键菜单

    // 创建布局并添加文本框
    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->addWidget(textEdit);

    // 显示对话框
    dialog->setLayout(layout);
    dialog->exec();
}

void StartMenu::showAuthorInfo() {
    // 创建对话框
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("关于作者");
    dialog->resize(400, 300);

    // 创建不可编辑但可复制的文本框
    QTextEdit *textEdit = new QTextEdit(dialog);
    textEdit->setReadOnly(true); // 设置为只读
    textEdit->setText(
        "作者: 王子聪\n"
        "邮箱: 3370748131@qq.com\n"
        "学号：20243021211032\n"
        "B站主页：https://space.bilibili.com/518755367?spm_id_from=333.1007.0.0\n\n"
        "感谢您的支持！\n"
        "您可以复制以上内容，但无法编辑。"
        );

    // 添加右键菜单来支持复制
    textEdit->setContextMenuPolicy(Qt::ActionsContextMenu); // 启用自定义右键菜单
    QAction *copyAction = new QAction("复制", textEdit); // 创建“复制”动作
    connect(copyAction, &QAction::triggered, textEdit, &QTextEdit::copy); // 连接复制动作
    textEdit->addAction(copyAction); // 将“复制”动作添加到右键菜单

    // 创建布局并添加文本框
    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->addWidget(textEdit);

    // 显示对话框
    dialog->setLayout(layout);
    dialog->exec();
}

void StartMenu::showRulesFromTxt(const QString &title, const QString &filePath) {
    QDialog dialog(this);
    dialog.setWindowTitle(title);
    dialog.resize(600, 500);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QTreeWidget *treeWidget = new QTreeWidget(&dialog);
    treeWidget->setHeaderHidden(true); // 隐藏标题栏
    layout->addWidget(treeWidget);

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法打开规则文件：" + filePath);
        return;
    }

    QTextStream in(&file);
    QTreeWidgetItem *currentMain = nullptr;  // 主标题
    QTreeWidgetItem *currentSub = nullptr;  // 子标题
    QTreeWidgetItem *currentSubSub = nullptr;  // 子条目

    // 改进后的正则表达式
    QRegularExpression mainTitleRegex(R"(^\s*###\s*.*)");  // 主标题
    QRegularExpression subTitleRegex(R"(^\s*####\s*.*)");  // 子标题
    QRegularExpression subSubTitleRegex(R"(^\s*#####\s*.*)"); // 子条目
    QRegularExpression listItemRegex(R"(^\s*[\•\-\·]\s+.*)"); // 项目条目

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (mainTitleRegex.match(line).hasMatch()) {
            // 主标题：创建顶层节点
            currentMain = new QTreeWidgetItem(treeWidget);
            currentMain->setText(0, line);
            currentMain->setData(0, Qt::UserRole, line); // 保存详细内容
            currentSub = nullptr;  // 重置子标题
            currentSubSub = nullptr;  // 重置子条目
        } else if (subTitleRegex.match(line).hasMatch()) {
            // 子标题：添加到主标题下
            if (currentMain) {
                currentSub = new QTreeWidgetItem(currentMain);
                currentSub->setText(0, line);
                currentSub->setData(0, Qt::UserRole, line); // 保存详细内容
                currentSubSub = nullptr;  // 重置子条目
            }
        } else if (subSubTitleRegex.match(line).hasMatch()) {
            // 子条目：添加到子标题下
            if (currentSub) {
                currentSubSub = new QTreeWidgetItem(currentSub);
                currentSubSub->setText(0, line);
                currentSubSub->setData(0, Qt::UserRole, line); // 保存详细内容
            } else if (currentMain) {
                currentSubSub = new QTreeWidgetItem(currentMain);  // 无子标题时直接挂在主标题下
                currentSubSub->setText(0, line);
                currentSubSub->setData(0, Qt::UserRole, line); // 保存详细内容
            }
        } else if (listItemRegex.match(line).hasMatch()) {
            // 项目条目：添加到最近的有效节点
            QTreeWidgetItem *listItem = nullptr;
            if (currentSubSub) {
                listItem = new QTreeWidgetItem(currentSubSub);
            } else if (currentSub) {
                listItem = new QTreeWidgetItem(currentSub);
            } else if (currentMain) {
                listItem = new QTreeWidgetItem(currentMain);
            }

            if (listItem) {
                listItem->setText(0, line);
                listItem->setData(0, Qt::UserRole, line); // 保存详细内容
            }
        }
    }

    file.close();
    treeWidget->expandAll(); // 默认展开所有条目

    // 双击事件
    connect(treeWidget, &QTreeWidget::itemDoubleClicked, this, [=](QTreeWidgetItem *item, int) {
        QString detailText = item->data(0, Qt::UserRole).toString(); // 获取详细内容
        if (!detailText.isEmpty()) {
            QMessageBox::information(this, "详细内容", detailText, QMessageBox::Ok);
        }
    });

    dialog.exec();
}


void StartMenu::showYGOHelp() {
    showRulesFromTxt("游戏王规则", ":/rules/ygo_rule.txt");
}

void StartMenu::showPTCGHelp() {
    showRulesFromTxt("宝可梦TCG规则", ":/rules/ptcg_rule.txt");
}

void StartMenu::showGenshinHelp() {
    showRulesFromTxt("原神七圣召唤规则", ":/rules/genshin_rule.txt");
}

void StartMenu::showHelpDialog(const QString &title, const QMap<QString, QStringList> &sections) {
    QDialog dialog(this);
    dialog.setWindowTitle(title);
    dialog.resize(400, 300);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QTreeWidget *treeWidget = new QTreeWidget(&dialog);
    treeWidget->setHeaderHidden(true);

    for (auto it = sections.begin(); it != sections.end(); ++it) {
        QTreeWidgetItem *sectionItem = new QTreeWidgetItem(treeWidget, {it.key()});
        for (const QString &item : it.value()) {
            new QTreeWidgetItem(sectionItem, {item});
        }
    }

    treeWidget->expandAll(); // 默认展开所有条目
    layout->addWidget(treeWidget);

    dialog.exec();
}

void StartMenu::openYGOSearch() {
    try {
        auto *ygoSearch = new YGOSearch(this); // 创建子窗口，设置父窗口为主窗口
        ygoSearch->setAttribute(Qt::WA_DeleteOnClose); // 窗口关闭时自动释放资源
        ygoSearch->show();
    } catch (const std::exception &e) {
        QMessageBox::critical(this, "错误", "打开游戏王搜索窗口时发生意外错误！");
    }
}

void StartMenu::openPTCGSearch() {
    try {
        auto *ptcgSearch = new PTCGSearch(this);
        ptcgSearch->setAttribute(Qt::WA_DeleteOnClose);
        ptcgSearch->show();
    } catch (const std::exception &e) {
        QMessageBox::critical(this, "错误", "打开宝可梦搜索窗口时发生意外错误！");
    }
}

void StartMenu::openGenshinSearch() {
    try {
        auto *genshinSearch = new GenshinSearch(this);
        genshinSearch->setAttribute(Qt::WA_DeleteOnClose);
        genshinSearch->show();
    } catch (const std::exception &e) {
        QMessageBox::critical(this, "错误", "打开原神搜索窗口时发生意外错误！");
    }
}
