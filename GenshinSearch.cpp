#include "GenshinSearch.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QDir>
#include <QHeaderView>
#include <QFontMetrics>
#include <QTextEdit>
#include <QTimer>

GenshinSearch::GenshinSearch(QWidget *parent) : CardSearch(parent) {
    originalTitle = "原神卡片搜索"; // 保存完整的标题
    setWindowTitle(originalTitle);
    loadCards(); // 加载卡片数据

    // 设置表头
    resultTable->setColumnCount(3); // 确保表格有3列
    resultTable->setHorizontalHeaderLabels({"ID", "名称", "类型"});

    // 禁止用户修改表格内容
    resultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 动态调整窗口大小
    adjustWindowSize();

    // 使用定时器延迟设置背景图片
    QTimer::singleShot(100, this, &GenshinSearch::setupTableBackground);

    // 将搜索框的回车键与搜索功能连接
    connect(searchBox, &QLineEdit::returnPressed, this, &GenshinSearch::performSearch);

    // 设置双击表格后显示详细信息
    connect(resultTable, &QTableWidget::itemDoubleClicked, this, &GenshinSearch::showCardDetails);
}

void GenshinSearch::setupTableBackground() {
    QString backgroundPath = ":/genshin_background.jpeg"; // 修改为资源路径或实际路径

    QPixmap backgroundPixmap(backgroundPath);
    if (backgroundPixmap.isNull()) {
        qDebug() << "Failed to load background image:" << backgroundPath;
        return;
    }

    // 设置背景图片并拉伸以适配表格大小
    QPalette palette;
    palette.setBrush(QPalette::Base, QBrush(backgroundPixmap.scaled(
                                         resultTable->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation))); // 拉伸背景图
    resultTable->setPalette(palette);
    resultTable->setAutoFillBackground(true); // 启用背景填充

    resultTable->viewport()->update(); // 强制刷新视图
}

void GenshinSearch::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event); // 调用父类的 resizeEvent
    resultTable->viewport()->update(); // 强制刷新表格
}

void GenshinSearch::adjustWindowSize() {
    if (!resultTable) return; // 防止空指针访问

    // 获取表格的总宽度
    int tableWidth = resultTable->verticalHeader()->width(); // 包含行头的宽度
    for (int i = 0; i < resultTable->columnCount(); ++i) {
        tableWidth += resultTable->columnWidth(i);
    }
    tableWidth += resultTable->frameWidth() * 2; // 加上表格的边框宽度

    // 额外增加的宽度余量，用于避免表格过于拥挤
    int extraWidth = 50;

    // 获取表格的总高度
    int tableHeight = resultTable->horizontalHeader()->height(); // 表头高度
    int visibleRows = qMin(resultTable->rowCount(), 10); // 显示最多10行
    for (int i = 0; i < visibleRows; ++i) {
        tableHeight += resultTable->rowHeight(i);
    }
    tableHeight += resultTable->frameWidth() * 2; // 加上表格的边框宽度

    // 根据宽高比计算窗口需要的宽度和高度
    int windowWidth = tableWidth + layout()->contentsMargins().left() + layout()->contentsMargins().right() + extraWidth;
    double aspectRatio = 1;
    int windowHeight = static_cast<int>(windowWidth * aspectRatio);

    // 调整标题显示
    adjustTitle(windowWidth);

    // 设置窗口大小
    this->resize(windowWidth, windowHeight);
}

void GenshinSearch::adjustTitle(int windowWidth) {
    // 使用 QFontMetrics 计算标题宽度
    QFontMetrics metrics(font());
    int titleWidth = metrics.horizontalAdvance(originalTitle);

    // 如果标题宽度超过窗口宽度，则截断标题并添加省略号
    if (titleWidth > windowWidth - 50) { // 保留一些边距
        QString shortenedTitle = metrics.elidedText(originalTitle, Qt::ElideRight, windowWidth - 50);
        setWindowTitle(shortenedTitle);
    } else {
        setWindowTitle(originalTitle); // 否则显示完整标题
    }
}

void GenshinSearch::loadCards() {
    QDir currentDir(QDir::currentPath());
    QString filePath = currentDir.filePath("cards_genshin.json");
    if (!QFile::exists(filePath)) {
        QMessageBox::critical(this, "错误", "找不到七圣召唤卡片数据文件：" + filePath);
        return;
    }

    loadJsonFile(filePath);
}

void GenshinSearch::loadJsonFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.exists()) {
        QMessageBox::critical(this, "错误", "找不到JSON文件：" + filePath);
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "错误", "无法打开JSON文件：" + filePath);
        return;
    }

    QByteArray fileData = file.read(1024 * 1024); // 分批读取 1MB 数据
    while (!file.atEnd()) {
        fileData.append(file.read(1024 * 1024));
    }
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(fileData);
    if (doc.isObject()) {
        QJsonObject root = doc.object();

        // 加载角色卡
        QJsonArray roleCards = root["data"].toObject()["role_card_infos"].toArray();
        for (const auto &item : roleCards) {
            QJsonObject card = item.toObject();
            card.insert("card_type", "角色"); // 标记卡牌类型
            cards.append(card);
        }

        // 加载行动卡
        QJsonArray actionCards = root["data"].toObject()["action_card_infos"].toArray();
        for (const auto &item : actionCards) {
            QJsonObject card = item.toObject();
            card.insert("card_type", "行动"); // 标记卡牌类型
            cards.append(card);
        }

        qDebug() << "加载的卡片数:" << cards.size();

    } else {
        QMessageBox::critical(this, "错误", "JSON格式不支持。");
    }
}

