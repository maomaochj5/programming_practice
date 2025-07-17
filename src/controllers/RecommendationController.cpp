#include "RecommendationController.h"
#include "ProductManager.h"
#include "../models/Product.h"
#include <QDebug>
#include <QRandomGenerator>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QRegularExpression>

RecommendationController::RecommendationController(ProductManager* productManager, QObject *parent)
    : QObject(parent)
    , m_productManager(productManager)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_aiServerUrl("http://127.0.0.1:5001")
    , m_useAiModel(true)  // 默认使用AI模型
{
    qDebug() << "RecommendationController 初始化完成";
    
    if (!m_productManager) {
        qWarning() << "RecommendationController: ProductManager 为空指针";
    }
    
    // 设置网络超时
    m_networkManager->setTransferTimeout(30000); // 30秒超时
    
    qDebug() << "AI服务器地址:" << m_aiServerUrl;
    qDebug() << "是否使用AI模型:" << m_useAiModel;
}

RecommendationController::~RecommendationController()
{
    qDebug() << "RecommendationController 析构";
}

void RecommendationController::generateRecommendationForCart(const QList<int>& cartProductIds)
{
    qDebug() << "RecommendationController::generateRecommendationForCart 开始";
    qDebug() << "购物车商品ID列表:" << cartProductIds;
    
    if (!m_productManager) {
        emit recommendationError("商品管理器未初始化");
        return;
    }
    
    if (cartProductIds.isEmpty()) {
        qDebug() << "购物车为空，返回空推荐列表";
        emit recommendationsReady(QList<Product*>());
        return;
    }
    
    if (m_useAiModel) {
        // 使用AI模型进行推荐
        qDebug() << "使用AI模型进行购物车推荐";
        
        // 构建请求数据
        QJsonArray cartArray;
        for (int productId : cartProductIds) {
            Product* product = m_productManager->getProductById(productId);
            if (product) {
                QJsonObject productObj;
                productObj["id"] = productId;
                productObj["name"] = product->getName();
                productObj["category"] = product->getCategory();
                productObj["price"] = product->getPrice();
                cartArray.append(productObj);
            }
        }
        
        QJsonObject requestData;
        requestData["cart_products"] = cartArray;
        
        qDebug() << "发送购物车推荐请求到AI服务器:" << QJsonDocument(requestData).toJson(QJsonDocument::Compact);
        
        // 发送API请求
        sendAiRequest("/api/recommend/cart", requestData, "cart");
        
    } else {
        // 使用占位符逻辑
        qDebug() << "使用占位符逻辑进行购物车推荐";
        
        // 构建查询提示
        QString prompt = buildCartPrompt(cartProductIds);
        qDebug() << "构建的查询提示:" << prompt;
        
        // 调用微调模型（占位符实现）
        QList<int> recommendedIds = callFineTunedModel(prompt);
        qDebug() << "模型返回的推荐商品ID列表 (购物车推荐):" << recommendedIds;
        
        // 转换为商品对象
        QList<Product*> recommendedProducts = convertIdsToProducts(recommendedIds);
        qDebug() << "成功转换为商品对象的数量 (购物车推荐):" << recommendedProducts.size();
        
        // 使用定时器异步发射信号，模拟网络请求延迟
        QTimer::singleShot(500, [this, recommendedProducts]() {
            qDebug() << "RecommendationController::generateRecommendationForCart 即将发射 recommendationsReady 信号";
            qDebug() << "推荐商品详细列表:";
            for (const Product* product : recommendedProducts) {
                if (product) {
                    qDebug() << "  - ID:" << product->getProductId() 
                             << "名称:" << product->getName() 
                             << "价格:" << product->getPrice() 
                             << "类别:" << product->getCategory();
                }
            }
            emit recommendationsReady(recommendedProducts);
            qDebug() << "RecommendationController::generateRecommendationForCart recommendationsReady 信号已发射";
        });
    }
}

