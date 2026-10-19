#include "PTCGSearch.h"
#include <QDir>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QDebug>
#include <QFontMetrics>
#include <QJsonDocument>
#include <QDialog>
#include <QLabel>
#include <QWidget>
#include <QTextEdit>
#include <QTimer>
#include <algorithm>

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
    setDataStatus("输入关键词后，将实时查询 tcg.mik.moe 的最新简体中文卡牌数据。", true);
}

void PTCGSearch::performSearch() {
    const QString keyword = searchBox->text().trimmed();
    if (keyword.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入关键词进行搜索！");
        return;
    }

    currentSearchKeyword = keyword;
    currentSearchPage = 1;
    totalSearchResults = 0;
    cards.clear();
    resultTable->setRowCount(0);
    requestSearchPage();
}

void PTCGSearch::requestSearchPage() {
    const QJsonObject parameters{
        {"searchText", currentSearchKeyword},
        {"exact", false},
        {"unique", false},
        {"page", currentSearchPage},
        {"pageSize", 100}
    };
    const QByteArray body = QJsonDocument(parameters).toJson(QJsonDocument::Compact);
    const QUrl url("https://tcg.mik.moe/api/v3/card/card-basic-search");
    requestJson(url, body,
                QString("正在从 tcg.mik.moe 获取第 %1 页搜索结果...").arg(currentSearchPage),
                [this](const QByteArray &response) {
        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(response, &parseError);
        if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
            showDataError("宝可梦卡牌网站返回了无效的 JSON 数据。");
            return;
        }

        const QJsonObject root = document.object();
        const QJsonObject data = root["data"].toObject();
        if (root["code"].toInt() != 200 || !data["list"].isArray()) {
            showDataError("宝可梦卡牌网站未能完成搜索。");
            return;
        }

        totalSearchResults = data["itemNum"].toInt(totalSearchResults);
        const int pageCount = data["pageNum"].toInt(1);
        const QJsonArray results = data["list"].toArray();
        for (const QJsonValue &value : results) {
            QJsonObject card = value.toObject();
            const QString setCode = card["setCode"].toString();
            const QString cardIndex = card["cardIndex"].toString();
            const QString id = setCode + "-" + cardIndex;
            const QString name = card["cardName"].toString();
            card.insert("id", id);
            card.insert("name", name);
            cards.append(card);

            const int row = resultTable->rowCount();
            resultTable->insertRow(row);
            resultTable->setItem(row, 0, new QTableWidgetItem(id));
            resultTable->setItem(row, 1, new QTableWidgetItem(name));
            resultTable->setItem(row, 2, new QTableWidgetItem(card["yorenCode"].toString()));
        }

        if (currentSearchPage < pageCount) {
            ++currentSearchPage;
            requestSearchPage();
            return;
        }

        QString status = QString("已从 tcg.mik.moe 获取 %1 条结果。")
                             .arg(totalSearchResults);
        setDataStatus(status, true);
        adjustWindowSize();
        if (cards.isEmpty()) {
            QMessageBox::information(this, "提示", "没有找到相关卡片！");
        }
    });
}

void PTCGSearch::showCardDetails(QTableWidgetItem *item) {
    if (!item || !resultTable->item(item->row(), 0)) {
        return;
    }

    const QString id = resultTable->item(item->row(), 0)->text();
    const auto cardIt = std::find_if(cards.cbegin(), cards.cend(), [&id](const QJsonObject &card) {
        return card["id"].toString() == id;
    });
    if (cardIt == cards.cend()) {
        QMessageBox::warning(this, "警告", "未找到卡片详细信息！");
        return;
    }

    const QJsonObject card = *cardIt;
    const QJsonObject parameters{
        {"setCode", card["setCode"]},
        {"cardIndex", card["cardIndex"]}
    };
    const QByteArray body = QJsonDocument(parameters).toJson(QJsonDocument::Compact);
    const QUrl url("https://tcg.mik.moe/api/v3/card/card-detail");
    requestJson(url, body, "正在从 tcg.mik.moe 获取卡片详情...", [this](const QByteArray &response) {
        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(response, &parseError);
        if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
            showDataError("宝可梦卡牌网站返回了无效的详情数据。");
            return;
        }

        const QJsonObject root = document.object();
        const QJsonObject details = root["data"].toObject();
        if (root["code"].toInt() != 200 || details.isEmpty()) {
            showDataError("宝可梦卡牌网站未能返回卡片详情。");
            return;
        }

        QString text = QStringList{
            "名称：" + details["name"].toString(),
            "卡牌类型：" + details["cardType"].toString(),
            "卡牌编号：" + details["setCode"].toString() + " " + details["cardIndex"].toString(),
            "Yoren Code：" + details["yorenCode"].toString(),
            "稀有度：" + details["rarity"].toString(),
            "规则标记：" + details["regulationMark"].toString(),
            "发行日期：" + details["releaseDate"].toString(),
            "画师：" + details["artist"].toString(),
            "",
            details["description"].toString()
        }.join("\n");

        const QJsonObject pokemon = details["pokemonAttr"].toObject();
        if (!pokemon.isEmpty()) {
            text += "\n\n宝可梦属性：\n"
                + QString::fromUtf8(QJsonDocument(pokemon).toJson(QJsonDocument::Indented));
        }

        QDialog detailDialog(this);
        detailDialog.setWindowTitle("卡片详情");
        detailDialog.resize(600, 500);
        auto *detailText = new QTextEdit(&detailDialog);
        detailText->setReadOnly(true);
        detailText->setText(text);
        auto *layout = new QVBoxLayout(&detailDialog);
        layout->addWidget(detailText);
        detailDialog.exec();
        setDataStatus("已从 tcg.mik.moe 获取卡片详情。", true);
    });
}
