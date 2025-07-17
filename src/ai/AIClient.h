#ifndef AICLIENT_H
#define AICLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include <QList>

class AIClient : public QObject
{
    Q_OBJECT

public:
    explicit AIClient(QObject *parent = nullptr);
    
    void ask(const QString& userQuery);

signals:
    void userQueryRecommendationsReady(const QString& responseText, const QList<int>& productIds);
    void errorOccurred(const QString& error);

private slots:
    void onNetworkReplyFinished(QNetworkReply *reply);

private:
    // Convert barcode IDs to database product IDs
    QList<int> convertBarcodeIdsToDbIds(const QList<QString>& barcodeIds);
    
    QNetworkAccessManager* m_networkManager;
    QString m_baseUrl;
};

#endif // AICLIENT_H 