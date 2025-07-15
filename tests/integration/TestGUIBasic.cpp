#include <QTest>
#include <QApplication>
#include <QTimer>
#include <QDebug>
#include <QTemporaryFile>
#include <QFile>
#include "../../src/ui/MainWindow.h"
#include "../../src/database/DatabaseManager.h"
#include "../../src/models/Product.h"
#include "../../src/models/Customer.h"
#include "../TestCommon.h"

class TestGUIBasic : public TestCommon
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 基础GUI测试用例
    void testMainWindowCreation();
    void testMainWindowDestruction();
    void testWindowProperties();

private:
    DatabaseManager* m_databaseManager;
    QString m_testDbPath;
    
    void setupDatabase();
};

void TestGUIBasic::initTestCase()
{
    qDebug() << "Starting Basic GUI Tests";
    
    // 注册自定义类型用于信号和槽
    qRegisterMetaType<Product*>("Product*");
    qRegisterMetaType<Customer*>("Customer*");
    
    // 创建临时数据库文件
    QTemporaryFile tempFile;
    if (tempFile.open()) {
        m_testDbPath = tempFile.fileName();
        tempFile.close();
    }
    QVERIFY(!m_testDbPath.isEmpty());
}

void TestGUIBasic::cleanupTestCase()
{
    qDebug() << "Finished Basic GUI Tests";
    
    // 清理临时数据库文件
    if (!m_testDbPath.isEmpty()) {
        QFile::remove(m_testDbPath);
    }
}

void TestGUIBasic::init()
{
    setupDatabase();
}

void TestGUIBasic::cleanup()
{
    if (m_databaseManager) {
        m_databaseManager->closeDatabase();
    }
}

void TestGUIBasic::setupDatabase()
{
    m_databaseManager = &DatabaseManager::getInstance();
    bool success = m_databaseManager->openDatabase(m_testDbPath);
    QVERIFY(success);
}

void TestGUIBasic::testMainWindowCreation()
{
    qDebug() << "Testing main window creation...";
    
    // 创建主窗口
    MainWindow* mainWindow = new MainWindow();
    QVERIFY(mainWindow != nullptr);
    qDebug() << "MainWindow created successfully";
    
    // 测试窗口标题
    QString title = mainWindow->windowTitle();
    qDebug() << "Window title:" << title;
    QVERIFY(!title.isEmpty());
    
    // 立即删除窗口
    delete mainWindow;
    qDebug() << "MainWindow deleted successfully";
    
    qDebug() << "Main window creation test finished successfully";
}

void TestGUIBasic::testMainWindowDestruction()
{
    qDebug() << "Testing main window destruction...";
    
    // 创建多个窗口并销毁，测试内存管理
    for (int i = 0; i < 3; ++i) {
        MainWindow* mainWindow = new MainWindow();
        QVERIFY(mainWindow != nullptr);
        
        // 设置窗口可见但不实际显示
        mainWindow->setVisible(false);
        
        // 立即销毁
        delete mainWindow;
        qDebug() << "Window" << i+1 << "created and destroyed successfully";
    }
    
    qDebug() << "Main window destruction test finished successfully";
}

void TestGUIBasic::testWindowProperties()
{
    qDebug() << "Testing window properties...";
    
    MainWindow* mainWindow = new MainWindow();
    QVERIFY(mainWindow != nullptr);
    
    // 测试默认属性
    qDebug() << "Default window size:" << mainWindow->size();
    qDebug() << "Is visible:" << mainWindow->isVisible();
    qDebug() << "Is enabled:" << mainWindow->isEnabled();
    
    // 基本断言
    QVERIFY(mainWindow->isEnabled());
    QVERIFY(mainWindow->size().width() > 0);
    QVERIFY(mainWindow->size().height() > 0);
    
    delete mainWindow;
    qDebug() << "Window properties test finished successfully";
}

#include "TestGUIBasic.moc"

QTEST_MAIN(TestGUIBasic)
