#include "YGOSearch.h"
#include <QDir>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QDebug>
#include <QFontMetrics>
#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QTimer>

YGOSearch::YGOSearch(QWidget *parent) : CardSearch(parent) {
    originalTitle = "游戏王卡片搜索"; // 保存完整标题
    setWindowTitle(originalTitle);
    loadCards(); // 加载卡片数据

    // 设置表头
    resultTable->setColumnCount(8); // 确保表格有8列
    resultTable->setHorizontalHeaderLabels({
        "卡片ID", "名称", "日文名称", "英文名称", "类型", "攻击力", "防御力", "描述"
    });
    if (!resultTable) {
        resultTable = new QTableWidget(this);
        resultTable->setColumnCount(8);
        resultTable->setHorizontalHeaderLabels({
            "卡片ID", "名称", "日文名称", "英文名称", "类型", "攻击力", "防御力", "描述"
        });
        resultTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // 禁止用户编辑
    }

    // 禁止用户修改表格内容
    resultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 动态调整窗口大小
    adjustWindowSize();

    // 使用定时器延迟设置背景图片
    QTimer::singleShot(100, this, &YGOSearch::setupTableBackground);

    // 将搜索框的回车键与搜索功能连接
    connect(searchBox, &QLineEdit::returnPressed, this, &YGOSearch::performSearch);

    // 设置双击表格后显示详细信息
    connect(resultTable, &QTableWidget::itemDoubleClicked, this, &YGOSearch::showCardDetails);
}

void YGOSearch::setupTableBackground() {
    QString backgroundPath = ":/ygo_background.jpg"; // 修改为资源路径或实际路径

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

void YGOSearch::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event); // 调用父类的 resizeEvent
    resultTable->viewport()->update(); // 强制刷新表格
}

void YGOSearch::adjustWindowSize() {
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

    // 根据宽高比 (2560:1440) 计算窗口需要的宽度和高度
    int windowWidth = tableWidth + layout()->contentsMargins().left() + layout()->contentsMargins().right() + extraWidth;
    double aspectRatio = 1440.0 / 2560.0;
    int windowHeight = static_cast<int>(windowWidth * aspectRatio);

    // 调整标题显示
    adjustTitle(windowWidth);

    // 设置窗口大小
    this->resize(windowWidth, windowHeight);
}

void YGOSearch::adjustTitle(int windowWidth) {
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

void YGOSearch::loadCards() {
    QDir currentDir(QDir::currentPath());
    QString filePath = currentDir.filePath("cards_ygo.json"); // 确保路径正确

    if (!QFile::exists(filePath)) {
        QMessageBox::critical(this, "错误", "游戏王卡片数据文件不存在：" + filePath);
        return;
    }

    loadJsonFile(filePath);
}

void YGOSearch::performSearch() {
    QString keyword = searchBox->text(); // 获取搜索框内容
    if (keyword.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入关键词进行搜索！"); // 如果为空，显示警告
        return;
    }

    resultTable->setRowCount(0); // 清空表格
    bool found = false; // 标记是否找到匹配的卡片

    for (const QJsonObject &card : cards) {
        QString id = QString::number(card["id"].toInt());
        QString cn_name = card["cn_name"].toString();
        QString jp_name = card["jp_name"].toString();
        QString en_name = card["en_name"].toString();
        QString types = card["text"].toObject()["types"].toString();
        QString atk = QString::number(card["data"].toObject()["atk"].toInt());
        QString def = QString::number(card["data"].toObject()["def"].toInt());
        QString desc = card["text"].toObject()["desc"].toString();

        if (cn_name.contains(keyword, Qt::CaseInsensitive) ||
            jp_name.contains(keyword, Qt::CaseInsensitive) ||
            en_name.contains(keyword, Qt::CaseInsensitive) ||
            desc.contains(keyword, Qt::CaseInsensitive)) {

            found = true;
            int row = resultTable->rowCount(); // 获取当前行数
            resultTable->insertRow(row); // 插入新行

            // 设置表格每列的内容
            resultTable->setItem(row, 0, new QTableWidgetItem(id));
            resultTable->setItem(row, 1, new QTableWidgetItem(cn_name));
            resultTable->setItem(row, 2, new QTableWidgetItem(jp_name));
            resultTable->setItem(row, 3, new QTableWidgetItem(en_name));
            resultTable->setItem(row, 4, new QTableWidgetItem(types));
            resultTable->setItem(row, 5, new QTableWidgetItem(atk));
            resultTable->setItem(row, 6, new QTableWidgetItem(def));
            resultTable->setItem(row, 7, new QTableWidgetItem(desc));
        }
    }

    if (!found) {
        QMessageBox::information(this, "提示", "没有找到相关卡片！"); // 如果未找到，显示提示信息
    }
}

void YGOSearch::showCardDetails(QTableWidgetItem *item) {
    int row = item->row(); // 获取被双击的行号

    auto getTextOrDefault = [&](int column) {
        QTableWidgetItem *cell = resultTable->item(row, column);
        return cell ? cell->text() : QString("无"); // 如果单元格为空，则返回默认值 "无"
    };

    QString id = getTextOrDefault(0);
    QString cn_name = getTextOrDefault(1);
    QString jp_name = getTextOrDefault(2);
    QString en_name = getTextOrDefault(3);
    QString types = getTextOrDefault(4);
    QString atk = getTextOrDefault(5);
    QString def = getTextOrDefault(6);
    QString desc = getTextOrDefault(7);

    QDialog *detailDialog = new QDialog(this);
    detailDialog->setWindowTitle("卡片详情"); // 设置对话框标题
    detailDialog->resize(600, 400);

    // 创建不可编辑但可复制的文本框
    QTextEdit *detailText = new QTextEdit(detailDialog);
    detailText->setReadOnly(true); // 设置为只读
    detailText->setText(
        "卡片ID: " + id + "\n" +
        "中文名称: " + cn_name + "\n" +
        "日文名称: " + jp_name + "\n" +
        "英文名称: " + en_name + "\n" +
        "类型: " + types + "\n" +
        "攻击力: " + atk + "\n" +
        "防御力: " + def + "\n" +
        "描述: " + desc
        );

    QVBoxLayout *layout = new QVBoxLayout(detailDialog);
    layout->addWidget(detailText);

    detailDialog->setLayout(layout);
    detailDialog->exec(); // 显示对话框
}

void YGOSearch::loadJsonFile(const QString &filePath) {
    QFile file(filePath); // 打开JSON文件
    if (!file.exists()) {
        QMessageBox::critical(this, "错误", "找不到JSON文件：" + filePath); // 如果文件不存在，显示错误
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "错误", "无法打开JSON文件：" + filePath); // 如果文件无法打开，显示错误
        return;
    }

    QByteArray fileData = file.read(1024 * 1024); // 分批读取 1MB 数据
    while (!file.atEnd()) {
        fileData.append(file.read(1024 * 1024));
    }
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(fileData); // 解析JSON数据
    if (doc.isObject()) {
        QJsonObject root = doc.object(); // 获取JSON根对象
        for (const QString &key : root.keys()) {
            QJsonObject card = root[key].toObject(); // 遍历每张卡片
            cards.append(card); // 添加到卡片列表
        }
        qDebug() << "加载的卡片数:" << cards.size();
    } else {
        QMessageBox::critical(this, "错误", "JSON格式不支持。"); // 如果JSON格式不支持，显示错误
    }
}
