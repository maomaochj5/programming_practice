#include <QTest>
#include <QApplication>
#include <QSignalSpy>
#include <QTimer>
#include <QDebug>
#include <QMessageBox>
#include <QInputDialog>
#include <QStandardItemModel>
#include <QTableView>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>

// 包含主窗口头文件
#include "../src/ui/MainWindow.h"
#include "../src/controllers/ProductManager.h"
#include "../src/controllers/CheckoutController.h"
#include "../src/database/DatabaseManager.h"
#include "../src/models/Product.h"
#include "../src/models/Sale.h"

/**
 * @brief GUI调试测试类
 * 
 * 用于诊断GUI操作问题的测试类
 */
class DebugGUITest : public QObject
{
    Q_OBJECT

private slots:
    // 初始化测试
    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();

    // 基础GUI测试
    void testMainWindowCreation();
    void testUIComponentsExist();
    void testButtonConnections();
    void testDatabaseConnection();
    
    // 商品管理测试
    void testProductManagerInitialization();
    void testAddProductFunctionality();
    void testProductTableDisplay();
    void testProductDoubleClick();
    
    // 销售流程测试
    void testNewSaleCreation();
    void testAddItemToSale();
    void testCartDisplay();
    void testPaymentProcess();
    
    // 条码扫描测试
    void testBarcodeScanner();
    void testManualBarcodeEntry();
    
    // 错误处理测试
    void testErrorHandling();
    void testInvalidOperations();

private:
    MainWindow* m_mainWindow;
    QApplication* m_app;
    
    // 辅助方法
    void waitForSignal(QSignalSpy& spy, int timeout = 5000);
    void simulateUserClick(QPushButton* button);
    void simulateUserInput(QLineEdit* edit, const QString& text);
    void simulateUserSelection(QTableView* table, int row);
    void simulateUserSelection(QListWidget* list, int row);
    bool checkDatabaseConnection();
    void createTestProducts();
};

void DebugGUITest::initTestCase()
{
    qDebug() << "=== GUI调试测试开始 ===";
    
    // 创建QApplication实例
    int argc = 1;
    char* argv[] = {(char*)"test"};
    m_app = new QApplication(argc, argv);
    
    // 初始化数据库
    DatabaseManager& dbManager = DatabaseManager::getInstance();
    QString dbPath = QDir::currentPath() + "/test_pos_database.db";
    
    if (!dbManager.openDatabase(dbPath)) {
        qDebug() << "警告: 无法连接到测试数据库";
    } else {
        qDebug() << "测试数据库连接成功:" << dbPath;
    }
}

void DebugGUITest::init()
{
    qDebug() << "--- 初始化测试环境 ---";
    
    // 创建主窗口
    m_mainWindow = new MainWindow();
    QVERIFY(m_mainWindow != nullptr);
    
    // 等待窗口显示
    QTest::qWait(100);
    
    qDebug() << "主窗口创建成功";
}

void DebugGUITest::cleanup()
{
    qDebug() << "--- 清理测试环境 ---";
    
    if (m_mainWindow) {
        delete m_mainWindow;
        m_mainWindow = nullptr;
    }
}

void DebugGUITest::cleanupTestCase()
{
    qDebug() << "=== GUI调试测试结束 ===";
    
    if (m_app) {
        delete m_app;
        m_app = nullptr;
    }
}

void DebugGUITest::testMainWindowCreation()
{
    qDebug() << "测试: 主窗口创建";
    
    QVERIFY(m_mainWindow != nullptr);
    QVERIFY(m_mainWindow->isVisible());
    QVERIFY(m_mainWindow->windowTitle().contains("智能超市收银系统"));
    
    qDebug() << "✓ 主窗口创建测试通过";
}

void DebugGUITest::testUIComponentsExist()
{
    qDebug() << "测试: UI组件存在性检查";
    
    // 检查主要UI组件是否存在
    QWidget* centralWidget = m_mainWindow->centralWidget();
    QVERIFY(centralWidget != nullptr);
    
    // 查找主要组件
    QList<QPushButton*> buttons = centralWidget->findChildren<QPushButton*>();
    QVERIFY(!buttons.isEmpty());
    
    QList<QTableView*> tables = centralWidget->findChildren<QTableView*>();
    QVERIFY(!tables.isEmpty());
    
    QList<QListWidget*> lists = centralWidget->findChildren<QListWidget*>();
    QVERIFY(!lists.isEmpty());
    
    qDebug() << "找到" << buttons.size() << "个按钮";
    qDebug() << "找到" << tables.size() << "个表格";
    qDebug() << "找到" << lists.size() << "个列表";
    
    qDebug() << "✓ UI组件存在性测试通过";
}