void GenshinSearch::performSearch() {
    QString keyword = searchBox->text(); // 获取搜索框内容
    if (keyword.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入关键词进行搜索！");
        return;
    }

    resultTable->setRowCount(0); // 清空表格
    bool found = false;

    for (const QJsonObject &card : cards) {
        QString id = QString::number(card["id"].toInt());
        QString name = card["name"].toString();
        QString cardType = card["card_type"].toString(); // 获取卡片类型

        QStringList attributes; // 收集可供搜索的字段
        attributes << id << name << cardType;

        // 对不同卡牌类型追加特定字段
        if (cardType == "角色") {
            attributes << card["weapon"].toString() << card["hp"].toString();
        } else if (cardType == "行动") {
            attributes << card["action_type"].toString();
        }

        // 判断是否匹配关键字
        bool matches = false;
        for (const QString &attr : attributes) {
            if (attr.contains(keyword, Qt::CaseInsensitive)) {
                matches = true;
                break;
            }
        }

        if (matches) {
            found = true;
            int row = resultTable->rowCount();
            resultTable->insertRow(row);

            resultTable->setItem(row, 0, new QTableWidgetItem(id));
            resultTable->setItem(row, 1, new QTableWidgetItem(name));
            resultTable->setItem(row, 2, new QTableWidgetItem(cardType));
        }
    }

    if (!found) {
        QMessageBox::information(this, "提示", "没有找到相关卡片！");
    }
}

void GenshinSearch::showCardDetails(QTableWidgetItem *item) {
    int row = item->row();
    QString id = resultTable->item(row, 0)->text();

    // 查找卡牌数据
    QJsonObject cardDetails;
    for (const QJsonObject &card : cards) {
        if (QString::number(card["id"].toInt()) == id) {
            cardDetails = card;
            break;
        }
    }

    if (cardDetails.isEmpty()) {
        QMessageBox::warning(this, "警告", "未找到卡片详细信息！");
        return;
    }

    QDialog *detailDialog = new QDialog(this);
    detailDialog->setWindowTitle("卡片详情");
    detailDialog->resize(600, 400);

    // 设置对话框背景为淡绿色
    detailDialog->setStyleSheet("background-color: #d4f7d4;"); // 设置淡绿色背景

    QVBoxLayout *layout = new QVBoxLayout(detailDialog);
    QLabel *idLabel = new QLabel("ID: " + id);
    idLabel->setWordWrap(true);
    layout->addWidget(idLabel);

    QLabel *nameLabel = new QLabel("名称: " + processColoredText(cardDetails["name"].toString()));
    nameLabel->setWordWrap(true);
    layout->addWidget(nameLabel);

    QLabel *typeLabel = new QLabel("类型: " + cardDetails["card_type"].toString());
    typeLabel->setWordWrap(true);
    layout->addWidget(typeLabel);

    QString cardType = cardDetails["card_type"].toString();
    if (cardType == "角色") {
        QLabel *weaponLabel = new QLabel("武器: " + processColoredText(cardDetails["weapon"].toString()));
        weaponLabel->setWordWrap(true);
        layout->addWidget(weaponLabel);

        QLabel *hpLabel = new QLabel("HP: " + cardDetails["hp"].toString());
        hpLabel->setWordWrap(true);
        layout->addWidget(hpLabel);

        // 添加角色技能信息
        QJsonArray roleSkills = cardDetails["role_skill_infos"].toArray();
        QLabel *skillsTitle = new QLabel("技能信息: ");
        skillsTitle->setWordWrap(true);
        layout->addWidget(skillsTitle);
        for (const auto &skill : roleSkills) {
            QJsonObject skillObj = skill.toObject();
            QLabel *skillLabel = new QLabel("- ID: " + skillObj["id"].toString() + ", 名称: " + processColoredText(skillObj["name"].toString()) + ", 描述: " + processNewlineText(processColoredText(skillObj["skill_text"].toString())));
            skillLabel->setWordWrap(true);
            layout->addWidget(skillLabel);
        }

        // 添加所属信息
        QJsonArray belongTo = cardDetails["belong_to"].toArray();
        QLabel *belongTitle = new QLabel("所属: ");
        belongTitle->setWordWrap(true);
        layout->addWidget(belongTitle);
        for (const auto &belong : belongTo) {
            QLabel *belongLabel = new QLabel("- " + processColoredText(belong.toString()));
            belongLabel->setWordWrap(true);
            layout->addWidget(belongLabel);
        }

    } else if (cardType == "行动") {
        // 添加行动牌的内容信息
        QLabel *contentLabel = new QLabel("内容: " + processNewlineText(processColoredText(cardDetails["content"].toString())));
        contentLabel->setWordWrap(true);
        layout->addWidget(contentLabel);
    }

    detailDialog->setLayout(layout);
    detailDialog->exec();
}

QString GenshinSearch::processColoredText(const QString &text) {
    QString processedText = text;

    // 处理 <color> 标签并应用颜色
    QRegularExpression colorRegex("<color=([^>]+)>(.*?)</color>");
    QRegularExpressionMatchIterator matchIterator = colorRegex.globalMatch(text);

    while (matchIterator.hasNext()) {
        QRegularExpressionMatch match = matchIterator.next();
        QString color = match.captured(1); // 获取颜色代码
        QString content = match.captured(2); // 获取内容

        // 替换为 HTML 样式文本
        QString coloredText = QString("<span style='color:%1;'>%2</span>").arg(color, content);
        processedText.replace(match.captured(0), coloredText);
    }

    return processedText;
}

QString GenshinSearch::processNewlineText(QString text) {
    return text.replace("\\n", "\n"); // 处理 \n 并替换为实际换行符
}
