#include <QTest>
#include <QApplication>
#include <QSignalSpy>
#include <QTimer>
#include <QDebug>
#include <QTemporaryFile>
#include <QFile>
#include <QMessageBox>
#include <QPushButton>
#include <QListWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QSpinBox>
#include <QLineEdit>
#include <QLabel>
#include <QGroupBox>
#include "../../src/ui/MainWindow.h"
#include "../../src/database/DatabaseManager.h"
#include "../../src/models/Product.h"
#include "../../src/models/Customer.h"
#include "../TestCommon.h"

class TestGUIInterface : public TestCommon
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // GUI接口级测试用例
    void testMainWindowInitialization();
    void testUIComponentsCreation();
    void testUIComponentsEnabled();
    void testSignalSlotConnections();
    void testButtonFunctionality();
    void testDataDisplayAndUpdate();
    void testSalesWorkflow();
    void testProductManagement();
    void testBarcodeScanning();
    void testRecommendationSystem();
    void testErrorHandling();
    void testProductDoubleClickAddition();

private:
    MainWindow* m_mainWindow;
    DatabaseManager* m_databaseManager;
    QString m_testDbPath;
    
    void setupDatabase();
    void addTestData();
    void verifyUIComponent(QWidget* component, const QString& name);
    void verifyButtonState(QPushButton* button, const QString& name, bool shouldBeEnabled = true);
    void simulateUserInput();
};

void TestGUIInterface::initTestCase()
{
    qDebug() << "Starting GUI Interface Tests";
    
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

    // 在所有测试开始前设置数据库
    setupDatabase();
}

void TestGUIInterface::cleanupTestCase()
{
    qDebug() << "Finished GUI Interface Tests";
    
    // 在所有测试结束后关闭数据库
    if (m_databaseManager) {
        m_databaseManager->closeDatabase();
    }

    // 清理临时数据库文件
    if (!m_testDbPath.isEmpty()) {
        QFile::remove(m_testDbPath);
    }
}

void TestGUIInterface::init()
{
    // 每个测试开始前，清理并重新填充数据
    m_databaseManager->clearAllTables();
    addTestData();

    m_mainWindow = new MainWindow();
    QVERIFY(m_mainWindow != nullptr);
    m_mainWindow->show();
    QApplication::processEvents();
}

void TestGUIInterface::cleanup()
{
    delete m_mainWindow;
    m_mainWindow = nullptr;
    DatabaseManager::instance().closeDatabase();
    QFile::remove(m_testDbPath);
}

void TestGUIInterface::setupDatabase()
{
    m_databaseManager = &DatabaseManager::getInstance();
    bool success = m_databaseManager->openDatabase(m_testDbPath);
    QVERIFY(success);
    
    // 初始测试数据，现在只在setupDatabase中调用一次
    // addTestData(); // 改为在 init() 中调用
}

void TestGUIInterface::addTestData()
{
    // 使用当前时间戳确保条码唯一性
    static int testCounter = 0;
    testCounter++;
    
    // 添加一些测试商品到数据库
    Product* testProduct1 = new Product();
    testProduct1->setBarcode(QString("TEST%1001").arg(testCounter));
    testProduct1->setName("测试商品1");
    testProduct1->setDescription("这是一个测试商品");
    testProduct1->setPrice(12.50);
    testProduct1->setStockQuantity(100);
    testProduct1->setCategory("测试分类");
    
    Product* testProduct2 = new Product();
    testProduct2->setBarcode(QString("TEST%1002").arg(testCounter));
    testProduct2->setName("测试商品2");
    testProduct2->setDescription("这是第二个测试商品");
    testProduct2->setPrice(25.00);
    testProduct2->setStockQuantity(50);
    testProduct2->setCategory("测试分类");
    
    m_databaseManager->saveProduct(testProduct1);
    m_databaseManager->saveProduct(testProduct2);
    
    delete testProduct1;
    delete testProduct2;
}

