#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QMessageBox>
#include <QDebug>
#include <QMetaType>
#include "ui/MainWindow.h"
#include "database/DatabaseManager.h"
#include "models/Product.h"
#include "models/Customer.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 注册自定义类型用于信号和槽
    qRegisterMetaType<Product*>("Product*");
    qRegisterMetaType<Customer*>("Customer*");
    
    // 设置应用程序信息
    app.setApplicationName("Smart POS System");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Smart Retail Solutions");
    
    // 设置现代化样式
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // 设置深色主题
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    app.setPalette(darkPalette);
    
    // 初始化数据库
    DatabaseManager& dbManager = DatabaseManager::getInstance();
    QString dbPath = QDir::currentPath() + "/pos_database.db";
    
    if (!dbManager.openDatabase(dbPath)) {
        QMessageBox::critical(nullptr, "数据库错误", 
                            "无法连接到数据库。请检查数据库文件权限。");
        return -1;
    }
    
    qDebug() << "数据库初始化成功:" << dbPath;
    
    // 创建并显示主窗口
    MainWindow window;
    window.show();
    
    qDebug() << "智能POS系统启动成功";
    
    return app.exec();
}
