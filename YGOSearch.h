#ifndef YGOSEARCH_H
#define YGOSEARCH_H

#include "CardSearch.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTableWidgetItem>
#include <QJsonObject>

// 定义YGOSearch类，用于处理游戏王卡片搜索
class YGOSearch : public CardSearch {
    Q_OBJECT

public:
    YGOSearch(QWidget *parent = nullptr); // 构造函数

protected:
    void loadCards() override; // 加载卡片数据
    void performSearch() override; // 执行搜索
    void showCardDetails(QTableWidgetItem *item) override; // 显示详细信息
    void resizeEvent(QResizeEvent *event);
    void setupTableBackground();

private:
    QString originalTitle; // 保存完整窗口标题
    void adjustWindowSize(); // 动态调整窗口大小
    void adjustTitle(int windowWidth); // 动态调整标题显示

};

#endif // YGOSEARCH_H
