#include <QApplication>
#include <QDebug>
#include <QTimer>
#include "src/ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "=== 推荐显示功能测试 ===";
    
    MainWindow* window = new MainWindow();
    window->show();
    
    // 等待窗口初始化完成
    QTimer::singleShot(2000, [window]() {
        qDebug() << "窗口已显示，开始测试推荐功能...";
        
        // 模拟添加商品到购物车
        qDebug() << "模拟添加商品到购物车...";
        // 这里可以添加具体的测试逻辑
    });
    
    return app.exec();
} 