#include "PTCGSearch.h"
#include <QDir>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QDebug>
#include <QFontMetrics>
#include <QJsonDocument>
#include <QLabel>
#include <QWidget>
#include <QTextEdit>
#include <QTimer>

PTCGSearch::PTCGSearch(QWidget *parent) : CardSearch(parent) {
    originalTitle = "PTCG卡片搜索"; // 初始化完整标题
    setWindowTitle(originalTitle);
    loadCards();

    // 设置表头
    resultTable->setColumnCount(3);
    resultTable->setHorizontalHeaderLabels({"卡片ID", "名称", "Yoren Code"});

    // 禁止用户修改表格内容
    resultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 动态调整窗口大小
    adjustWindowSize();

    // 设置表格列宽根据内容调整
    resultTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // 调整窗口大小以适应表格内容
    adjustWindowSize();

    // 搜索框回车触发搜索
    connect(searchBox, &QLineEdit::returnPressed, this, &PTCGSearch::performSearch);

    // 双击显示详细信息
    connect(resultTable, &QTableWidget::itemDoubleClicked, this, &PTCGSearch::showCardDetails);
}

void PTCGSearch::adjustWindowSize() {
    if (!resultTable) return; // 防止空指针访问

    // 获取表格的总宽度
    int tableWidth = resultTable->verticalHeader()->width(); // 包含行头的宽度
    for (int i = 0; i < resultTable->columnCount(); ++i) {
        tableWidth += resultTable->columnWidth(i);
    }
    tableWidth += resultTable->frameWidth() * 2; // 加上表格的边框宽度
    tableWidth += 100;

    // 计算标题宽度
    QFontMetrics metrics(font());
    int titleWidth = metrics.horizontalAdvance(originalTitle) + 50; // 为标题预留边距

    // 窗口宽度取标题宽度和表格宽度的最大值
    int windowWidth = qMax(tableWidth, titleWidth);

    // 获取表格的总高度
    int tableHeight = resultTable->horizontalHeader()->height(); // 表头高度
    int visibleRows = qMin(resultTable->rowCount(), 10); // 显示最多10行
    for (int i = 0; i < visibleRows; ++i) {
        tableHeight += resultTable->rowHeight(i);
    }
    tableHeight += resultTable->frameWidth() * 2; // 加上表格的边框宽度

    // 根据宽高比调整窗口高度
    int windowHeight = tableHeight + layout()->contentsMargins().top() + layout()->contentsMargins().bottom();

    // 调整标题显示
    adjustTitle(windowWidth);

    // 设置窗口大小
    this->resize(windowWidth, windowHeight);
}

void PTCGSearch::adjustTitle(int windowWidth) {
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

void PTCGSearch::loadCards() {
    loadJsonFromUrl(QUrl("https://raw.githubusercontent.com/tsingsuan3009/TCGSearcher/main/data/cards_ptcg.json"));
}

bool PTCGSearch::loadJsonData(const QByteArray &data) {
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        return false;
    }

    const QJsonArray collectionsArray = doc.object()["collections"].toArray();
    for (const QJsonValue &collectionValue : collectionsArray) {
        const QJsonArray cardsArray = collectionValue.toObject()["cards"].toArray();
        for (const QJsonValue &cardValue : cardsArray) {
            cards.append(cardValue.toObject());
        }
    }
    if (cards.isEmpty()) {
        return false;
    }

    adjustWindowSize();
    qDebug() << "加载的卡片数:" << cards.size();
    return true;
}

void PTCGSearch::performSearch() {
    QString keyword = searchBox->text();
    if (keyword.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入关键词进行搜索！");
        return;
    }

    resultTable->setRowCount(0);
    bool found = false;

    for (const QJsonValue &value : cards) {
        QJsonObject card = value.toObject(); // 使用 toObject() 将 QJsonValue 转换为 QJsonObject
        QString id = QString::number(card["id"].toInt());
        QString name = card["name"].toString();
        QString yorenCode = card["yorenCode"].toString();

        if (id.contains(keyword, Qt::CaseInsensitive) ||
            name.contains(keyword, Qt::CaseInsensitive) ||
            yorenCode.contains(keyword, Qt::CaseInsensitive)) {

            found = true;
            int row = resultTable->rowCount();
            resultTable->insertRow(row);

            resultTable->setItem(row, 0, new QTableWidgetItem(id));
            resultTable->setItem(row, 1, new QTableWidgetItem(name));
            resultTable->setItem(row, 2, new QTableWidgetItem(yorenCode));
        }
    }

    if (!found) {
        QMessageBox::information(this, "提示", "没有找到相关卡片！");
    }

    // 重新调整窗口大小
    adjustWindowSize();
}

