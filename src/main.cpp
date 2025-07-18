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
    
    // 设置浅色主题
    QPalette lightPalette;
    lightPalette.setColor(QPalette::Window, QColor(245, 245, 245));
    lightPalette.setColor(QPalette::WindowText, QColor(51, 51, 51));
    lightPalette.setColor(QPalette::Base, QColor(255, 255, 255));
    lightPalette.setColor(QPalette::AlternateBase, QColor(249, 249, 249));
    lightPalette.setColor(QPalette::ToolTipBase, QColor(255, 255, 255));
    lightPalette.setColor(QPalette::ToolTipText, QColor(51, 51, 51));
    lightPalette.setColor(QPalette::Text, QColor(51, 51, 51));
    lightPalette.setColor(QPalette::Button, QColor(255, 255, 255));
    lightPalette.setColor(QPalette::ButtonText, QColor(51, 51, 51));
    lightPalette.setColor(QPalette::BrightText, QColor(220, 38, 38));
    lightPalette.setColor(QPalette::Link, QColor(59, 130, 246));
    lightPalette.setColor(QPalette::Highlight, QColor(59, 130, 246));
    lightPalette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    app.setPalette(lightPalette);
    
    // 设置全局样式表，确保所有对话框都使用正确的样式
    app.setStyleSheet(
        "QMessageBox, QDialog {"
        "    background-color: #f5f5f5;"
        "    color: #333333;"
        "    font-family: 'Microsoft YaHei UI', Arial, sans-serif;"
        "}"
        "QMessageBox QPushButton {"
        "    background-color: #3b82f6;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 8px 16px;"
        "    font-weight: 500;"
        "    font-size: 14px;"
        "    min-height: 32px;"
        "    min-width: 80px;"
        "}"
        "QMessageBox QPushButton:hover {"
        "    background-color: #2563eb;"
        "}"
        "QMessageBox QPushButton:pressed {"
        "    background-color: #1d4ed8;"
        "}"
        "QMessageBox QLabel {"
        "    color: #374151;"
        "    font-size: 14px;"
        "}"
        "QMessageBox {"
        "    messagebox-text-interaction-flags: 5;"
        "}"
    );
    
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
