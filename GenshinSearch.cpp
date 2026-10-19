#include "GenshinSearch.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QFontMetrics>
#include <QTextEdit>
#include <QDialog>
#include <QJsonParseError>
#include <algorithm>

GenshinSearch::GenshinSearch(QWidget *parent) : CardSearch(parent) {
    originalTitle = "原神卡片搜索"; // 保存完整的标题
    setWindowTitle(originalTitle);
    loadCards(); // 加载卡片数据

    // 设置表头
    resultTable->setColumnCount(3); // 确保表格有3列
    resultTable->setHorizontalHeaderLabels({"ID", "名称", "类型"});
    setTableBackground(":/genshin_background.jpeg");

    // 禁止用户修改表格内容
    resultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 动态调整窗口大小
    adjustWindowSize();

    // 将搜索框的回车键与搜索功能连接
    connect(searchBox, &QLineEdit::returnPressed, this, &GenshinSearch::performSearch);

    // 设置双击表格后显示详细信息
    connect(resultTable, &QTableWidget::itemDoubleClicked, this, &GenshinSearch::showCardDetails);
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
    const QUrl rolesUrl("https://api-takumi.mihoyo.com/event/cardsquare/roles");
    const QUrl actionsUrl("https://api-takumi.mihoyo.com/event/cardsquare/actions");
    setDataStatus("正在从米游社获取最新原神七圣召唤卡牌...", false);
    requestJson(rolesUrl, {}, "获取原神角色卡", [this, actionsUrl](const QByteArray &rolesData) {
        QJsonParseError parseError;
        const QJsonDocument rolesDocument = QJsonDocument::fromJson(rolesData, &parseError);
        if (parseError.error != QJsonParseError::NoError || !rolesDocument.isObject()) {
            showDataError("米游社角色卡数据格式无效。");
            return;
        }

        const QJsonObject rolesRoot = rolesDocument.object();
        const QJsonArray roleCards = rolesRoot["data"].toObject()["roles"].toArray();
        if (rolesRoot["retcode"].toInt(-1) != 0 || roleCards.isEmpty()) {
            showDataError("米游社未返回有效的原神角色卡数据。");
            return;
        }

        requestJson(actionsUrl, QByteArray("{}"), "获取原神行动卡", [this, roleCards](const QByteArray &actionsData) {
            QJsonParseError actionsParseError;
            const QJsonDocument actionsDocument = QJsonDocument::fromJson(actionsData, &actionsParseError);
            if (actionsParseError.error != QJsonParseError::NoError || !actionsDocument.isObject()) {
                showDataError("米游社行动卡数据格式无效。");
                return;
            }

            const QJsonObject actionsRoot = actionsDocument.object();
            const QJsonArray actionCards = actionsRoot["data"].toObject()["actions"].toArray();
            if (actionsRoot["retcode"].toInt(-1) != 0 || actionCards.isEmpty()) {
                showDataError("米游社未返回有效的原神行动卡数据。");
                return;
            }

            cards.clear();
            for (const QJsonValue &value : roleCards) {
                const QJsonObject role = value.toObject();
                const QJsonObject basic = role["basic"].toObject();
                QJsonObject card;
                card.insert("id", basic["item_id"]);
                card.insert("name", basic["name"]);
                card.insert("card_type", "角色");
                card.insert("hp", role["hp"].toVariant().toString());
                card.insert("element", role["element"]);
                card.insert("skill_cost", role["skill_cost"]);
                card.insert("max_cnt", basic["max_cnt"]);
                card.insert("icon", basic["icon"]);
                cards.append(card);
            }

            for (const QJsonValue &value : actionCards) {
                const QJsonObject action = value.toObject();
                const QJsonObject basic = action["basic"].toObject();
                QJsonObject card;
                card.insert("id", basic["item_id"]);
                card.insert("name", basic["name"]);
                card.insert("card_type", "行动");
                card.insert("action_type", action["card_type"]);
                card.insert("cost_num", action["skill_value"]);
                card.insert("energy_cost", action["energy_cost"]);
                card.insert("card_tag", action["card_tag"]);
                card.insert("max_cnt", basic["max_cnt"]);
                card.insert("icon", basic["icon"]);
                card.insert("wiki_url", basic["wiki_url"]);
                cards.append(card);
            }

            adjustWindowSize();
            setDataStatus(QString("已从米游社更新 %1 张卡牌。").arg(cards.size()), true);
            qDebug() << "加载的卡片数:" << cards.size();
        });
    });
}

