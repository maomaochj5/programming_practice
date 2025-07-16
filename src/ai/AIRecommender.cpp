#include "AIRecommender.h"
#include "../database/DatabaseManager.h"
#include <QDebug>
#include <QDateTime>
#include <QElapsedTimer>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRandomGenerator>
#include <QtMath>
#include <algorithm>

AIRecommender::AIRecommender(QObject *parent)
    : QObject(parent)
    , m_databaseManager(&DatabaseManager::getInstance())
    , m_collaborativeWeight(0.5)
    , m_contentWeight(0.3)
    , m_popularityWeight(0.2)
    , m_updateTimer(new QTimer(this))
    , m_modelLoaded(false)
    , m_totalRecommendations(0)
    , m_successfulRecommendations(0)
    , m_averageResponseTime(0.0)
{
    qDebug() << "AIRecommender 构造函数开始";
    
    // 设置模型文件路径
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    m_modelPath = dataDir + "/recommendation_model.json";
    qDebug() << "AIRecommender 模型路径:" << m_modelPath;
    
    // 连接自动更新定时器
    connect(m_updateTimer, &QTimer::timeout, this, &AIRecommender::onAutoModelUpdate);
    
    // 初始化推荐系统
    qDebug() << "AIRecommender 开始初始化推荐系统";
    initializeRecommender();
    
    qDebug() << "AI推荐系统初始化完成";
}

AIRecommender::~AIRecommender()
{
    // 保存模型
    saveModel();
    qDebug() << "AI推荐系统析构";
}

bool AIRecommender::loadModel(const QString& modelPath)
{
    qDebug() << "AIRecommender::loadModel 开始";
    
    QString path = modelPath.isEmpty() ? m_modelPath : modelPath;
    qDebug() << "AIRecommender::loadModel 尝试加载模型:" << path;
    
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "无法打开模型文件:" << path;
        return false;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "模型文件解析错误:" << error.errorString();
        emit recommendationError(QString("模型文件解析错误: %1").arg(error.errorString()));
        return false;
    }
    
    QJsonObject root = doc.object();
    
    // 加载物品相似度矩阵
    QJsonObject similarityMatrix = root["itemSimilarityMatrix"].toObject();
    m_itemSimilarityMatrix.clear();
    
    for (auto it = similarityMatrix.begin(); it != similarityMatrix.end(); ++it) {
        int itemId = it.key().toInt();
        QJsonObject similarities = it.value().toObject();
        
        QHash<int, double> itemSimilarities;
        for (auto simIt = similarities.begin(); simIt != similarities.end(); ++simIt) {
            itemSimilarities[simIt.key().toInt()] = simIt.value().toDouble();
        }
        m_itemSimilarityMatrix[itemId] = itemSimilarities;
    }
    
    // 加载流行度分数
    QJsonObject popularityScores = root["popularityScores"].toObject();
    m_popularityScores.clear();
    
    for (auto it = popularityScores.begin(); it != popularityScores.end(); ++it) {
        m_popularityScores[it.key().toInt()] = it.value().toDouble();
    }
    
    // 加载元数据
    QJsonObject metadata = root["metadata"].toObject();
    m_lastTrainingTime = QDateTime::fromString(metadata["lastTrainingTime"].toString(), Qt::ISODate);
    m_totalRecommendations = metadata["totalRecommendations"].toInt();
    m_successfulRecommendations = metadata["successfulRecommendations"].toInt();
    
    m_modelLoaded = true;
    qDebug() << "推荐模型加载成功，训练时间:" << m_lastTrainingTime.toString();
    
    return true;
}