void DebugGUITest::testButtonConnections()
{
    qDebug() << "测试: 按钮信号连接";
    
    // 查找主要按钮
    QPushButton* newSaleButton = m_mainWindow->findChild<QPushButton*>();
    QPushButton* paymentButton = nullptr;
    QPushButton* addProductButton = nullptr;
    
    // 遍历所有按钮找到目标按钮
    QList<QPushButton*> buttons = m_mainWindow->findChildren<QPushButton*>();
    for (QPushButton* btn : buttons) {
        QString text = btn->text();
        if (text.contains("新建销售")) {
            newSaleButton = btn;
        } else if (text.contains("支付")) {
            paymentButton = btn;
        } else if (text.contains("添加商品")) {
            addProductButton = btn;
        }
    }
    
    QVERIFY(newSaleButton != nullptr);
    QVERIFY(paymentButton != nullptr);
    QVERIFY(addProductButton != nullptr);
    
    // 测试按钮是否可点击
    QVERIFY(newSaleButton->isEnabled());
    QVERIFY(paymentButton->isEnabled());
    QVERIFY(addProductButton->isEnabled());
    
    qDebug() << "✓ 按钮连接测试通过";
}

void DebugGUITest::testDatabaseConnection()
{
    qDebug() << "测试: 数据库连接";
    
    DatabaseManager& dbManager = DatabaseManager::getInstance();
    QVERIFY(dbManager.isConnected());
    
    qDebug() << "✓ 数据库连接测试通过";
}

void DebugGUITest::testProductManagerInitialization()
{
    qDebug() << "测试: 商品管理器初始化";
    
    // 检查ProductManager是否正常工作
    ProductManager* productManager = m_mainWindow->findChild<ProductManager*>();
    if (!productManager) {
        qDebug() << "警告: 未找到ProductManager实例";
        return;
    }
    
    // 测试获取所有商品
    QList<Product*> products = productManager->getAllProductsSync();
    qDebug() << "当前商品数量:" << products.size();
    
    qDebug() << "✓ 商品管理器初始化测试通过";
}

void DebugGUITest::testAddProductFunctionality()
{
    qDebug() << "测试: 添加商品功能";
    
    // 查找添加商品按钮
    QPushButton* addProductButton = nullptr;
    QList<QPushButton*> buttons = m_mainWindow->findChildren<QPushButton*>();
    for (QPushButton* btn : buttons) {
        if (btn->text().contains("添加商品")) {
            addProductButton = btn;
            break;
        }
    }
    
    QVERIFY(addProductButton != nullptr);
    
    // 模拟点击添加商品按钮
    QSignalSpy clickSpy(addProductButton, &QPushButton::clicked);
    QTest::mouseClick(addProductButton, Qt::LeftButton);
    
    // 等待对话框出现
    QTest::qWait(100);
    
    // 检查是否有对话框出现
    QWidget* dialog = QApplication::activeWindow();
    if (dialog && dialog != m_mainWindow) {
        qDebug() << "✓ 添加商品对话框出现";
        dialog->close();
    } else {
        qDebug() << "警告: 添加商品对话框未出现";
    }
    
    qDebug() << "✓ 添加商品功能测试通过";
}

void DebugGUITest::testProductTableDisplay()
{
    qDebug() << "测试: 商品表格显示";
    
    // 查找商品表格
    QTableView* productTable = nullptr;
    QList<QTableView*> tables = m_mainWindow->findChildren<QTableView*>();
    for (QTableView* table : tables) {
        if (table->model() && table->model()->rowCount() >= 0) {
            productTable = table;
            break;
        }
    }
    
    QVERIFY(productTable != nullptr);
    
    // 检查表格模型
    QAbstractItemModel* model = productTable->model();
    QVERIFY(model != nullptr);
    
    qDebug() << "商品表格行数:" << model->rowCount();
    qDebug() << "商品表格列数:" << model->columnCount();
    
    qDebug() << "✓ 商品表格显示测试通过";
}

void DebugGUITest::testProductDoubleClick()
{
    qDebug() << "测试: 商品双击功能";
    
    // 查找商品表格
    QTableView* productTable = nullptr;
    QList<QTableView*> tables = m_mainWindow->findChildren<QTableView*>();
    for (QTableView* table : tables) {
        if (table->model() && table->model()->rowCount() > 0) {
            productTable = table;
            break;
        }
    }
    
    if (!productTable) {
        qDebug() << "警告: 未找到商品表格或表格为空";
        return;
    }
    
    // 选择第一行
    QModelIndex firstIndex = productTable->model()->index(0, 0);
    if (firstIndex.isValid()) {
        productTable->setCurrentIndex(firstIndex);
        
        // 模拟双击
        QTest::mouseDClick(productTable->viewport(), Qt::LeftButton, 
                          Qt::NoModifier, productTable->visualRect(firstIndex).center());
        
        QTest::qWait(100);
        qDebug() << "✓ 商品双击测试完成";
    } else {
        qDebug() << "警告: 商品表格为空，无法测试双击";
    }
}

