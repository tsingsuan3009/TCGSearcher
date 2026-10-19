#ifndef GENSHINSEARCH_H
#define GENSHINSEARCH_H

#include "CardSearch.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QWidget>

class GenshinSearch : public CardSearch {
    Q_OBJECT

public:
    explicit GenshinSearch(QWidget *parent = nullptr);

protected:
    void loadCards() override; // 加载卡片数据
    void performSearch() override; // 执行搜索
    void showCardDetails(QTableWidgetItem *item) override; // 显示详细信息
    void resizeEvent(QResizeEvent *event);
    void setupTableBackground();
private:
    QString originalTitle; // 保存完整窗口标题
    void adjustWindowSize(); // 动态调整窗口大小，支持序号列和额外空间
    void adjustTitle(int windowWidth); // 动态调整标题显示
};

#endif // GENSHINSEARCH_H