void RecommendationController::generateRecommendationForQuery(const QString& userQuery)
{
    qDebug() << "RecommendationController::generateRecommendationForQuery 开始";
    qDebug() << "用户查询:" << userQuery;
    
    if (!m_productManager) {
        emit recommendationError("商品管理器未初始化");
        return;
    }
    
    if (userQuery.trimmed().isEmpty()) {
        qDebug() << "用户查询为空，返回空推荐列表";
        emit recommendationsReady(QList<Product*>());
        return;
    }
    
    if (m_useAiModel) {
        // 使用AI模型进行推荐
        qDebug() << "使用AI模型进行查询推荐";
        
        QJsonObject requestData;
        requestData["query"] = userQuery;
        
        qDebug() << "发送查询推荐请求到AI服务器:" << QJsonDocument(requestData).toJson(QJsonDocument::Compact);
        
        // 发送API请求
        sendAiRequest("/api/recommend/query", requestData, "query");
        
    } else {
        // 使用占位符逻辑
        qDebug() << "使用占位符逻辑进行查询推荐";
        
        // 构建查询提示
        QString prompt = buildUserQueryPrompt(userQuery);
        qDebug() << "构建的查询提示:" << prompt;
        
        // 调用微调模型（占位符实现）
        QList<int> recommendedIds = callFineTunedModel(prompt);
        qDebug() << "模型返回的推荐商品ID列表 (查询推荐):" << recommendedIds;
        
        // 转换为商品对象
        QList<Product*> recommendedProducts = convertIdsToProducts(recommendedIds);
        qDebug() << "成功转换为商品对象的数量 (查询推荐):" << recommendedProducts.size();
        
        // 使用定时器异步发射信号，模拟网络请求延迟  
        QTimer::singleShot(800, [this, recommendedProducts]() {
            qDebug() << "RecommendationController::generateRecommendationForQuery 即将发射 recommendationsReady 信号";
            qDebug() << "推荐商品详细列表:";
            for (const Product* product : recommendedProducts) {
                if (product) {
                    qDebug() << "  - ID:" << product->getProductId() 
                             << "名称:" << product->getName() 
                             << "价格:" << product->getPrice() 
                             << "类别:" << product->getCategory();
                }
            }
            emit recommendationsReady(recommendedProducts);
            qDebug() << "RecommendationController::generateRecommendationForQuery recommendationsReady 信号已发射";
        });
    }
}

QList<int> RecommendationController::callFineTunedModel(const QString& prompt)
{
    qDebug() << "RecommendationController::callFineTunedModel 开始 (占位符实现)";
    qDebug() << "发送到模型的提示:" << prompt;
    
    // 占位符实现：生成一些随机的商品ID作为推荐结果
    QList<int> mockRecommendations;
    
    // 生成3-5个随机商品ID（使用更小的范围确保存在）
    int recommendationCount = QRandomGenerator::global()->bounded(3, 6); // 3到5个推荐
    
    for (int i = 0; i < recommendationCount; ++i) {
        int randomProductId = QRandomGenerator::global()->bounded(1, 11); // 1到10的随机ID
        
        // 避免重复推荐
        if (!mockRecommendations.contains(randomProductId)) {
            mockRecommendations.append(randomProductId);
        }
    }
    
    qDebug() << "模型返回的推荐商品ID:" << mockRecommendations;
    
    // TODO: 在这里集成真实的微调模型API调用
    // 例如：
    // 1. 发送HTTP请求到微调模型服务
    // 2. 解析返回的JSON响应
    // 3. 提取推荐的商品ID列表
    // 4. 处理错误情况
    
    return mockRecommendations;
}

QString RecommendationController::buildCartPrompt(const QList<int>& cartProductIds)
{
    QString prompt = "基于以下购物车商品，推荐其他相关商品：\n\n";
    
    // 获取购物车中商品的详细信息
    for (int productId : cartProductIds) {
        Product* product = m_productManager->getProductById(productId);
        if (product) {
            prompt += QString("- %1 (%2, 类别: %3)\n")
                     .arg(product->getName())
                     .arg(product->getDescription())
                     .arg(product->getCategory());
        } else {
            prompt += QString("- 商品ID: %1 (详情获取失败)\n").arg(productId);
        }
    }
    
    prompt += "\n请推荐3-5个与这些商品相关或互补的商品。";
    
    return prompt;
}

