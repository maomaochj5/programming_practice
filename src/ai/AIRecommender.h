#ifndef AIRECOMMENDER_H
#define AIRECOMMENDER_H

#include <QObject>
#include <QHash>
#include <QList>
#include <QVector>
#include <QTimer>
#include <QDateTime>
#include <memory>

// 前向声明
class DatabaseManager;

/**
 * @brief AIRecommender类 - AI商品推荐系统
 * 
 * 基于协同过滤和内容过滤的混合推荐算法
 */
class AIRecommender : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 推荐策略枚举
     */
    enum RecommendationStrategy {
        CollaborativeFiltering = 0,     ///< 协同过滤
        ContentBasedFiltering,          ///< 基于内容的过滤
        HybridFiltering,                ///< 混合过滤
        PopularityBased                 ///< 基于流行度
    };
    Q_ENUM(RecommendationStrategy)

    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    explicit AIRecommender(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~AIRecommender();

    /**
     * @brief 加载推荐模型
     * @param modelPath 模型文件路径
     * @return 如果加载成功返回true
     */
    bool loadModel(const QString& modelPath = "");

    /**
     * @brief 训练推荐模型
     * @param days 使用最近多少天的数据进行训练（默认30天）
     * @return 如果训练成功返回true
     */
    bool trainModel(int days = 30);

    /**
     * @brief 保存推荐模型
     * @param modelPath 模型文件路径
     * @return 如果保存成功返回true
     */
    bool saveModel(const QString& modelPath = "");

    /**
     * @brief 获取商品推荐
     * @param cartProductIds 当前购物车中的商品ID列表
     * @param topN 返回推荐数量（默认3个）
     * @param strategy 推荐策略（默认混合过滤）
     * @return 推荐商品ID列表
     */
    QList<int> getRecommendations(const QList<int>& cartProductIds, 
                                 int topN = 3, 
                                 RecommendationStrategy strategy = HybridFiltering);

    /**
     * @brief 获取用户个性化推荐
     * @param customerId 客户ID
     * @param topN 返回推荐数量
     * @return 推荐商品ID列表
     */
    QList<int> getPersonalizedRecommendations(int customerId, int topN = 5);

    /**
     * @brief 获取热门商品推荐
     * @param topN 返回推荐数量
     * @param days 统计天数（默认7天）
     * @return 热门商品ID列表
     */
    QList<int> getPopularRecommendations(int topN = 5, int days = 7);

    /**
     * @brief 获取相似商品推荐
     * @param productId 商品ID
     * @param topN 返回推荐数量
     * @return 相似商品ID列表
     */
    QList<int> getSimilarProducts(int productId, int topN = 3);

    /**
     * @brief 获取交叉销售推荐（购买了A的用户还购买了什么）
     * @param productId 商品ID
     * @param topN 返回推荐数量
     * @return 推荐商品ID列表
     */
    QList<int> getCrossSellRecommendations(int productId, int topN = 3);

    /**
     * @brief 处理冷启动问题 - 新用户推荐
     * @param demographicInfo 人口统计信息（如年龄段、地区等）
     * @param topN 返回推荐数量
     * @return 推荐商品ID列表
     */
    QList<int> getColdStartRecommendations(const QHash<QString, QString>& demographicInfo, 
                                          int topN = 5);

    /**
     * @brief 处理冷启动问题 - 新商品推荐
     * @param productId 新商品ID
     * @param topN 推荐给多少个用户
     * @return 目标用户ID列表
     */
    QList<int> getNewProductTargetUsers(int productId, int topN = 10);

    /**
     * @brief 设置推荐权重
     * @param collaborativeWeight 协同过滤权重
     * @param contentWeight 内容过滤权重
     * @param popularityWeight 流行度权重
     */
    void setRecommendationWeights(double collaborativeWeight, 
                                 double contentWeight, 
                                 double popularityWeight);

    /**
     * @brief 启用自动模型更新
     * @param intervalHours 更新间隔（小时）
     */
    void enableAutoModelUpdate(int intervalHours = 24);

    /**
     * @brief 禁用自动模型更新
     */
    void disableAutoModelUpdate();

    /**
     * @brief 获取推荐系统统计信息
     * @return 统计信息的键值对
     */
    QHash<QString, double> getRecommendationStats();

    /**
     * @brief 评估推荐质量
     * @param testDays 测试天数
     * @return 评估指标（如准确率、召回率等）
     */
    QHash<QString, double> evaluateRecommendations(int testDays = 7);

signals:
    /**
     * @brief 模型训练完成时发射的信号
     * @param success 是否成功
     * @param modelInfo 模型信息
     */
    void modelTrained(bool success, const QString& modelInfo);

    /**
     * @brief 推荐更新时发射的信号
     * @param productIds 推荐商品ID列表
     */
    void recommendationsUpdated(const QList<int>& productIds);

    /**
     * @brief 推荐系统错误时发射的信号
     * @param errorMessage 错误消息
     */
    void recommendationError(const QString& errorMessage);

private slots:
    /**
     * @brief 自动模型更新槽函数
     */
    void onAutoModelUpdate();

private:
    /**
     * @brief 初始化推荐系统
     */
    void initializeRecommender();

    /**
     * @brief 构建物品相似度矩阵（协同过滤）
     * @return 如果构建成功返回true
     */
    bool buildItemSimilarityMatrix();

    /**
     * @brief 构建用户-物品评分矩阵
     * @return 如果构建成功返回true
     */
    bool buildUserItemMatrix();

    /**
     * @brief 计算物品间的余弦相似度
     * @param item1Ratings 物品1的评分向量
     * @param item2Ratings 物品2的评分向量
     * @return 相似度值（0-1）
     */
    double calculateCosineSimilarity(const QVector<double>& item1Ratings, 
                                   const QVector<double>& item2Ratings);

    /**
     * @brief 基于内容的相似度计算
     * @param product1Id 商品1 ID
     * @param product2Id 商品2 ID
     * @return 相似度值（0-1）
     */
    double calculateContentSimilarity(int product1Id, int product2Id);

    /**
     * @brief 获取基于协同过滤的推荐
     * @param cartProductIds 购物车商品ID
     * @param topN 推荐数量
     * @return 推荐商品ID列表
     */
    QList<int> getCollaborativeRecommendations(const QList<int>& cartProductIds, int topN);

    /**
     * @brief 获取基于内容的推荐
     * @param cartProductIds 购物车商品ID
     * @param topN 推荐数量
     * @return 推荐商品ID列表
     */
    QList<int> getContentBasedRecommendations(const QList<int>& cartProductIds, int topN);

    /**
     * @brief 获取基于流行度的推荐
     * @param topN 推荐数量
     * @return 推荐商品ID列表
     */
    QList<int> getPopularityBasedRecommendations(int topN);

    /**
     * @brief 合并多种推荐结果
     * @param collaborativeRecs 协同过滤推荐
     * @param contentRecs 内容推荐
     * @param popularityRecs 流行度推荐
     * @param topN 最终推荐数量
     * @return 合并后的推荐列表
     */
    QList<int> mergeRecommendations(const QList<int>& collaborativeRecs,
                                   const QList<int>& contentRecs,
                                   const QList<int>& popularityRecs,
                                   int topN);

    /**
     * @brief 过滤已购买的商品
     * @param recommendations 原始推荐列表
     * @param cartProductIds 购物车商品ID
     * @return 过滤后的推荐列表
     */
    QList<int> filterPurchasedItems(const QList<int>& recommendations, 
                                   const QList<int>& cartProductIds);

    /**
     * @brief 记录推荐日志
     * @param operation 操作类型
     * @param details 详细信息
     */
    void logRecommendation(const QString& operation, const QString& details);

private:
    DatabaseManager* m_databaseManager;         ///< 数据库管理器
    
    // 推荐模型数据
    QHash<int, QHash<int, double>> m_itemSimilarityMatrix;  ///< 物品相似度矩阵
    QHash<int, QVector<double>> m_userItemMatrix;           ///< 用户-物品评分矩阵
    QHash<int, double> m_popularityScores;                  ///< 商品流行度分数
    QHash<int, QHash<QString, QString>> m_productFeatures;  ///< 商品特征
    
    // 推荐权重
    double m_collaborativeWeight;               ///< 协同过滤权重
    double m_contentWeight;                     ///< 内容过滤权重
    double m_popularityWeight;                  ///< 流行度权重
    
    // 系统参数
    QTimer* m_updateTimer;                      ///< 自动更新定时器
    QString m_modelPath;                        ///< 模型文件路径
    bool m_modelLoaded;                         ///< 模型是否已加载
    QDateTime m_lastTrainingTime;               ///< 上次训练时间
    
    // 性能统计
    int m_totalRecommendations;                 ///< 总推荐次数
    int m_successfulRecommendations;            ///< 成功推荐次数
    double m_averageResponseTime;               ///< 平均响应时间
};

#endif // AIRECOMMENDER_H
