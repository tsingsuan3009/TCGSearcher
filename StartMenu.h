#ifndef STARTMENU_H
#define STARTMENU_H

#include <QMainWindow>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QTreeWidget>

#include "YGOSearch.h"
#include "PTCGSearch.h"
#include "GenshinSearch.h"

// 定义 StartMenu 类，作为程序的开始菜单
class StartMenu : public QMainWindow {
    Q_OBJECT

public:
    explicit StartMenu(QWidget *parent = nullptr); // 构造函数

private slots:
    // 打开具体搜索界面的槽函数
    void openYGOSearch();
    void openPTCGSearch();
    void openGenshinSearch();

    // 显示帮助对话框的槽函数
    void showYGOHelp();
    void showPTCGHelp();
    void showGenshinHelp();

    // 显示“关于”对话框的槽函数
    void showAboutDialog();
    void showAuthorInfo();

    void addToolMenu();
    void clearSearchResults();
    void resetWindowLayout();

private:
    // 辅助函数：显示帮助对话框
    void showHelpDialog(const QString &title, const QMap<QString, QStringList> &sections);
    void showRulesFromTxt(const QString &title, const QString &filePath);
};

#endif // STARTMENU_H
