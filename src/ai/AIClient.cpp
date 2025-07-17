#include "AIClient.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QDebug>
#include <QFile>
#include <QDir>

AIClient::AIClient(QObject *parent)
    : QObject(parent), m_networkManager(new QNetworkAccessManager(this)), m_mappingsLoaded(false)
{
    m_baseUrl = "http://127.0.0.1:5001";
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &AIClient::onNetworkReplyFinished);
    loadIdMappings();
}

void AIClient::loadIdMappings()
{
    // Use relative path from build directory to project root
    QString aiToDbPath = "../Ai_model/ai_to_db_id_mapping.json";
    QString dbToAiPath = "../Ai_model/db_to_ai_id_mapping.json";
    
    // Load AI ID to DB ID mapping
    QFile aiToDbFile(aiToDbPath);
    if (aiToDbFile.open(QIODevice::ReadOnly)) {
        QByteArray data = aiToDbFile.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isObject()) {
            m_aiToDbMapping = doc.object();
            qDebug() << "Loaded AI to DB ID mapping with" << m_aiToDbMapping.size() << "entries";
        }
    } else {
        qWarning() << "Failed to load AI to DB ID mapping from" << aiToDbPath;
    }
    
    // Load DB ID to AI ID mapping
    QFile dbToAiFile(dbToAiPath);
    if (dbToAiFile.open(QIODevice::ReadOnly)) {
        QByteArray data = dbToAiFile.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isObject()) {
            m_dbToAiMapping = doc.object();
            qDebug() << "Loaded DB to AI ID mapping with" << m_dbToAiMapping.size() << "entries";
        }
    } else {
        qWarning() << "Failed to load DB to AI ID mapping from" << dbToAiPath;
    }
    
    m_mappingsLoaded = (!m_aiToDbMapping.isEmpty() && !m_dbToAiMapping.isEmpty());
    if (m_mappingsLoaded) {
        qDebug() << "ID mappings loaded successfully";
    } else {
        qWarning() << "ID mappings failed to load - product recommendations may not work correctly";
    }
}

QList<QString> AIClient::convertDbIdsToAiIds(const QList<int>& dbIds)
{
    QList<QString> aiIds;
    if (!m_mappingsLoaded) {
        qWarning() << "ID mappings not loaded, cannot convert DB IDs to AI IDs";
        return aiIds;
    }
    
    for (int dbId : dbIds) {
        QString dbIdStr = QString::number(dbId);
        if (m_dbToAiMapping.contains(dbIdStr)) {
            QString aiId = m_dbToAiMapping.value(dbIdStr).toString();
            aiIds.append(aiId);
            qDebug() << "Converted DB ID" << dbId << "to AI ID" << aiId;
        } else {
            qWarning() << "No AI ID mapping found for DB ID:" << dbId;
        }
    }
    return aiIds;
}

QList<int> AIClient::convertAiIdsToDbIds(const QList<QString>& aiIds)
{
    QList<int> dbIds;
    if (!m_mappingsLoaded) {
        qWarning() << "ID mappings not loaded, cannot convert AI IDs to DB IDs";
        return dbIds;
    }
    
    for (const QString& aiId : aiIds) {
        if (m_aiToDbMapping.contains(aiId)) {
            int dbId = m_aiToDbMapping.value(aiId).toInt();
            dbIds.append(dbId);
            qDebug() << "Converted AI ID" << aiId << "to DB ID" << dbId;
        } else {
            qWarning() << "No DB ID mapping found for AI ID:" << aiId;
        }
    }
    return dbIds;
}

void AIClient::getRecommendations(const QList<int>& cartProductIds)
{
    if (!m_mappingsLoaded) {
        emit errorOccurred("ID mappings not loaded. Please check mapping files.");
        return;
    }
    
    m_currentRequestType = CartRecommendation;  // 设置请求类型为购物车推荐
    
    // Convert DB IDs to AI IDs for the API call
    QList<QString> aiIds = convertDbIdsToAiIds(cartProductIds);
    
    QJsonObject json;
    QJsonArray idArray;
    for (const QString& aiId : aiIds) {
        idArray.append(aiId);
    }
    json["cart_items"] = idArray;

    qDebug() << "Sending cart recommendation request with AI IDs:" << aiIds;

    QNetworkRequest request(QUrl(m_baseUrl + "/recommend"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    m_networkManager->post(request, QJsonDocument(json).toJson());
}

void AIClient::ask(const QString& userQuery)
{
    m_currentRequestType = UserQuery;  // 设置请求类型为用户查询
    
    QJsonObject json;
    json["query"] = userQuery;

    qDebug() << "Sending user query:" << userQuery;

    QNetworkRequest request(QUrl(m_baseUrl + "/ask"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    m_networkManager->post(request, QJsonDocument(json).toJson());
}

void AIClient::onNetworkReplyFinished(QNetworkReply *reply)
{
    if (reply->error()) {
        qDebug() << "Network error:" << reply->errorString();
        emit errorOccurred(reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
    
    if (jsonDoc.isNull() || !jsonDoc.isObject()) {
        emit errorOccurred("Invalid JSON response from server.");
        reply->deleteLater();
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();
    QString responseText = jsonObj.value("response").toString();
    
    QList<QString> aiProductIds;
    if (jsonObj.contains("products") && jsonObj["products"].isArray()) {
        QJsonArray productsArray = jsonObj["products"].toArray();
        for (const QJsonValue &value : productsArray) {
            QString aiId = value.toString();
            aiProductIds.append(aiId);
        }
    }

    // Convert AI IDs back to DB IDs for the C++ application
    QList<int> dbProductIds = convertAiIdsToDbIds(aiProductIds);
    
    qDebug() << "Received AI response with" << aiProductIds.size() << "AI IDs, converted to" << dbProductIds.size() << "DB IDs";
    qDebug() << "AI IDs:" << aiProductIds;
    qDebug() << "DB IDs:" << dbProductIds;

    // 根据请求类型发射不同的信号
    if (m_currentRequestType == CartRecommendation) {
        qDebug() << "Emitting cartRecommendationsReady signal for cart recommendation";
        emit cartRecommendationsReady(dbProductIds);
    } else if (m_currentRequestType == UserQuery) {
        qDebug() << "Emitting userQueryRecommendationsReady signal for user query";
        emit userQueryRecommendationsReady(responseText, dbProductIds);
    }
    
    // 保持向后兼容性，仍然发射原始信号
    emit recommendationsReady(responseText, dbProductIds);
    reply->deleteLater();
} 