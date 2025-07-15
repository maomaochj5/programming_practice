#include <QTest>
#include <QSignalSpy>
#include "../../src/models/Product.h"
#include "../TestCommon.h"

class TestProduct : public TestCommon
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 基本功能测试
    void testConstructor();
    void testSettersAndGetters();
    void testCopyConstructor();
    void testAssignmentOperator();
    
    // 业务逻辑测试
    void testValidation();
    void testPriceCalculation();
    void testStockManagement();
    void testCategoryHandling();
    void testToString();
    
    // 信号测试
    void testProductChangedSignal();
    void testStockChangedSignal();
    void testPriceChangedSignal();

private:
    Product* m_product;
};

void TestProduct::initTestCase()
{
    // 测试套件初始化
    qDebug() << "Starting Product model tests";
}

void TestProduct::cleanupTestCase()
{
    // 测试套件清理
    qDebug() << "Finished Product model tests";
}

void TestProduct::init()
{
    // 每个测试前的初始化
    m_product = new Product(this);
}

void TestProduct::cleanup()
{
    // 每个测试后的清理
    if (m_product) {
        delete m_product;
        m_product = nullptr;
    }
}

void TestProduct::testConstructor()
{
    // 测试默认构造函数
    Product product;
    QCOMPARE(product.getProductId(), -1);
    QVERIFY(product.getBarcode().isEmpty());
    QVERIFY(product.getName().isEmpty());
    QVERIFY(product.getDescription().isEmpty());
    QCOMPARE(product.getPrice(), 0.0);
    QCOMPARE(product.getStockQuantity(), 0);
    QVERIFY(product.getCategory().isEmpty());
    
    // 测试参数构造函数
    Product product2(1, "1234567890123", "Test Product", "Description", 19.99, 10, "Category");
    QCOMPARE(product2.getBarcode(), QString("1234567890123"));
    QCOMPARE(product2.getName(), QString("Test Product"));
    QCOMPARE(product2.getPrice(), 19.99);
}

void TestProduct::testSettersAndGetters()
{
    // 测试ProductId
    m_product->setProductId(123);
    QCOMPARE(m_product->getProductId(), 123);
    
    // 测试Barcode
    QString testBarcode = "1234567890123";
    m_product->setBarcode(testBarcode);
    QCOMPARE(m_product->getBarcode(), testBarcode);
    
    // 测试Name
    QString testName = "测试商品";
    m_product->setName(testName);
    QCOMPARE(m_product->getName(), testName);
    
    // 测试Description
    QString testDesc = "这是一个测试商品的描述";
    m_product->setDescription(testDesc);
    QCOMPARE(m_product->getDescription(), testDesc);
    
    // 测试Price
    double testPrice = 25.99;
    m_product->setPrice(testPrice);
    QVERIFY(doubleEquals(m_product->getPrice(), testPrice));
    
    // 测试StockQuantity
    int testStock = 100;
    m_product->setStockQuantity(testStock);
    QCOMPARE(m_product->getStockQuantity(), testStock);
    
    // 测试Category
    QString testCategory = "电子产品";
    m_product->setCategory(testCategory);
    QCOMPARE(m_product->getCategory(), testCategory);
}

void TestProduct::testCopyConstructor()
{
    // 设置原始商品数据
    m_product->setProductId(456);
    m_product->setBarcode("9876543210987");
    m_product->setName("原始商品");
    m_product->setDescription("原始描述");
    m_product->setPrice(99.99);
    m_product->setStockQuantity(50);
    m_product->setCategory("测试分类");
    
    // 复制构造
    Product copiedProduct(*m_product);
    
    // 验证所有字段都被正确复制
    QCOMPARE(copiedProduct.getProductId(), m_product->getProductId());
    QCOMPARE(copiedProduct.getBarcode(), m_product->getBarcode());
    QCOMPARE(copiedProduct.getName(), m_product->getName());
    QCOMPARE(copiedProduct.getDescription(), m_product->getDescription());
    QVERIFY(doubleEquals(copiedProduct.getPrice(), m_product->getPrice()));
    QCOMPARE(copiedProduct.getStockQuantity(), m_product->getStockQuantity());
    QCOMPARE(copiedProduct.getCategory(), m_product->getCategory());
}

void TestProduct::testAssignmentOperator()
{
    // 设置源商品
    Product sourceProduct;
    sourceProduct.setProductId(789);
    sourceProduct.setBarcode("1111111111111");
    sourceProduct.setName("源商品");
    sourceProduct.setPrice(15.50);
    
    // 赋值操作
    *m_product = sourceProduct;
    
    // 验证赋值结果
    QCOMPARE(m_product->getProductId(), sourceProduct.getProductId());
    QCOMPARE(m_product->getBarcode(), sourceProduct.getBarcode());
    QCOMPARE(m_product->getName(), sourceProduct.getName());
    QVERIFY(doubleEquals(m_product->getPrice(), sourceProduct.getPrice()));
}

