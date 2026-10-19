#include <QApplication>
#include "StartMenu.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv); // 创建应用程序对象

    StartMenu startMenu; // 创建开始菜单
    startMenu.show(); // 显示开始菜单

    return app.exec(); // 进入应用程序事件循环
}
