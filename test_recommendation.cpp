#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QList>
#include <memory>

#include "src/controllers/RecommendationController.h"
#include "src/controllers/ProductManager.h"
#include "src/models/Product.h"

class TestApp : public QObject
{
    Q_OBJECT

public:
    TestApp(QObject *parent = nullptr) : QObject(parent)
    {
        // 初始化组件
        m_productManager = std::make_unique<ProductManager>(this);
        m_recommendationController = std::make_unique<RecommendationController>(m_productManager.get(), this);
        
        // 连接信号
        connect(m_recommendationController.get(), &RecommendationController::recommendationsReady,
                this, &TestApp::onRecommendationsReady);
        connect(m_recommendationController.get(), &RecommendationController::recommendationError,
                this, &TestApp::onRecommendationError);
        
        qDebug() << "=== 推荐功能调试测试开始 ===";
        
        // 创建测试商品数据
        createTestProducts();
        
        // 延迟开始测试，确保所有组件初始化完成
        QTimer::singleShot(1000, this, &TestApp::startTests);
    }

private:
    void createTestProducts()
    {
        qDebug() << "创建测试商品数据...";
        
        // 创建一些测试商品
        QList<Product*> testProducts;
        
        testProducts.append(new Product(1, "1234567890", "苹果", "新鲜红苹果", 5.99, 100, "水果", this));
        testProducts.append(new Product(2, "1234567891", "香蕉", "香甜香蕉", 3.99, 80, "水果", this));
        testProducts.append(new Product(3, "1234567892", "牛奶", "纯牛奶1L", 8.99, 50, "饮品", this));
        testProducts.append(new Product(4, "1234567893", "面包", "全麦面包", 12.99, 30, "主食", this));
        testProducts.append(new Product(5, "1234567894", "酸奶", "草莓酸奶", 6.99, 40, "饮品", this));
        testProducts.append(new Product(6, "1234567895", "鸡蛋", "新鲜鸡蛋12个装", 15.99, 25, "蛋类", this));
        testProducts.append(new Product(7, "1234567896", "橙汁", "100%纯橙汁", 9.99, 60, "饮品", this));
        testProducts.append(new Product(8, "1234567897", "薯片", "原味薯片", 4.99, 70, "零食", this));
        testProducts.append(new Product(9, "1234567898", "巧克力", "黑巧克力", 19.99, 35, "零食", this));
        testProducts.append(new Product(10, "1234567899", "矿泉水", "天然矿泉水500ml", 2.99, 200, "饮品", this));
        
        qDebug() << "创建了" << testProducts.size() << "个测试商品";
        
        // 模拟ProductManager接收到商品数据
        // 这里我们直接调用onProductsRead来模拟数据库返回的商品
        QMetaObject::invokeMethod(m_productManager.get(), "onProductsRead", 
                                 Qt::QueuedConnection,
                                 Q_ARG(QList<Product*>, testProducts));
        
        qDebug() << "测试商品数据已加载到ProductManager";
    }

private slots:
    void startTests()
    {
        qDebug() << "\n=== 测试1: 基于购物车的推荐 ===";
        
        // 模拟购物车中的商品ID
        QList<int> cartItems = {1, 3, 5, 7};
        qDebug() << "模拟购物车商品ID:" << cartItems;
        
        m_recommendationController->generateRecommendationForCart(cartItems);
        
        // 5秒后测试用户查询
        QTimer::singleShot(5000, this, &TestApp::testUserQuery);
    }
    
    void testUserQuery()
    {
        qDebug() << "\n=== 测试2: 基于用户查询的推荐 ===";
        
        QString userQuery = "我想买一些健康的零食";
        qDebug() << "用户查询内容:" << userQuery;
        
        m_recommendationController->generateRecommendationForQuery(userQuery);
        
        // 5秒后退出
        QTimer::singleShot(5000, this, &TestApp::finishTests);
    }
    
    void finishTests()
    {
        qDebug() << "\n=== 推荐功能调试测试完成 ===";
        QCoreApplication::quit();
    }
    
    void onRecommendationsReady(const QList<Product*>& products)
    {
        qDebug() << "\n*** 收到推荐结果回调 ***";
        qDebug() << "推荐商品数量:" << products.size();
        
        if (products.isEmpty()) {
            qDebug() << "推荐列表为空";
            return;
        }
        
        qDebug() << "推荐商品详情:";
        for (int i = 0; i < products.size(); ++i) {
            const Product* product = products.at(i);
            if (product) {
                qDebug() << QString("  [%1] %2 (ID:%3, 价格:¥%4)")
                            .arg(i + 1)
                            .arg(product->getName())
                            .arg(product->getProductId())
                            .arg(product->getPrice(), 0, 'f', 2);
            } else {
                qDebug() << QString("  [%1] 商品指针为空").arg(i + 1);
            }
        }
        
        qDebug() << "*** 推荐结果处理完成 ***\n";
    }
    
    void onRecommendationError(const QString& errorMessage)
    {
        qDebug() << "*** 推荐错误 ***";
        qDebug() << "错误信息:" << errorMessage;
        qDebug() << "*** 错误处理完成 ***\n";
    }

private:
    std::unique_ptr<ProductManager> m_productManager;
    std::unique_ptr<RecommendationController> m_recommendationController;
};

#include "test_recommendation.moc"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "推荐功能调试程序启动";
    qDebug() << "Qt版本:" << QT_VERSION_STR;
    
    TestApp testApp;
    
    return app.exec();
} 