bool AIRecommender::trainModel(int days)
{
    QElapsedTimer timer;
    timer.start();
    
    qDebug() << "开始训练推荐模型，使用最近" << days << "天的数据";
    
    // 构建用户-物品矩阵
    if (!buildUserItemMatrix()) {
        emit recommendationError("构建用户-物品矩阵失败");
        return false;
    }
    
    // 构建物品相似度矩阵
    if (!buildItemSimilarityMatrix()) {
        emit recommendationError("构建物品相似度矩阵失败");
        return false;
    }
    
    // 计算流行度分数
    auto popularProducts = m_databaseManager->getPopularProducts(100, days);
    m_popularityScores.clear();
    
    for (int i = 0; i < popularProducts.size(); ++i) {
        int productId = popularProducts[i];
        // 流行度分数递减
        double score = 1.0 - (double)i / popularProducts.size();
        m_popularityScores[productId] = score;
    }
    
    m_lastTrainingTime = QDateTime::currentDateTime();
    m_modelLoaded = true;
    
    qint64 elapsed = timer.elapsed();
    QString modelInfo = QString("训练完成，耗时: %1ms，物品数: %2，相似度对数: %3")
                       .arg(elapsed)
                       .arg(m_itemSimilarityMatrix.size())
                       .arg(m_itemSimilarityMatrix.size() * (m_itemSimilarityMatrix.size() - 1) / 2);
    
    emit modelTrained(true, modelInfo);
    logRecommendation("模型训练", modelInfo);
    
    return true;
}

bool AIRecommender::saveModel(const QString& modelPath)
{
    QString path = modelPath.isEmpty() ? m_modelPath : modelPath;
    
    QJsonObject root;
    
    // 保存物品相似度矩阵
    QJsonObject similarityMatrix;
    for (auto it = m_itemSimilarityMatrix.begin(); it != m_itemSimilarityMatrix.end(); ++it) {
        QJsonObject similarities;
        const auto& itemSimilarities = it.value();
        
        for (auto simIt = itemSimilarities.begin(); simIt != itemSimilarities.end(); ++simIt) {
            similarities[QString::number(simIt.key())] = simIt.value();
        }
        similarityMatrix[QString::number(it.key())] = similarities;
    }
    root["itemSimilarityMatrix"] = similarityMatrix;
    
    // 保存流行度分数
    QJsonObject popularityScores;
    for (auto it = m_popularityScores.begin(); it != m_popularityScores.end(); ++it) {
        popularityScores[QString::number(it.key())] = it.value();
    }
    root["popularityScores"] = popularityScores;
    
    // 保存元数据
    QJsonObject metadata;
    metadata["lastTrainingTime"] = m_lastTrainingTime.toString(Qt::ISODate);
    metadata["totalRecommendations"] = m_totalRecommendations;
    metadata["successfulRecommendations"] = m_successfulRecommendations;
    metadata["version"] = "1.0";
    root["metadata"] = metadata;
    
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        emit recommendationError(QString("无法保存模型文件: %1").arg(path));
        return false;
    }
    
    QJsonDocument doc(root);
    file.write(doc.toJson());
    
    qDebug() << "推荐模型保存成功:" << path;
    return true;
}

QList<int> AIRecommender::getRecommendations(const QList<int>& cartProductIds, 
                                            int topN, 
                                            RecommendationStrategy strategy)
{
    QElapsedTimer timer;
    timer.start();
    
    if (!m_modelLoaded) {
        // 尝试加载模型，如果失败则训练新模型
        if (!loadModel()) {
            trainModel();
        }
    }
    
    QList<int> recommendations;
    
    switch (strategy) {
        case CollaborativeFiltering:
            recommendations = getCollaborativeRecommendations(cartProductIds, topN);
            break;
        case ContentBasedFiltering:
            recommendations = getContentBasedRecommendations(cartProductIds, topN);
            break;
        case PopularityBased:
            recommendations = getPopularityBasedRecommendations(topN);
            break;
        case HybridFiltering:
        default:
            {
                auto collaborativeRecs = getCollaborativeRecommendations(cartProductIds, topN * 2);
                auto contentRecs = getContentBasedRecommendations(cartProductIds, topN * 2);
                auto popularityRecs = getPopularityBasedRecommendations(topN);
                
                recommendations = mergeRecommendations(collaborativeRecs, contentRecs, popularityRecs, topN);
            }
            break;
    }
    
    // 过滤已购买的商品
    recommendations = filterPurchasedItems(recommendations, cartProductIds);
    
    // 限制返回数量
    if (recommendations.size() > topN) {
        recommendations = recommendations.mid(0, topN);
    }
    
    m_totalRecommendations++;
    if (!recommendations.isEmpty()) {
        m_successfulRecommendations++;
    }
    
    qint64 elapsed = timer.elapsed();
    m_averageResponseTime = (m_averageResponseTime * (m_totalRecommendations - 1) + elapsed) / m_totalRecommendations;
    
    logRecommendation("获取推荐", 
                     QString("策略: %1, 输入: %2个商品, 输出: %3个推荐, 耗时: %4ms")
                     .arg(strategy)
                     .arg(cartProductIds.size())
                     .arg(recommendations.size())
                     .arg(elapsed));
    
    emit recommendationsUpdated(recommendations);
    return recommendations;
}