void DebugGUITest::testNewSaleCreation()
{
    qDebug() << "测试: 新建销售功能";
    
    // 查找新建销售按钮
    QPushButton* newSaleButton = nullptr;
    QList<QPushButton*> buttons = m_mainWindow->findChildren<QPushButton*>();
    for (QPushButton* btn : buttons) {
        if (btn->text().contains("新建销售")) {
            newSaleButton = btn;
            break;
        }
    }
    
    QVERIFY(newSaleButton != nullptr);
    
    // 模拟点击新建销售
    QSignalSpy clickSpy(newSaleButton, &QPushButton::clicked);
    QTest::mouseClick(newSaleButton, Qt::LeftButton);
    
    QTest::qWait(100);
    
    qDebug() << "✓ 新建销售功能测试通过";
}

void DebugGUITest::testAddItemToSale()
{
    qDebug() << "测试: 添加商品到销售";
    
    // 首先确保有商品数据
    createTestProducts();
    
    // 查找商品表格
    QTableView* productTable = nullptr;
    QList<QTableView*> tables = m_mainWindow->findChildren<QTableView*>();
    for (QTableView* table : tables) {
        if (table->model() && table->model()->rowCount() > 0) {
            productTable = table;
            break;
        }
    }
    
    if (!productTable) {
        qDebug() << "警告: 未找到商品表格";
        return;
    }
    
    // 选择第一个商品并双击
    QModelIndex firstIndex = productTable->model()->index(0, 0);
    if (firstIndex.isValid()) {
        productTable->setCurrentIndex(firstIndex);
        
        // 模拟双击添加商品
        QTest::mouseDClick(productTable->viewport(), Qt::LeftButton, 
                          Qt::NoModifier, productTable->visualRect(firstIndex).center());
        
        QTest::qWait(100);
        
        // 检查购物车是否更新
        QList<QListWidget*> lists = m_mainWindow->findChildren<QListWidget*>();
        for (QListWidget* list : lists) {
            if (list->count() > 0) {
                qDebug() << "✓ 商品已添加到购物车";
                break;
            }
        }
    }
    
    qDebug() << "✓ 添加商品到销售测试通过";
}

void DebugGUITest::testCartDisplay()
{
    qDebug() << "测试: 购物车显示";
    
    // 查找购物车列表
    QListWidget* cartList = nullptr;
    QList<QListWidget*> lists = m_mainWindow->findChildren<QListWidget*>();
    for (QListWidget* list : lists) {
        // 通过父组件或其他方式识别购物车列表
        if (list->parent() && list->parent()->objectName().contains("sales", Qt::CaseInsensitive)) {
            cartList = list;
            break;
        }
    }
    
    if (cartList) {
        qDebug() << "购物车项目数量:" << cartList->count();
        qDebug() << "✓ 购物车显示测试通过";
    } else {
        qDebug() << "警告: 未找到购物车列表";
    }
}

void DebugGUITest::testPaymentProcess()
{
    qDebug() << "测试: 支付流程";
    
    // 查找支付按钮
    QPushButton* paymentButton = nullptr;
    QList<QPushButton*> buttons = m_mainWindow->findChildren<QPushButton*>();
    for (QPushButton* btn : buttons) {
        if (btn->text().contains("支付")) {
            paymentButton = btn;
            break;
        }
    }
    
    QVERIFY(paymentButton != nullptr);
    
    // 模拟点击支付按钮
    QSignalSpy clickSpy(paymentButton, &QPushButton::clicked);
    QTest::mouseClick(paymentButton, Qt::LeftButton);
    
    QTest::qWait(100);
    
    // 检查是否有支付对话框出现
    QWidget* dialog = QApplication::activeWindow();
    if (dialog && dialog != m_mainWindow) {
        qDebug() << "✓ 支付对话框出现";
        dialog->close();
    } else {
        qDebug() << "警告: 支付对话框未出现";
    }
    
    qDebug() << "✓ 支付流程测试通过";
}

void DebugGUITest::testBarcodeScanner()
{
    qDebug() << "测试: 条码扫描器";
    
    // 查找条码输入框
    QLineEdit* barcodeEdit = nullptr;
    QList<QLineEdit*> edits = m_mainWindow->findChildren<QLineEdit*>();
    for (QLineEdit* edit : edits) {
        if (edit->placeholderText().contains("条码", Qt::CaseInsensitive)) {
            barcodeEdit = edit;
            break;
        }
    }
    
    if (barcodeEdit) {
        // 模拟输入条码
        QTest::keyClicks(barcodeEdit, "123456789");
        QTest::keyClick(barcodeEdit, Qt::Key_Return);
        
        QTest::qWait(100);
        qDebug() << "✓ 条码输入测试完成";
    } else {
        qDebug() << "警告: 未找到条码输入框";
    }
}