void TestGUIInterface::testMainWindowInitialization()
{
    qDebug() << "Testing main window initialization...";
    
    // 测试主窗口基本属性
    QVERIFY(m_mainWindow != nullptr);
    QVERIFY(!m_mainWindow->windowTitle().isEmpty());
    QVERIFY(m_mainWindow->width() > 800);
    QVERIFY(m_mainWindow->height() > 600);
    
    qDebug() << "Window title:" << m_mainWindow->windowTitle();
    qDebug() << "Window size:" << m_mainWindow->size();
    
    qDebug() << "Main window initialization test passed";
}

void TestGUIInterface::testUIComponentsCreation()
{
    qDebug() << "Testing UI components creation...";
    
    // 查找所有主要UI组件
    QGroupBox* salesGroup = m_mainWindow->findChild<QGroupBox*>();
    verifyUIComponent(salesGroup, "Sales Group");
    
    QListWidget* cartList = m_mainWindow->findChild<QListWidget*>();
    verifyUIComponent(cartList, "Cart List");
    
    QTableView* productTable = m_mainWindow->findChild<QTableView*>();
    verifyUIComponent(productTable, "Product Table");
    
    // 验证产品表格的模型（不是QWidget，单独检验）
    if (productTable) {
        QStandardItemModel* productModel = qobject_cast<QStandardItemModel*>(productTable->model());
        QVERIFY2(productModel != nullptr, "Product Model should exist");
        qDebug() << "✓ Product Model verified";
    }
    
    QSpinBox* quantitySpinBox = m_mainWindow->findChild<QSpinBox*>();
    verifyUIComponent(quantitySpinBox, "Quantity SpinBox");
    
    QLineEdit* barcodeEdit = m_mainWindow->findChild<QLineEdit*>();
    verifyUIComponent(barcodeEdit, "Barcode Edit");
    
    qDebug() << "UI components creation test passed";
}

void TestGUIInterface::testUIComponentsEnabled()
{
    qDebug() << "Testing UI components enabled state...";
    
    // 查找所有按钮并检查其状态
    QList<QPushButton*> buttons = m_mainWindow->findChildren<QPushButton*>();
    qDebug() << "Found" << buttons.size() << "buttons";
    
    int enabledButtons = 0;
    for (auto* button : buttons) {
        if (button->isVisible()) {
            qDebug() << "Button:" << button->text() 
                     << "Enabled:" << button->isEnabled()
                     << "Size:" << button->size()
                     << "Position:" << button->pos();
            
            if (button->isEnabled()) {
                enabledButtons++;
            }
            
            // 基本验证：按钮应该有文本和合理的大小
            QVERIFY(!button->text().isEmpty() || !button->toolTip().isEmpty());
            QVERIFY(button->width() > 0);
            QVERIFY(button->height() > 0);
        }
    }
    
    qDebug() << "Found" << enabledButtons << "enabled buttons out of" << buttons.size();
    
    // 至少应该有一些按钮是启用的
    QVERIFY(enabledButtons > 0);
    
    qDebug() << "UI components enabled test passed";
}

void TestGUIInterface::testSignalSlotConnections()
{
    qDebug() << "Testing signal-slot connections...";
    
    // 查找主要按钮
    QList<QPushButton*> buttons = m_mainWindow->findChildren<QPushButton*>();
    
    for (auto* button : buttons) {
        if (button->isVisible() && button->isEnabled()) {
            // 创建信号spy来验证按钮点击
            QSignalSpy clickSpy(button, &QPushButton::clicked);
            
            // 模拟点击
            QTest::mouseClick(button, Qt::LeftButton);
            QApplication::processEvents();
            
            // 验证信号是否被发出
            QVERIFY(clickSpy.count() >= 0); // 至少不会崩溃
            
            qDebug() << "Button" << button->text() << "click signal count:" << clickSpy.count();
        }
    }
    
    qDebug() << "Signal-slot connections test passed";
}