static QString elementName(const QJsonValue &value) {
    switch (value.toInt()) {
    case 301: return "冰";
    case 302: return "水";
    case 303: return "火";
    case 304: return "雷";
    case 305: return "风";
    case 306: return "岩";
    case 307: return "草";
    default: return value.toVariant().toString();
    }
}

void GenshinSearch::performSearch() {
    const QString keyword = searchBox->text().trimmed();
    if (keyword.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入关键词进行搜索！");
        return;
    }

    resultTable->setRowCount(0);
    bool found = false;
    for (const QJsonObject &card : cards) {
        const QString id = card["id"].toVariant().toString();
        const QString name = card["name"].toString();
        const QString cardType = card["card_type"].toString();
        const QString element = elementName(card["element"]);
        const QString actionType = card["action_type"].toVariant().toString();
        const QString cost = card["cost_num"].toVariant().toString();
        const QString skillCost = card["skill_cost"].toVariant().toString();

        if (!id.contains(keyword, Qt::CaseInsensitive) &&
            !name.contains(keyword, Qt::CaseInsensitive) &&
            !cardType.contains(keyword, Qt::CaseInsensitive) &&
            !element.contains(keyword, Qt::CaseInsensitive) &&
            !actionType.contains(keyword, Qt::CaseInsensitive) &&
            !cost.contains(keyword, Qt::CaseInsensitive) &&
            !skillCost.contains(keyword, Qt::CaseInsensitive)) {
            continue;
        }

        found = true;
        const int row = resultTable->rowCount();
        resultTable->insertRow(row);
        resultTable->setItem(row, 0, new QTableWidgetItem(id));
        resultTable->setItem(row, 1, new QTableWidgetItem(name));
        resultTable->setItem(row, 2, new QTableWidgetItem(cardType));
    }

    if (!found) {
        QMessageBox::information(this, "提示", "没有找到相关卡片！");
    }
}

void GenshinSearch::showCardDetails(QTableWidgetItem *item) {
    if (!item || !resultTable->item(item->row(), 0)) {
        return;
    }

    const QString id = resultTable->item(item->row(), 0)->text();
    const auto cardIt = std::find_if(cards.cbegin(), cards.cend(), [&id](const QJsonObject &card) {
        return card["id"].toVariant().toString() == id;
    });
    if (cardIt == cards.cend()) {
        QMessageBox::warning(this, "警告", "未找到卡片详细信息！");
        return;
    }

    const QJsonObject card = *cardIt;
    QDialog detailDialog(this);
    detailDialog.setWindowTitle("卡片详情");
    detailDialog.resize(600, 400);

    auto *detailText = new QTextEdit(&detailDialog);
    detailText->setReadOnly(true);
    QString details = "卡片ID: " + id + "\n"
        + "名称: " + card["name"].toString() + "\n"
        + "类型: " + card["card_type"].toString() + "\n";

    if (card["card_type"].toString() == "角色") {
        details += "元素: " + elementName(card["element"]) + "\n"
            + "生命值: " + card["hp"].toVariant().toString() + "\n"
            + "技能费用: " + card["skill_cost"].toVariant().toString() + "\n";
    } else {
        details += "行动卡类型代码: " + card["action_type"].toVariant().toString() + "\n"
            + "元素骰费用: " + card["cost_num"].toVariant().toString() + "\n"
            + "能量费用: " + card["energy_cost"].toVariant().toString() + "\n"
            + "卡牌标签代码: "
            + QString::fromUtf8(QJsonDocument(card["card_tag"].toArray()).toJson(QJsonDocument::Compact))
            + "\n";
    }

    detailText->setText(details);
    auto *layout = new QVBoxLayout(&detailDialog);
    layout->addWidget(detailText);
    detailDialog.exec();
}