QString RecommendationController::buildUserQueryPrompt(const QString& userQuery)
{
    QString prompt = QString("用户查询: \"%1\"\n\n").arg(userQuery);
    prompt += "基于用户的需求，请推荐3-5个最相关的商品。";
    
    return prompt;
}

QList<Product*> RecommendationController::convertIdsToProducts(const QList<int>& productIds)
{
    qDebug() << "RecommendationController::convertIdsToProducts 开始转换，输入ID数量:" << productIds.size();
    qDebug() << "要转换的商品ID列表:" << productIds;
    
    QList<Product*> products;
    
    for (int productId : productIds) {
        Product* product = m_productManager->getProductById(productId);
        if (product) {
            products.append(product);
            qDebug() << "✓ 成功找到并添加推荐商品:" << product->getName() 
                     << "(ID:" << productId << ", 价格:¥" << product->getPrice() << ")";
        } else {
            qWarning() << "✗ 未找到商品ID:" << productId;
        }
    }
    
    qDebug() << "RecommendationController::convertIdsToProducts 转换完成，成功:" << products.size() 
             << "失败:" << (productIds.size() - products.size());
    return products;
}

// ========== 新增的AI API调用方法 ==========

void RecommendationController::setAiServerUrl(const QString& url)
{
    m_aiServerUrl = url;
    qDebug() << "RecommendationController AI服务器地址已更新:" << m_aiServerUrl;
}

void RecommendationController::setUseAiModel(bool useAi)
{
    m_useAiModel = useAi;
    qDebug() << "RecommendationController AI模型使用状态已更新:" << m_useAiModel;
}

void RecommendationController::sendAiRequest(const QString& endpoint, const QJsonObject& jsonData, const QString& requestType)
{
    qDebug() << "RecommendationController::sendAiRequest 开始";
    qDebug() << "端点:" << endpoint;
    qDebug() << "请求类型:" << requestType;
    qDebug() << "请求数据:" << QJsonDocument(jsonData).toJson(QJsonDocument::Compact);
    
    QUrl url(m_aiServerUrl + endpoint);
    QNetworkRequest request(url);
    
    // 设置请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Accept", "application/json");
    
    // 转换JSON数据
    QByteArray postData = QJsonDocument(jsonData).toJson();
    
    // 发送POST请求
    QNetworkReply* reply = m_networkManager->post(request, postData);
    
    // 存储请求类型以便后续处理
    m_pendingRequests[reply] = requestType;
    
    // 连接响应信号
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        onAiRecommendationFinished(reply);
    });
    
    qDebug() << "AI推荐请求已发送到:" << url.toString();
}

void RecommendationController::onAiRecommendationFinished(QNetworkReply* reply)
{
    qDebug() << "RecommendationController::onAiRecommendationFinished AI推荐响应接收完成";
    
    // 确保回复会被自动删除
    reply->deleteLater();
    
    // 获取请求类型
    QString requestType = m_pendingRequests.take(reply);
    qDebug() << "处理响应类型:" << requestType;
    
    // 检查网络错误
    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg = QString("AI推荐请求失败: %1").arg(reply->errorString());
        qWarning() << errorMsg;
        emit recommendationError(errorMsg);
        return;
    }
    
    // 检查HTTP状态码
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "HTTP状态码:" << statusCode;
    
    if (statusCode != 200) {
        QString errorMsg = QString("AI服务器响应错误，状态码: %1").arg(statusCode);
        qWarning() << errorMsg;
        emit recommendationError(errorMsg);
        return;
    }
    
    // 读取响应数据
    QByteArray responseData = reply->readAll();
    qDebug() << "AI服务器响应数据:" << responseData;
    
    // 解析JSON响应
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        QString errorMsg = QString("解析AI响应JSON失败: %1").arg(parseError.errorString());
        qWarning() << errorMsg;
        emit recommendationError(errorMsg);
        return;
    }
    
    QJsonObject jsonResponse = jsonDoc.object();
    
    // 检查API响应状态
    if (!jsonResponse["success"].toBool()) {
        QString errorMsg = jsonResponse["message"].toString();
        if (errorMsg.isEmpty()) {
            errorMsg = "AI推荐失败，未知错误";
        }
        qWarning() << "AI推荐API错误:" << errorMsg;
        emit recommendationError(errorMsg);
        return;
    }
    
    // 解析推荐结果
    QList<Product*> recommendedProducts = parseAiResponse(jsonResponse);
    
    qDebug() << "AI推荐解析完成，推荐商品数量:" << recommendedProducts.size();
    
    // 发射推荐结果
    emit recommendationsReady(recommendedProducts);
}

