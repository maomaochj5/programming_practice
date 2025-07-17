#ifndef AIRECOMMENDER_H
#define AIRECOMMENDER_H

#include <QObject>
#include <QList>

class AIRecommender : public QObject
{
    Q_OBJECT

public:
    explicit AIRecommender(QObject *parent = nullptr);
    ~AIRecommender();

    QList<int> getPopularRecommendations(int topN);
};

#endif // AIRECOMMENDER_H
