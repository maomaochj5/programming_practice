#ifndef RECOMMENDATIONCONTROLLER_H
#define RECOMMENDATIONCONTROLLER_H

#include <QObject>
#include <QList>
#include <QString>
#include <QHash>

// Forward declarations
class ProductManager;
class Product;
class QNetworkAccessManager;
class QNetworkReply;

/**
 * @brief RecommendationController类 - 基于微调模型的商品推荐控制器
 * 
 * 处理与微调语言模型的交互，生成智能商品推荐
 */
class RecommendationController : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param productManager 商品管理器指针，用于获取商品详情
     * @param parent 父对象指针
     */
    explicit RecommendationController(ProductManager* productManager, QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~RecommendationController();

    /**
     * @brief 设置AI服务器URL
     * @param url AI服务器地址
     */
    void setAiServerUrl(const QString& url);

    /**
     * @brief 设置是否使用AI模型
     * @param useAi 是否使用AI模型（false则使用占位符逻辑）
     */
    void setUseAiModel(bool useAi);

public slots:
    /**
     * @brief 基于购物车内容生成推荐
     * @param cartProductIds 当前购物车中的商品ID列表
     */
    void generateRecommendationForCart(const QList<int>& cartProductIds);

    /**
     * @brief 基于用户查询生成推荐
     * @param userQuery 用户的原始文本查询
     */
    void generateRecommendationForQuery(const QString& userQuery);

signals:
    /**
     * @brief 推荐结果准备完成时发射的信号
     * @param recommendedProducts 推荐的商品列表
     */
    void recommendationsReady(const QList<Product*>& recommendedProducts);

    /**
     * @brief 推荐过程中发生错误时发射的信号
     * @param errorMessage 错误消息
     */
    void recommendationError(const QString& errorMessage);

private:
    /**
     * @brief 调用微调模型进行推荐（占位符实现）
     * @param prompt 发送给模型的提示文本
     * @return 模型返回的推荐商品ID列表
     */
    QList<int> callFineTunedModel(const QString& prompt);

    /**
     * @brief 构建基于购物车的查询提示
     * @param cartProductIds 购物车商品ID列表
     * @return 格式化的查询提示
     */
    QString buildCartPrompt(const QList<int>& cartProductIds);

    /**
     * @brief 构建基于用户查询的提示
     * @param userQuery 用户查询
     * @return 格式化的查询提示
     */
    QString buildUserQueryPrompt(const QString& userQuery);

    /**
     * @brief 将商品ID列表转换为商品对象列表
     * @param productIds 商品ID列表
     * @return 商品对象列表
     */
    QList<Product*> convertIdsToProducts(const QList<int>& productIds);

private slots:
    /**
     * @brief 处理AI推荐API响应
     * @param reply 网络响应
     */
    void onAiRecommendationFinished(QNetworkReply* reply);

private:
    /**
     * @brief 发送HTTP请求到AI服务器
     * @param endpoint API端点
     * @param jsonData 请求数据
     * @param requestType 请求类型标识
     */
    void sendAiRequest(const QString& endpoint, const QJsonObject& jsonData, const QString& requestType);

    /**
     * @brief 解析AI服务器响应
     * @param jsonResponse AI服务器JSON响应
     * @return 推荐商品列表
     */
    QList<Product*> parseAiResponse(const QJsonObject& jsonResponse);

    /**
     * @brief 创建测试商品对象
     * @param productId 商品ID
     * @param name 商品名称
     * @param category 商品类别
     * @param price 商品价格
     * @param description 商品描述
     * @return 商品对象指针
     */
    Product* createTestProduct(const QString& productId, const QString& name, 
                              const QString& category, double price, const QString& description);

private:
    ProductManager* m_productManager;           ///< 商品管理器指针
    QNetworkAccessManager* m_networkManager;   ///< 网络管理器
    QString m_aiServerUrl;                     ///< AI服务器地址
    bool m_useAiModel;                         ///< 是否使用AI模型
    QHash<QNetworkReply*, QString> m_pendingRequests; ///< 待处理的请求映射
};

#endif // RECOMMENDATIONCONTROLLER_H 