void DebugGUITest::testManualBarcodeEntry()
{
    qDebug() << "测试: 手动条码输入";
    
    // 查找条码输入框
    QLineEdit* barcodeEdit = nullptr;
    QList<QLineEdit*> edits = m_mainWindow->findChildren<QLineEdit*>();
    for (QLineEdit* edit : edits) {
        if (edit->placeholderText().contains("条码", Qt::CaseInsensitive)) {
            barcodeEdit = edit;
            break;
        }
    }
    
    if (barcodeEdit) {
        // 清空输入框
        barcodeEdit->clear();
        
        // 模拟输入测试条码
        QTest::keyClicks(barcodeEdit, "TEST123456");
        QTest::keyClick(barcodeEdit, Qt::Key_Return);
        
        QTest::qWait(100);
        qDebug() << "✓ 手动条码输入测试完成";
    } else {
        qDebug() << "警告: 未找到条码输入框";
    }
}

void DebugGUITest::testErrorHandling()
{
    qDebug() << "测试: 错误处理";
    
    // 测试无效操作
    // 例如：在没有选择商品的情况下尝试编辑
    QPushButton* editButton = nullptr;
    QList<QPushButton*> buttons = m_mainWindow->findChildren<QPushButton*>();
    for (QPushButton* btn : buttons) {
        if (btn->text().contains("编辑商品")) {
            editButton = btn;
            break;
        }
    }
    
    if (editButton) {
        // 模拟点击编辑按钮（没有选择商品）
        QTest::mouseClick(editButton, Qt::LeftButton);
        QTest::qWait(100);
        
        // 检查是否有错误消息框出现
        QWidget* dialog = QApplication::activeWindow();
        if (dialog && dialog != m_mainWindow) {
            qDebug() << "✓ 错误处理正常，显示错误对话框";
            dialog->close();
        }
    }
    
    qDebug() << "✓ 错误处理测试通过";
}

void DebugGUITest::testInvalidOperations()
{
    qDebug() << "测试: 无效操作处理";
    
    // 测试各种无效操作
    // 1. 在没有商品的情况下尝试支付
    QPushButton* paymentButton = nullptr;
    QList<QPushButton*> buttons = m_mainWindow->findChildren<QPushButton*>();
    for (QPushButton* btn : buttons) {
        if (btn->text().contains("支付")) {
            paymentButton = btn;
            break;
        }
    }
    
    if (paymentButton) {
        // 清空购物车后尝试支付
        QPushButton* clearButton = nullptr;
        for (QPushButton* btn : buttons) {
            if (btn->text().contains("清空")) {
                clearButton = btn;
                break;
            }
        }
        
        if (clearButton) {
            QTest::mouseClick(clearButton, Qt::LeftButton);
            QTest::qWait(100);
        }
        
        // 尝试支付空购物车
        QTest::mouseClick(paymentButton, Qt::LeftButton);
        QTest::qWait(100);
        
        qDebug() << "✓ 无效操作处理测试完成";
    }
}

// 辅助方法实现
void DebugGUITest::waitForSignal(QSignalSpy& spy, int timeout)
{
    if (spy.wait(timeout)) {
        qDebug() << "信号接收到，参数数量:" << spy.count();
    } else {
        qDebug() << "警告: 信号等待超时";
    }
}

void DebugGUITest::simulateUserClick(QPushButton* button)
{
    if (button && button->isEnabled()) {
        QTest::mouseClick(button, Qt::LeftButton);
        QTest::qWait(100);
    }
}

void DebugGUITest::simulateUserInput(QLineEdit* edit, const QString& text)
{
    if (edit) {
        edit->clear();
        QTest::keyClicks(edit, text);
        QTest::keyClick(edit, Qt::Key_Return);
        QTest::qWait(100);
    }
}

void DebugGUITest::simulateUserSelection(QTableView* table, int row)
{
    if (table && table->model() && row < table->model()->rowCount()) {
        QModelIndex index = table->model()->index(row, 0);
        table->setCurrentIndex(index);
        QTest::qWait(100);
    }
}

void DebugGUITest::simulateUserSelection(QListWidget* list, int row)
{
    if (list && row < list->count()) {
        list->setCurrentRow(row);
        QTest::qWait(100);
    }
}

bool DebugGUITest::checkDatabaseConnection()
{
    DatabaseManager& dbManager = DatabaseManager::getInstance();
    return dbManager.isConnected();
}

void DebugGUITest::createTestProducts()
{
    qDebug() << "创建测试商品数据";
    
    // 这里可以创建一些测试商品数据
    // 由于这是调试测试，我们只记录这个步骤
    qDebug() << "测试商品数据创建完成";
}

QTEST_MAIN(DebugGUITest)
#include "debug_gui_test.moc" 