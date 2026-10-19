#ifndef CARDSEARCH_H
#define CARDSEARCH_H

#include <QMainWindow>
#include <QLineEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QComboBox>
#include <QFileDialog>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>
#include <QVector>
#include <QMap>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QLabel>

// 定义 CardSearch 类，用于处理卡片搜索功能
class CardSearch : public QMainWindow {
    Q_OBJECT

public:
    explicit CardSearch(QWidget *parent = nullptr);
    virtual ~CardSearch() {}

protected:
    virtual void performSearch(); // 搜索功能
    virtual void loadCards() = 0; // 纯虚函数，用于加载卡片数据
    virtual bool loadJsonData(const QByteArray &data) = 0;
    virtual void showCardDetails(QTableWidgetItem *item);

    QLineEdit *searchBox; // 搜索框
    QTableWidget *resultTable; // 显示搜索结果的表格
    QPushButton *searchButton;
    QVector<QJsonObject> cards; // 存储卡片数据的向量

    void loadJsonFromUrl(const QUrl &url);
    void logError(const QString &message); // 错误日志记录

private:
    QLabel *dataStatusLabel;
    QNetworkAccessManager *networkManager;
};

#endif // CARDSEARCH_H