void TestProduct::testValidation()
{
    // 测试有效商品
    m_product->setBarcode("1234567890123");
    m_product->setName("有效商品");
    m_product->setPrice(10.0);
    m_product->setStockQuantity(5);
    QVERIFY(m_product->isValid());
    
    // 测试无效条码
    m_product->setBarcode("");
    QVERIFY(!m_product->isValid());
    
    // 测试无效名称
    m_product->setBarcode("1234567890123");
    m_product->setName("");
    QVERIFY(!m_product->isValid());
    
    // 测试负价格
    m_product->setName("有效商品");
    m_product->setPrice(-1.0);
    QVERIFY(!m_product->isValid());
    
    // 测试负库存
    m_product->setPrice(10.0);
    m_product->setStockQuantity(-1);
    QVERIFY(!m_product->isValid());
}

void TestProduct::testPriceCalculation()
{
    // Product类没有calculateDiscountedPrice方法，跳过这个测试
    // 或者可以在这里测试基本的价格设置和获取
    m_product->setPrice(100.0);
    QVERIFY(doubleEquals(m_product->getPrice(), 100.0));
    
    m_product->setPrice(0.0);
    QVERIFY(doubleEquals(m_product->getPrice(), 0.0));
    
    m_product->setPrice(999.99);
    QVERIFY(doubleEquals(m_product->getPrice(), 999.99));
}

void TestProduct::testStockManagement()
{
    m_product->setStockQuantity(100);
    
    // 测试减少库存
    bool result = m_product->decreaseStock(30);
    QVERIFY(result);
    QCOMPARE(m_product->getStockQuantity(), 70);
    
    // 测试库存不足
    result = m_product->decreaseStock(80);
    QVERIFY(!result);
    QCOMPARE(m_product->getStockQuantity(), 70); // 库存应该不变
    
    // 测试增加库存
    m_product->increaseStock(20);
    QCOMPARE(m_product->getStockQuantity(), 90);
    
    // 测试检查低库存 - Product类没有isLowStock方法，测试基本库存功能
    m_product->setStockQuantity(3);
    QVERIFY(m_product->isInStock());
    
    m_product->setStockQuantity(0);
    QVERIFY(!m_product->isInStock());
}

void TestProduct::testCategoryHandling()
{
    // 测试分类设置
    QString category1 = "食品";
    m_product->setCategory(category1);
    QCOMPARE(m_product->getCategory(), category1);
    
    // 测试空分类
    m_product->setCategory("");
    QCOMPARE(m_product->getCategory(), QString(""));
    
    // 测试特殊字符分类
    QString specialCategory = "电子产品/手机/智能手机";
    m_product->setCategory(specialCategory);
    QCOMPARE(m_product->getCategory(), specialCategory);
}

void TestProduct::testToString()
{
    m_product->setBarcode("1234567890123");
    m_product->setName("测试商品");
    m_product->setPrice(19.99);
    m_product->setStockQuantity(50);
    
    QString str = m_product->toString();
    
    // 验证字符串包含关键信息
    QVERIFY(str.contains("1234567890123"));
    QVERIFY(str.contains("测试商品"));
    QVERIFY(str.contains("19.99"));
    QVERIFY(str.contains("50"));
}

void TestProduct::testProductChangedSignal()
{
    QSignalSpy spy(m_product, &Product::productChanged);
    
    // 修改商品应该发射信号
    m_product->setName("新名称");
    QVERIFY(verifySignalEmitted(spy, 1));
    
    // 设置相同值不应该发射信号
    spy.clear();
    m_product->setName("新名称");
    QVERIFY(verifySignalEmitted(spy, 0));
}

void TestProduct::testStockChangedSignal()
{
    QSignalSpy spy(m_product, &Product::stockChanged);
    
    // 修改库存应该发射信号
    m_product->setStockQuantity(100);
    QVERIFY(verifySignalEmitted(spy, 1));
    
    // 验证信号参数
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toInt(), 100);
}

void TestProduct::testPriceChangedSignal()
{
    // Product类的setPrice方法会发射productChanged信号，不是priceChanged
    QSignalSpy spy(m_product, &Product::productChanged);
    
    // 修改价格应该发射信号
    m_product->setPrice(25.99);
    QVERIFY(verifySignalEmitted(spy, 1));
    
    // 设置相同价格不应该发射信号
    spy.clear();
    m_product->setPrice(25.99);
    QVERIFY(verifySignalEmitted(spy, 0));
}

QTEST_MAIN(TestProduct)
#include "TestProduct.moc"
