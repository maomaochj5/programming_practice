#include "AIClient.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

AIClient::AIClient(QObject *parent)
    : QObject(parent), m_networkManager(new QNetworkAccessManager(this))
{
    m_baseUrl = "http://127.0.0.1:5001";
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &AIClient::onNetworkReplyFinished);
    qDebug() << "AIClient initialized - using direct barcode lookup (no mapping files required)";
}

QList<int> AIClient::convertBarcodeIdsToDbIds(const QList<QString>& barcodeIds)
{
    QList<int> dbIds;
    
    // Get the database connection from DatabaseManager
    QSqlDatabase db = QSqlDatabase::database("mainConnection");
    if (!db.isOpen()) {
        qWarning() << "Database not available for barcode lookup";
        return dbIds;
    }
    
    QSqlQuery query(db);
    query.prepare("SELECT product_id FROM Products WHERE barcode = ?");
    
    for (const QString& barcodeId : barcodeIds) {
        query.bindValue(0, barcodeId);
        
        if (query.exec() && query.next()) {
            int dbId = query.value(0).toInt();
            dbIds.append(dbId);
            qDebug() << "Found product for barcode" << barcodeId << "-> DB ID:" << dbId;
        } else {
            qWarning() << "No product found for barcode:" << barcodeId;
        }
    }
    
    return dbIds;
}

void AIClient::ask(const QString& userQuery)
{
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
    
    QList<QString> barcodeIds;
    if (jsonObj.contains("products") && jsonObj["products"].isArray()) {
        QJsonArray productsArray = jsonObj["products"].toArray();
        for (const QJsonValue &value : productsArray) {
            QString barcodeId = value.toString();
            barcodeIds.append(barcodeId);
        }
    }

    // Convert barcode IDs to database product IDs
    QList<int> dbProductIds = convertBarcodeIdsToDbIds(barcodeIds);
    
    qDebug() << "Received AI response with" << barcodeIds.size() << "barcode IDs, found" << dbProductIds.size() << "products in database";
    qDebug() << "Barcode IDs:" << barcodeIds;
    qDebug() << "DB Product IDs:" << dbProductIds;

    // Emit user query recommendations signal for display in the lower-left corner
    qDebug() << "Emitting userQueryRecommendationsReady signal for user query";
    emit userQueryRecommendationsReady(responseText, dbProductIds);
    
    reply->deleteLater();
} 