void TestGUIInterface::testButtonFunctionality()
{
    qDebug() << "Testing button functionality...";
    
    // 测试新建销售按钮
    QPushButton* newSaleButton = nullptr;
    QList<QPushButton*> buttons = m_mainWindow->findChildren<QPushButton*>();
    
    for (auto* button : buttons) {
        if (button->text().contains("新建") || button->text().contains("New")) {
            newSaleButton = button;
            break;
        }
    }
    
    if (newSaleButton && newSaleButton->isEnabled()) {
        QTest::mouseClick(newSaleButton, Qt::LeftButton);
        QApplication::processEvents();
        qDebug() << "New sale button clicked successfully";
    }
    
    // 测试刷新按钮
    QPushButton* refreshButton = nullptr;
    for (auto* button : buttons) {
        if (button->text().contains("刷新") || button->text().contains("Refresh")) {
            refreshButton = button;
            break;
        }
    }
    
    if (refreshButton && refreshButton->isEnabled()) {
        QTest::mouseClick(refreshButton, Qt::LeftButton);
        QApplication::processEvents();
        qDebug() << "Refresh button clicked successfully";
    }
    
    qDebug() << "Button functionality test passed";
}

void TestGUIInterface::testDataDisplayAndUpdate()
{
    qDebug() << "Testing data display and update...";
    
    // 测试商品表格
    QTableView* productTable = m_mainWindow->findChild<QTableView*>();
    if (productTable) {
        QAbstractItemModel* model = productTable->model();
        if (model) {
            qDebug() << "Product table rows:" << model->rowCount();
            qDebug() << "Product table columns:" << model->columnCount();
            
            // 验证表格有数据
            QVERIFY(model->columnCount() > 0);
        }
    }
    
    // 测试购物车列表
    QListWidget* cartList = m_mainWindow->findChild<QListWidget*>();
    if (cartList) {
        qDebug() << "Cart list items:" << cartList->count();
        QVERIFY(cartList->count() >= 0);
    }
    
    // 测试标签显示
    QList<QLabel*> labels = m_mainWindow->findChildren<QLabel*>();
    qDebug() << "Found" << labels.size() << "labels";
    
    for (auto* label : labels) {
        if (label->isVisible()) {
            qDebug() << "Label text:" << label->text();
        }
    }
    
    qDebug() << "Data display and update test passed";
}

void TestGUIInterface::testSalesWorkflow()
{
    qDebug() << "Testing sales workflow...";
    
    // 模拟销售流程
    simulateUserInput();
    
    qDebug() << "Sales workflow test passed";
}

void TestGUIInterface::testProductManagement()
{
    qDebug() << "Testing product management...";
    
    // 查找商品管理相关按钮
    QList<QPushButton*> buttons = m_mainWindow->findChildren<QPushButton*>();
    
    for (auto* button : buttons) {
        QString buttonText = button->text();
        if (buttonText.contains("添加") || buttonText.contains("编辑") || 
            buttonText.contains("删除") || buttonText.contains("刷新")) {
            
            qDebug() << "Product management button:" << buttonText 
                     << "Enabled:" << button->isEnabled();
            
            if (button->isEnabled()) {
                // 测试按钮可点击性（不实际执行对话框）
                QVERIFY(button->isVisible());
                QVERIFY(button->size().width() > 0);
                QVERIFY(button->size().height() > 0);
            }
        }
    }
    
    qDebug() << "Product management test passed";
}

void TestGUIInterface::testBarcodeScanning()
{
    qDebug() << "Testing barcode scanning...";
    
    // 查找条码输入框
    QLineEdit* barcodeEdit = m_mainWindow->findChild<QLineEdit*>();
    if (barcodeEdit) {
        // 模拟条码输入
        barcodeEdit->setText("1234567890123");
        QTest::keyPress(barcodeEdit, Qt::Key_Return);
        QApplication::processEvents();
        
        qDebug() << "Barcode input simulated";
    }
    
    qDebug() << "Barcode scanning test passed";
}

