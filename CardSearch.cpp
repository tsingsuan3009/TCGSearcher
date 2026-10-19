#include "CardSearch.h"
#include <QHeaderView>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QTextStream>
#include <QJsonDocument>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QInputDialog>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonParseError>

CardSearch::CardSearch(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Card Search"); // 设置窗口标题

    QWidget *centralWidget = new QWidget(this); // 创建中心窗口
    QVBoxLayout *layout = new QVBoxLayout(centralWidget); // 创建布局

    searchBox = new QLineEdit(this); // 创建搜索框
    searchBox->setPlaceholderText("请输入关键字"); // 设置占位文本
    layout->addWidget(searchBox); // 将搜索框添加到布局中

    searchButton = new QPushButton("搜索", this); // 创建搜索按钮
    layout->addWidget(searchButton); // 将搜索按钮添加到布局中

    dataStatusLabel = new QLabel(this);
    dataStatusLabel->hide();
    layout->addWidget(dataStatusLabel);

    resultTable = new QTableWidget(this); // 创建表格
    layout->addWidget(resultTable); // 将表格添加到布局中

    connect(searchButton, &QPushButton::clicked, this, &CardSearch::performSearch);

    networkManager = new QNetworkAccessManager(this);
    setCentralWidget(centralWidget);
}

void CardSearch::loadJsonFromUrl(const QUrl &url) {
    if (!url.isValid() || url.scheme() != QStringLiteral("https")) {
        logError("卡片数据地址无效，必须使用 HTTPS。");
        return;
    }

    dataStatusLabel->setText("正在联网加载卡片数据...");
    dataStatusLabel->show();
    searchBox->setEnabled(false);
    searchButton->setEnabled(false);

    QNetworkRequest request(url);
    request.setTransferTimeout(60000);
    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, url]() {
        const QNetworkReply::NetworkError networkError = reply->error();
        const QString errorMessage = reply->errorString();
        const QByteArray response = reply->readAll();
        reply->deleteLater();

        if (networkError != QNetworkReply::NoError) {
            dataStatusLabel->setText("数据加载失败。请检查网络后重新打开此窗口重试。");
            logError(QString("无法从 %1 加载卡片数据：%2").arg(url.toString(), errorMessage));
            return;
        }

        cards.clear();
        if (!loadJsonData(response) || cards.isEmpty()) {
            dataStatusLabel->setText("卡片数据格式无效，请稍后重试。");
            logError(QString("无法解析从 %1 获取的卡片数据。").arg(url.toString()));
            return;
        }

        dataStatusLabel->hide();
        searchBox->setEnabled(true);
        searchButton->setEnabled(true);
    });
}

void CardSearch::performSearch() {
    // 留空：子类实现具体逻辑
}

void CardSearch::showCardDetails(QTableWidgetItem *item) {
    // 留空：子类实现具体逻辑
}

void CardSearch::logError(const QString &message) {
    // 将错误日志写入文件
    QFile logFile("error_log.txt");
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream out(&logFile);
        out << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")
            << " - 错误: " << message << "\n";
        logFile.close();
    }

    // 在控制台输出错误
    qDebug() << "错误:" << message;

    // 显示错误消息框
    QMessageBox::critical(this, "错误", message);
}