void PTCGSearch::showCardDetails(QTableWidgetItem *item) {
    int row = item->row(); // 获取被双击的行号

    // 从表格中获取基本信息，若单元格为空则显示 "-"
    QString id = resultTable->item(row, 0) ? resultTable->item(row, 0)->text() : "-";
    QString name = resultTable->item(row, 1) ? resultTable->item(row, 1)->text() : "-";
    QString yorenCode = resultTable->item(row, 2) ? resultTable->item(row, 2)->text() : "-";

    // 查找对应卡片的详细信息
    QJsonObject cardDetails;
    for (const QJsonValue &value : cards) {
        QJsonObject card = value.toObject();
        if (QString::number(card["id"].toInt()) == id) {
            cardDetails = card["details"].toObject();
            break;
        }
    }

    // 创建一个对话框用于显示卡片详细信息
    QDialog *detailDialog = new QDialog(this);
    detailDialog->setWindowTitle("卡片详情"); // 设置对话框标题

    QVBoxLayout *layout = new QVBoxLayout(detailDialog);

    // 工具函数：创建可复制的标签
    auto createLabelWithCopy = [layout](const QString &fieldName, const QString &value) {
        QLabel *label = new QLabel(fieldName + ": " + value); // 显示字段名称和值
        label->setTextInteractionFlags(Qt::TextSelectableByMouse); // 设置为可复制
        label->setWordWrap(true); // 自动换行
        layout->addWidget(label); // 添加到布局
    };

    // 添加基本信息
    createLabelWithCopy("卡片ID", id);
    createLabelWithCopy("名称", name);
    createLabelWithCopy("Yoren Code", yorenCode);

    // 如果卡片详情不为空，则添加更多字段信息
    if (!cardDetails.isEmpty()) {
        createLabelWithCopy("Ability List", cardDetails["abilityItemList"].toString("-"));
        createLabelWithCopy("Attribute", cardDetails["attribute"].toString("-"));
        createLabelWithCopy("Card Type", cardDetails["cardType"].toString("-"));
        createLabelWithCopy("Card Type Text", cardDetails["cardTypeText"].toString("-"));
        createLabelWithCopy("Collection Flag", cardDetails["collectionFlag"].toString("-"));
        createLabelWithCopy("Collection Number", cardDetails["collectionNumber"].toString("-"));
        createLabelWithCopy("Commodity List", cardDetails["commodityList"].toString("-"));
        createLabelWithCopy("Evolve", cardDetails["evolve"].toString("-"));
        createLabelWithCopy("Evolve Text", cardDetails["evolveText"].toString("-"));
        createLabelWithCopy("Feature Flag", cardDetails["featureFlag"].toString("-"));
        createLabelWithCopy("Feature Name", cardDetails["featureName"].toString("-"));
        createLabelWithCopy("Feature Text", cardDetails["featureText"].toString("-"));
        createLabelWithCopy("Height", cardDetails["height"].toString("-"));
        createLabelWithCopy("HP", cardDetails["hp"].toString("-"));
        createLabelWithCopy("Illustrator Name", cardDetails["illustratorName"].toString("-"));
        createLabelWithCopy("Is Pokeball Shiny", cardDetails["is_pokeball_shiny"].toString("-"));
        createLabelWithCopy("Pokedex Code", cardDetails["pokedexCode"].toString("-"));
        createLabelWithCopy("Pokedex Text", cardDetails["pokedexText"].toString("-"));
        createLabelWithCopy("Pokemon Category", cardDetails["pokemonCategory"].toString("-"));
        createLabelWithCopy("Rarity", cardDetails["rarity"].toString("-"));
        createLabelWithCopy("Rarity Text", cardDetails["rarityText"].toString("-"));
        createLabelWithCopy("Regulation Mark", cardDetails["regulationMark"].toString("-"));
        createLabelWithCopy("Regulation Mark Text", cardDetails["regulationMarkText"].toString("-"));
        createLabelWithCopy("Retreat Cost", cardDetails["retreatCost"].toString("-"));
        createLabelWithCopy("Weakness Formula", cardDetails["weaknessFormula"].toString("-"));
        createLabelWithCopy("Weakness Type", cardDetails["weaknessType"].toString("-"));
        createLabelWithCopy("Weight", cardDetails["weight"].toString("-"));
    }

    // 设置对话框布局并显示
    detailDialog->setLayout(layout);
    detailDialog->exec();
}