QList<int> AIRecommender::getPopularRecommendations(int topN, int days)
{
    auto popularProducts = m_databaseManager->getPopularProducts(topN, days);
    
    logRecommendation("热门推荐", QString("返回 %1 个热门商品").arg(popularProducts.size()));
    
    return popularProducts;
}

QList<int> AIRecommender::getSimilarProducts(int productId, int topN)
{
    QList<int> similarProducts;
    
    if (!m_itemSimilarityMatrix.contains(productId)) {
        return getPopularRecommendations(topN);
    }
    
    const auto& similarities = m_itemSimilarityMatrix[productId];
    
    // 按相似度排序
    QList<QPair<double, int>> sortedSimilarities;
    for (auto it = similarities.begin(); it != similarities.end(); ++it) {
        sortedSimilarities.append({it.value(), it.key()});
    }
    
    std::sort(sortedSimilarities.begin(), sortedSimilarities.end(), 
              [](const QPair<double, int>& a, const QPair<double, int>& b) {
                  return a.first > b.first;
              });
    
    for (int i = 0; i < qMin(topN, sortedSimilarities.size()); ++i) {
        similarProducts.append(sortedSimilarities[i].second);
    }
    
    return similarProducts;
}

void AIRecommender::setRecommendationWeights(double collaborativeWeight, 
                                            double contentWeight, 
                                            double popularityWeight)
{
    // 归一化权重
    double total = collaborativeWeight + contentWeight + popularityWeight;
    if (total > 0) {
        m_collaborativeWeight = collaborativeWeight / total;
        m_contentWeight = contentWeight / total;
        m_popularityWeight = popularityWeight / total;
        
        qDebug() << "更新推荐权重 - 协同过滤:" << m_collaborativeWeight 
                 << "内容过滤:" << m_contentWeight 
                 << "流行度:" << m_popularityWeight;
    }
}

void AIRecommender::enableAutoModelUpdate(int intervalHours)
{
    m_updateTimer->start(intervalHours * 60 * 60 * 1000); // 转换为毫秒
    qDebug() << "启用自动模型更新，间隔:" << intervalHours << "小时";
}

void AIRecommender::disableAutoModelUpdate()
{
    m_updateTimer->stop();
    qDebug() << "禁用自动模型更新";
}

QHash<QString, double> AIRecommender::getRecommendationStats()
{
    QHash<QString, double> stats;
    
    stats["totalRecommendations"] = m_totalRecommendations;
    stats["successfulRecommendations"] = m_successfulRecommendations;
    stats["successRate"] = m_totalRecommendations > 0 ? 
                          (double)m_successfulRecommendations / m_totalRecommendations : 0.0;
    stats["averageResponseTime"] = m_averageResponseTime;
    stats["modelAge"] = m_lastTrainingTime.secsTo(QDateTime::currentDateTime()) / 3600.0; // 小时
    stats["itemCount"] = m_itemSimilarityMatrix.size();
    
    return stats;
}

void AIRecommender::onAutoModelUpdate()
{
    qDebug() << "执行自动模型更新";
    trainModel();
}

void AIRecommender::initializeRecommender()
{
    qDebug() << "AIRecommender::initializeRecommender 开始";
    
    // 尝试加载现有模型
    try {
    if (!loadModel()) {
        qDebug() << "没有找到现有模型，将在首次使用时训练";
    }
    } catch (const std::exception& e) {
        qWarning() << "AIRecommender 初始化异常:" << e.what();
    } catch (...) {
        qWarning() << "AIRecommender 初始化发生未知异常";
    }
    
    qDebug() << "AIRecommender::initializeRecommender 完成";
}

