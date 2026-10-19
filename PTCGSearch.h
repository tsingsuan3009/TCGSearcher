#ifndef PTCGSEARCH_H
#define PTCGSEARCH_H

#include "CardSearch.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTableWidgetItem>

class PTCGSearch : public CardSearch {
    Q_OBJECT

public:
    explicit PTCGSearch(QWidget *parent = nullptr);

protected:
    void loadCards() override; // 加载卡片数据
    void performSearch(); // 执行搜索
    void showCardDetails(QTableWidgetItem *item) override; // 显示详细信息
    void addToFavorites(); // 添加到收藏夹
    void sortResults();

private:
    QString originalTitle; // 保存完整窗口标题
    void adjustWindowSize(); // 动态调整窗口大小，支持序号列和额外空间
    void adjustTitle(int windowWidth); // 动态调整标题显示

    bool loadJsonData(const QByteArray &data) override;
};

#endif // PTCGSEARCH_H
