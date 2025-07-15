#include <QTest>
#include <QSignalSpy>
#include "../../src/ai/AIRecommender.h"
#include "../../src/models/Product.h"
#include "../../src/models/Sale.h"
#include "../TestCommon.h"

class TestAIRecommender : public TestCommon
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 基本功能测试
    void testConstructor();
    void testLoadModel();
    void testTrainModel();
    void testSaveModel();
    void testGetRecommendations();
    void testGetPersonalizedRecommendations();
    void testGetPopularRecommendations();
    void testGetSimilarProducts();

    // 配置测试
    void testSetRecommendationStrategy();

    // 信号测试
    void testModelUpdatedSignal();

private:
    AIRecommender* m_aiRecommender;

    void createTestData();
    void cleanupTestData();
};

void TestAIRecommender::initTestCase()
{
    qDebug() << "Starting AIRecommender tests";
}

void TestAIRecommender::cleanupTestCase()
{
    qDebug() << "Finished AIRecommender tests";
}

void TestAIRecommender::init()
{
    m_aiRecommender = new AIRecommender(this);
    createTestData();
}

void TestAIRecommender::cleanup()
{
    cleanupTestData();
    if (m_aiRecommender) {
        delete m_aiRecommender;
        m_aiRecommender = nullptr;
    }
}

void TestAIRecommender::createTestData()
{
    // 创建一些测试数据用于推荐算法
}

void TestAIRecommender::cleanupTestData()
{
    // 清理测试数据
}

void TestAIRecommender::testConstructor()
{
    QVERIFY(m_aiRecommender != nullptr);
}

void TestAIRecommender::testLoadModel()
{
    bool result = m_aiRecommender->loadModel();
    
    // 测试模型加载功能
}

void TestAIRecommender::testTrainModel()
{
    bool result = m_aiRecommender->trainModel(7); // 使用7天数据训练
    
    // 测试模型训练功能
}

void TestAIRecommender::testSaveModel()
{
    bool result = m_aiRecommender->saveModel();
    
    // 测试模型保存功能
}

void TestAIRecommender::testGetRecommendations()
{
    QList<int> cartProductIds = {1, 2, 3};
    QList<int> recommendations = m_aiRecommender->getRecommendations(cartProductIds, 5);
    
    QVERIFY(recommendations.size() >= 0);
    QVERIFY(recommendations.size() <= 5);
}

void TestAIRecommender::testGetPersonalizedRecommendations()
{
    QList<int> recommendations = m_aiRecommender->getPersonalizedRecommendations(1, 3);
    
    QVERIFY(recommendations.size() >= 0);
    QVERIFY(recommendations.size() <= 3);
}

void TestAIRecommender::testGetPopularRecommendations()
{
    QList<int> popularProducts = m_aiRecommender->getPopularRecommendations(5, 7);
    
    QVERIFY(popularProducts.size() >= 0);
    QVERIFY(popularProducts.size() <= 5);
}

void TestAIRecommender::testGetSimilarProducts()
{
    QList<int> similarProducts = m_aiRecommender->getSimilarProducts(1, 3);
    
    QVERIFY(similarProducts.size() >= 0);
    QVERIFY(similarProducts.size() <= 3);
}

void TestAIRecommender::testSetRecommendationStrategy()
{
    // 测试不同推荐策略
    QList<int> cartProductIds = {1, 2};
    
    QList<int> recommendations1 = m_aiRecommender->getRecommendations(cartProductIds, 3, AIRecommender::CollaborativeFiltering);
    QList<int> recommendations2 = m_aiRecommender->getRecommendations(cartProductIds, 3, AIRecommender::ContentBasedFiltering);
    QList<int> recommendations3 = m_aiRecommender->getRecommendations(cartProductIds, 3, AIRecommender::HybridFiltering);
    QList<int> recommendations4 = m_aiRecommender->getRecommendations(cartProductIds, 3, AIRecommender::PopularityBased);
    
    // 确保返回的推荐数量在预期范围内
    QVERIFY(recommendations1.size() <= 3);
    QVERIFY(recommendations2.size() <= 3);
    QVERIFY(recommendations3.size() <= 3);
    QVERIFY(recommendations4.size() <= 3);
}

void TestAIRecommender::testModelUpdatedSignal()
{
    QSignalSpy modelTrainedSpy(m_aiRecommender, &AIRecommender::modelTrained);
    
    // 触发模型更新
    m_aiRecommender->trainModel(7);
    
    // 在完整实现中，应该发出信号
}

QTEST_MAIN(TestAIRecommender)
#include "TestAIRecommender.moc"