bool AIRecommender::buildItemSimilarityMatrix()
{
    // 获取商品销售统计数据
    auto salesStats = m_databaseManager->getProductSalesStats(30);
    
    if (salesStats.isEmpty()) {
        qWarning() << "没有销售数据，无法构建相似度矩阵";
        return false;
    }
    
    QList<int> productIds = salesStats.keys();
    m_itemSimilarityMatrix.clear();
    
    // 这是一个简化的相似度计算
    // 在实际项目中，应该基于真实的用户-物品交互数据
    for (int i = 0; i < productIds.size(); ++i) {
        int productId1 = productIds[i];
        QHash<int, double> similarities;
        
        for (int j = i + 1; j < productIds.size(); ++j) {
            int productId2 = productIds[j];
            
            // 计算相似度（这里使用销量相关性作为简化的相似度）
            double sales1 = salesStats[productId1];
            double sales2 = salesStats[productId2];
            
            // 简化的相似度计算：基于销量差异
            double maxSales = qMax(sales1, sales2);
            double minSales = qMin(sales1, sales2);
            double similarity = maxSales > 0 ? minSales / maxSales : 0.0;
            
            // 加入一些随机性以模拟真实的协同过滤
            similarity *= (0.8 + 0.4 * (QRandomGenerator::global()->generateDouble()));
            
            if (similarity > 0.1) { // 只保存相似度较高的
                similarities[productId2] = similarity;
                
                // 确保对称性
                if (!m_itemSimilarityMatrix.contains(productId2)) {
                    m_itemSimilarityMatrix[productId2] = QHash<int, double>();
                }
                m_itemSimilarityMatrix[productId2][productId1] = similarity;
            }
        }
        
        if (!similarities.isEmpty()) {
            m_itemSimilarityMatrix[productId1] = similarities;
        }
    }
    
    qDebug() << "构建物品相似度矩阵完成，商品数:" << m_itemSimilarityMatrix.size();
    return true;
}

QList<int> AIRecommender::getCollaborativeRecommendations(const QList<int>& cartProductIds, int topN)
{
    QHash<int, double> candidateScores;
    
    // 对购物车中的每个商品，找出相似商品
    for (int productId : cartProductIds) {
        if (m_itemSimilarityMatrix.contains(productId)) {
            const auto& similarities = m_itemSimilarityMatrix[productId];
            
            for (auto it = similarities.begin(); it != similarities.end(); ++it) {
                int candidateId = it.key();
                double similarity = it.value();
                
                // 累加相似度分数
                candidateScores[candidateId] += similarity;
            }
        }
    }
    
    // 按分数排序
    QList<QPair<double, int>> sortedCandidates;
    for (auto it = candidateScores.begin(); it != candidateScores.end(); ++it) {
        sortedCandidates.append({it.value(), it.key()});
    }
    
    std::sort(sortedCandidates.begin(), sortedCandidates.end(),
              [](const QPair<double, int>& a, const QPair<double, int>& b) {
                  return a.first > b.first;
              });
    
    QList<int> recommendations;
    for (int i = 0; i < qMin(topN, sortedCandidates.size()); ++i) {
        recommendations.append(sortedCandidates[i].second);
    }
    
    return recommendations;
}

QList<int> AIRecommender::getContentBasedRecommendations(const QList<int>& cartProductIds, int topN)
{
    // 这是一个简化的基于内容的推荐
    // 在实际项目中，应该基于商品的分类、品牌、属性等特征
    
    Q_UNUSED(cartProductIds)
    
    // 暂时返回热门商品作为基于内容的推荐
    return getPopularityBasedRecommendations(topN);
}