QList<Product*> RecommendationController::parseAiResponse(const QJsonObject& jsonResponse)
{
    qDebug() << "RecommendationController::parseAiResponse 开始解析AI响应";
    QList<Product*> recommendedProducts;
    QJsonArray recommendations = jsonResponse["recommendations"].toArray();
    qDebug() << "AI响应中的推荐数量:" << recommendations.size();

    for (const QJsonValue& value : recommendations) {
        QJsonObject productObj = value.toObject();
        QString productId = productObj["product_id"].toString();
        QString productName = productObj["product_name"].toString();
        QString category = productObj["category"].toString();
        double price = productObj["price"].toDouble();
        QString description = productObj["description"].toString();

        qDebug() << "解析AI推荐商品:" << productName << "ID:" << productId << "价格:" << price;

        Product* product = nullptr;

        // 1. 优先用名称精准查找
        if (m_productManager) {
            auto products = m_productManager->searchProducts(productName);
            for (Product* p : products) {
                if (p && p->getName() == productName) {
                    product = p;
                    qDebug() << "✓ 通过名称精准找到本地商品:" << productName << "(ID:" << product->getProductId() << ")";
                    break;
                }
            }
        }

        // 2. 如果没找到，再尝试用条码查找（仅当AI返回的product_id全为数字时）
        if (!product && m_productManager) {
            bool isAllDigit = !productId.isEmpty() && productId.indexOf(QRegularExpression("[^0-9]")) == -1;
            if (isAllDigit) {
                // 获取所有商品
                auto allProducts = m_productManager->searchProducts("");
                for (Product* p : allProducts) {
                    if (p && p->getBarcode() == productId) {
                        product = p;
                        qDebug() << "✓ 通过条码找到本地商品:" << productName << "(ID:" << product->getProductId() << ")";
                        break;
                    }
                }
            }
        }

        // 3. 如果还没找到，创建临时商品
        if (!product) {
            product = createTestProduct(productId, productName, category, price, description);
            if (product) {
                qDebug() << "✓ 创建临时AI推荐商品:" << productName;
            } else {
                qWarning() << "✗ 创建AI推荐商品失败:" << productName;
                continue;
            }
        }

        recommendedProducts.append(product);
    }

    qDebug() << "RecommendationController::parseAiResponse 解析完成，成功创建" << recommendedProducts.size() << "个推荐商品";
    return recommendedProducts;
}

Product* RecommendationController::createTestProduct(const QString& productId, const QString& name, 
                                                   const QString& category, double price, const QString& description)
{
    qDebug() << "RecommendationController::createTestProduct 创建商品:" << name;
    
    try {
        // 为了简化，我们创建一个临时的Product对象
        // 在实际应用中，您可能希望将这些商品存储在ProductManager中
        Product* product = new Product(static_cast<QObject*>(this));
        
        // 生成一个简单的数字ID（从productId提取或生成）
        int numericId = qHash(productId) % 10000; // 生成0-9999的ID
        
        product->setProductId(numericId);
        product->setName(name);
        product->setCategory(category);
        product->setPrice(price);
        product->setDescription(description);
        product->setBarcode(QString("AI_%1").arg(productId)); // 设置一个唯一的条码
        product->setStockQuantity(50); // 默认库存
        
        qDebug() << "✓ 成功创建AI推荐商品:" << name << "(ID:" << numericId << ")";
        
        return product;
        
    } catch (const std::exception& e) {
        qWarning() << "创建商品对象失败:" << e.what();
        return nullptr;
    }
} 