#include "AIRecommender.h"
#include <QDebug>

AIRecommender::AIRecommender(QObject *parent) : QObject(parent)
{
    qDebug() << "AIRecommender instantiated (stub)";
}

AIRecommender::~AIRecommender()
{
    qDebug() << "AIRecommender destroyed (stub)";
}

QList<int> AIRecommender::getPopularRecommendations(int topN)
{
    qDebug() << "AIRecommender::getPopularRecommendations called (stub). Returning empty list.";
    // This is a stub. In a real scenario, you might want to return some
    // pre-defined popular items.
    Q_UNUSED(topN);
    return {};
}