QList<int> AIRecommender::getPopularityBasedRecommendations(int topN)
{
    QList<QPair<double, int>> sortedPopularity;
    
    for (auto it = m_popularityScores.begin(); it != m_popularityScores.end(); ++it) {
        sortedPopularity.append({it.value(), it.key()});
    }
    
    std::sort(sortedPopularity.begin(), sortedPopularity.end(),
              [](const QPair<double, int>& a, const QPair<double, int>& b) {
                  return a.first > b.first;
              });
    
    QList<int> recommendations;
    for (int i = 0; i < qMin(topN, sortedPopularity.size()); ++i) {
        recommendations.append(sortedPopularity[i].second);
    }
    
    return recommendations;
}

QList<int> AIRecommender::mergeRecommendations(const QList<int>& collaborativeRecs,
                                              const QList<int>& contentRecs,
                                              const QList<int>& popularityRecs,
                                              int topN)
{
    QHash<int, double> combinedScores;
    
    // 合并协同过滤推荐
    for (int i = 0; i < collaborativeRecs.size(); ++i) {
        int productId = collaborativeRecs[i];
        double score = (collaborativeRecs.size() - i) / (double)collaborativeRecs.size();
        combinedScores[productId] += score * m_collaborativeWeight;
    }
    
    // 合并内容推荐
    for (int i = 0; i < contentRecs.size(); ++i) {
        int productId = contentRecs[i];
        double score = (contentRecs.size() - i) / (double)contentRecs.size();
        combinedScores[productId] += score * m_contentWeight;
    }
    
    // 合并流行度推荐
    for (int i = 0; i < popularityRecs.size(); ++i) {
        int productId = popularityRecs[i];
        double score = (popularityRecs.size() - i) / (double)popularityRecs.size();
        combinedScores[productId] += score * m_popularityWeight;
    }
    
    // 排序并返回
    QList<QPair<double, int>> sortedCombined;
    for (auto it = combinedScores.begin(); it != combinedScores.end(); ++it) {
        sortedCombined.append({it.value(), it.key()});
    }
    
    std::sort(sortedCombined.begin(), sortedCombined.end(),
              [](const QPair<double, int>& a, const QPair<double, int>& b) {
                  return a.first > b.first;
              });
    
    QList<int> recommendations;
    for (int i = 0; i < qMin(topN, sortedCombined.size()); ++i) {
        recommendations.append(sortedCombined[i].second);
    }
    
    return recommendations;
}

QList<int> AIRecommender::filterPurchasedItems(const QList<int>& recommendations, 
                                              const QList<int>& cartProductIds)
{
    QList<int> filtered;
    
    for (int productId : recommendations) {
        if (!cartProductIds.contains(productId)) {
            filtered.append(productId);
        }
    }
    
    return filtered;
}

void AIRecommender::logRecommendation(const QString& operation, const QString& details)
{
    QString logMessage = QString("[%1] AI推荐 - %2: %3")
                        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                        .arg(operation)
                        .arg(details);
    qDebug() << logMessage;
}

// 其他方法的占位符实现
bool AIRecommender::buildUserItemMatrix() { return true; }
QList<int> AIRecommender::getPersonalizedRecommendations(int customerId, int topN) { Q_UNUSED(customerId) return getPopularRecommendations(topN); }
QList<int> AIRecommender::getCrossSellRecommendations(int productId, int topN) { return getSimilarProducts(productId, topN); }
QList<int> AIRecommender::getColdStartRecommendations(const QHash<QString, QString>& demographicInfo, int topN) { Q_UNUSED(demographicInfo) return getPopularRecommendations(topN); }
QList<int> AIRecommender::getNewProductTargetUsers(int productId, int topN) { Q_UNUSED(productId) Q_UNUSED(topN) return {}; }
QHash<QString, double> AIRecommender::evaluateRecommendations(int testDays) { Q_UNUSED(testDays) return {}; }
double AIRecommender::calculateCosineSimilarity(const QVector<double>& item1Ratings, const QVector<double>& item2Ratings) { Q_UNUSED(item1Ratings) Q_UNUSED(item2Ratings) return 0.0; }
double AIRecommender::calculateContentSimilarity(int product1Id, int product2Id) { Q_UNUSED(product1Id) Q_UNUSED(product2Id) return 0.0; }
