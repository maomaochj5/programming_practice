#ifndef AICLIENT_H
#define AICLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include <QList>
#include <QJsonObject>

class AIClient : public QObject
{
    Q_OBJECT

public:
    explicit AIClient(QObject *parent = nullptr);
    
    void getRecommendations(const QList<int>& cartProductIds);
    void ask(const QString& userQuery);

signals:
    void recommendationsReady(const QString& responseText, const QList<int>& productIds);
    void cartRecommendationsReady(const QList<int>& productIds);  // 购物车推荐（左下角显示）
    void userQueryRecommendationsReady(const QString& responseText, const QList<int>& productIds);  // 用户查询推荐（弹出对话框）
    void errorOccurred(const QString& error);

private slots:
    void onNetworkReplyFinished(QNetworkReply *reply);

private:
    enum RequestType {
        CartRecommendation,
        UserQuery
    };
    
    // ID mapping functions
    void loadIdMappings();
    QList<QString> convertDbIdsToAiIds(const QList<int>& dbIds);
    QList<int> convertAiIdsToDbIds(const QList<QString>& aiIds);
    
    QNetworkAccessManager* m_networkManager;
    QString m_baseUrl;
    RequestType m_currentRequestType;  // 跟踪当前请求类型
    
    // ID mapping dictionaries
    QJsonObject m_aiToDbMapping;    // AI ID -> DB ID
    QJsonObject m_dbToAiMapping;    // DB ID -> AI ID
    bool m_mappingsLoaded;
};

#endif // AICLIENT_H 