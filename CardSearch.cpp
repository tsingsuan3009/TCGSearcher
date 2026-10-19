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
#include <QDebug>
#include <QResizeEvent>

CardSearch::CardSearch(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Card Search"); // 设置窗口标题
    setStyleSheet(R"(
        QMainWindow { background-color: #0b1120; }
        QWidget { color: #e2e8f0; font-family: "Microsoft YaHei UI", "Segoe UI"; font-size: 10pt; }
        QLineEdit {
            background-color: #111c30; border: 1px solid #334155; border-radius: 8px;
            padding: 10px 12px; selection-background-color: #2563eb;
        }
        QLineEdit:focus { border: 1px solid #60a5fa; }
        QPushButton {
            color: #f8fafc; background-color: #2563eb; border: none; border-radius: 8px;
            padding: 10px 18px; font-weight: 600;
        }
        QPushButton:hover { background-color: #3b82f6; }
        QPushButton:disabled { color: #94a3b8; background-color: #334155; }
        QLabel { color: #cbd5e1; }
        QTableWidget {
            color: #f8fafc; gridline-color: rgba(148, 163, 184, 70);
            border: 1px solid #334155; border-radius: 8px;
            selection-background-color: rgba(37, 99, 235, 190);
        }
        QTableWidget::item { padding: 7px; background-color: rgba(11, 17, 32, 205); }
        QTableWidget::item:alternate { background-color: rgba(20, 30, 48, 205); }
        QHeaderView::section {
            color: #e2e8f0; background-color: #1e293b; border: none;
            border-bottom: 1px solid #475569; padding: 9px; font-weight: 600;
        }
        QScrollBar:vertical { background: #111827; width: 12px; margin: 2px; }
        QScrollBar::handle:vertical { background: #475569; min-height: 24px; border-radius: 5px; }
        QScrollBar::handle:vertical:hover { background: #64748b; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        QScrollBar:horizontal { background: #111827; height: 12px; margin: 2px; }
        QScrollBar::handle:horizontal { background: #475569; min-width: 24px; border-radius: 5px; }
        QScrollBar::handle:horizontal:hover { background: #64748b; }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }
    )");

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
    resultTable->setAlternatingRowColors(true);
    resultTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    layout->addWidget(resultTable); // 将表格添加到布局中

    connect(searchButton, &QPushButton::clicked, this, &CardSearch::performSearch);

    networkManager = new QNetworkAccessManager(this);
    setCentralWidget(centralWidget);
}

void CardSearch::requestJson(const QUrl &url,
                             const QByteArray &postData,
                             const QString &operation,
                             const std::function<void(const QByteArray &)> &onSuccess) {
    if (!url.isValid() || url.scheme() != QStringLiteral("https")) {
        showDataError("卡片数据地址无效，必须使用 HTTPS。");
        return;
    }

    setDataStatus(operation, false);

    QNetworkRequest request(url);
    request.setTransferTimeout(60000);
    QNetworkReply *reply = nullptr;
    if (postData.isEmpty()) {
        reply = networkManager->get(request);
    } else {
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=utf-8");
        reply = networkManager->post(request, postData);
    }

    connect(reply, &QNetworkReply::finished, this, [this, reply, url, operation, onSuccess]() {
        const QNetworkReply::NetworkError networkError = reply->error();
        const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QString errorMessage = reply->errorString();
        const QByteArray response = reply->readAll();
        reply->deleteLater();

        if (networkError != QNetworkReply::NoError || httpStatus < 200 || httpStatus >= 300) {
            const QString reason = networkError != QNetworkReply::NoError
                                       ? errorMessage
                                       : QString("HTTP %1").arg(httpStatus);
            showDataError(QString("%1失败（%2）：%3")
                              .arg(operation, url.toString(), reason));
            return;
        }

        onSuccess(response);
    });
}

void CardSearch::setDataStatus(const QString &message, bool searchEnabled) {
    dataStatusLabel->setText(message);
    dataStatusLabel->setVisible(!message.isEmpty());
    searchBox->setEnabled(searchEnabled);
    searchButton->setEnabled(searchEnabled);
}

void CardSearch::showDataError(const QString &message) {
    setDataStatus("联网获取数据失败，请检查网络后重试。", true);
    logError(message);
}

void CardSearch::setTableBackground(const QString &resourcePath) {
    QPixmap background(resourcePath);
    if (background.isNull()) {
        qWarning() << "无法加载卡片列表背景图片:" << resourcePath;
        return;
    }

    tableBackground = background;
    updateTableBackground();
}

void CardSearch::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    updateTableBackground();
}

void CardSearch::updateTableBackground() {
    if (tableBackground.isNull() || !resultTable) {
        return;
    }

    const QSize viewportSize = resultTable->viewport()->size();
    if (viewportSize.isEmpty()) {
        return;
    }

    const QPixmap scaled = tableBackground.scaled(
        viewportSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    const QRect crop((scaled.width() - viewportSize.width()) / 2,
                     (scaled.height() - viewportSize.height()) / 2,
                     viewportSize.width(),
                     viewportSize.height());
    QPalette palette = resultTable->palette();
    palette.setBrush(QPalette::Base, QBrush(scaled.copy(crop)));
    resultTable->setPalette(palette);
    resultTable->setAutoFillBackground(true);
    resultTable->viewport()->update();
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