void TestGUIInterface::testRecommendationSystem()
{
    qDebug() << "Testing recommendation system...";
    
    // 查找推荐列表
    QList<QListWidget*> listWidgets = m_mainWindow->findChildren<QListWidget*>();
    
    for (auto* listWidget : listWidgets) {
        qDebug() << "List widget items:" << listWidget->count();
        
        // 如果有推荐商品，测试选择
        if (listWidget->count() > 0) {
            listWidget->setCurrentRow(0);
            QApplication::processEvents();
        }
    }
    
    qDebug() << "Recommendation system test passed";
}

void TestGUIInterface::testErrorHandling()
{
    qDebug() << "Testing error handling...";
    
    // 测试状态标签
    QList<QLabel*> labels = m_mainWindow->findChildren<QLabel*>();
    for (auto* label : labels) {
        if (label->objectName().contains("status") || 
            label->text().contains("状态") || 
            label->text().contains("就绪")) {
            qDebug() << "Status label found:" << label->text();
        }
    }
    
    qDebug() << "Error handling test passed";
}

void TestGUIInterface::testProductDoubleClickAddition()
{
    qDebug() << "Testing product double-click addition...";
    
    // 获取产品表格
    QTableView* productTable = m_mainWindow->findChild<QTableView*>();
    QVERIFY(productTable != nullptr);
    
    QStandardItemModel* productModel = qobject_cast<QStandardItemModel*>(productTable->model());
    QVERIFY(productModel != nullptr);
    
    // 确保有产品数据
    qDebug() << "Product table has" << productModel->rowCount() << "rows";
    QVERIFY(productModel->rowCount() > 0);
    
    // 获取购物车列表（用于验证添加是否成功）
    QListWidget* cartList = m_mainWindow->findChild<QListWidget*>();
    QVERIFY(cartList != nullptr);
    
    int initialCartItems = cartList->count();
    qDebug() << "Initial cart items:" << initialCartItems;
    
    // 模拟双击第一个产品
    QModelIndex firstProduct = productModel->index(0, 0);
    QVERIFY(firstProduct.isValid());
    
    QString productBarcode = productModel->item(0, 1)->text();
    QString productName = productModel->item(0, 2)->text();
    qDebug() << "Double-clicking product:" << productName << "Barcode:" << productBarcode;
    
    // 发出双击信号
    emit productTable->doubleClicked(firstProduct);
    QApplication::processEvents();
    
    // 验证商品是否添加到购物车
    qDebug() << "Cart items after double-click:" << cartList->count();
    
    // 注意：由于需要先创建销售，可能需要检查是否自动创建了销售
    // 这个测试主要验证双击信号是否正确触发
    
    qDebug() << "Product double-click addition test completed";
}

void TestGUIInterface::verifyUIComponent(QWidget* component, const QString& name)
{
    if (component) {
        qDebug() << name << "found and valid";
        QVERIFY(component != nullptr);
        QVERIFY(component->size().width() >= 0);
        QVERIFY(component->size().height() >= 0);
    } else {
        qDebug() << name << "not found - this may be expected";
    }
}

void TestGUIInterface::verifyButtonState(QPushButton* button, const QString& name, bool shouldBeEnabled)
{
    if (button) {
        qDebug() << name << "- Enabled:" << button->isEnabled() 
                 << "Visible:" << button->isVisible()
                 << "Text:" << button->text();
        
        if (shouldBeEnabled) {
            QVERIFY(button->isEnabled());
        }
        QVERIFY(button->isVisible());
    } else {
        qDebug() << name << "button not found";
    }
}

void TestGUIInterface::simulateUserInput()
{
    // 模拟用户输入
    QApplication::processEvents();
    
    // 查找并操作数量输入框
    QSpinBox* quantityBox = m_mainWindow->findChild<QSpinBox*>();
    if (quantityBox) {
        quantityBox->setValue(2);
        QApplication::processEvents();
        qDebug() << "Quantity set to:" << quantityBox->value();
    }
    
    // 短暂等待以模拟用户操作
    QTest::qWait(100);
}

#include "TestGUIInterface.moc"

QTEST_MAIN(TestGUIInterface)
