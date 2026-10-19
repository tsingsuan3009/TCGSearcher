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

CardSearch::CardSearch(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Card Search"); // 设置窗口标题

    QWidget *centralWidget = new QWidget(this); // 创建中心窗口
    QVBoxLayout *layout = new QVBoxLayout(centralWidget); // 创建布局

    searchBox = new QLineEdit(this); // 创建搜索框
    searchBox->setPlaceholderText("请输入关键字"); // 设置占位文本
    layout->addWidget(searchBox); // 将搜索框添加到布局中

    searchButton = new QPushButton("搜索", this); // 创建搜索按钮
    layout->addWidget(searchButton); // 将搜索按钮添加到布局中

    resultTable = new QTableWidget(this); // 创建表格
    layout->addWidget(resultTable); // 将表格添加到布局中

    connect(searchButton, &QPushButton::clicked, this, &CardSearch::performSearch);

    setCentralWidget(centralWidget);
}

void CardSearch::loadJsonFile(const QString &filePath) {
    try {
        QFile file(filePath);
        if (!file.exists()) throw std::runtime_error("JSON 文件不存在！");
        if (!file.open(QIODevice::ReadOnly)) throw std::runtime_error("无法打开 JSON 文件！");

        QByteArray fileData = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(fileData);
        if (!doc.isObject()) throw std::runtime_error("JSON 格式错误！");

        QJsonObject root = doc.object();
        QJsonArray cardsArray = root["cards"].toArray();
        for (const QJsonValue &cardValue : cardsArray) {
            cards.append(cardValue.toObject());
        }
    } catch (const std::exception &e) {
        logError(e.what());
        QMessageBox::critical(this, "错误", e.what());
    }
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
