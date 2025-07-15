#include "Customer.h"
#include <QDebug>

Customer::Customer(QObject *parent)
    : QObject(parent)
    , m_customerId(-1)
    , m_loyaltyPoints(0)
    , m_registrationDate(QDateTime::currentDateTime())
    , m_lastVisit(QDateTime::currentDateTime())
{
}

Customer::Customer(int customerId, const QString& name, const QString& contactInfo,
                  int loyaltyPoints, QObject *parent)
    : QObject(parent)
    , m_customerId(customerId)
    , m_name(name)
    , m_contactInfo(contactInfo)
    , m_loyaltyPoints(loyaltyPoints)
    , m_registrationDate(QDateTime::currentDateTime())
    , m_lastVisit(QDateTime::currentDateTime())
{
}

void Customer::setCustomerId(int customerId)
{
    if (m_customerId != customerId) {
        m_customerId = customerId;
        emit customerChanged();
    }
}

void Customer::setName(const QString& name)
{
    if (m_name != name) {
        m_name = name;
        emit customerChanged();
    }
}

void Customer::setContactInfo(const QString& contactInfo)
{
    if (m_contactInfo != contactInfo) {
        m_contactInfo = contactInfo;
        emit customerChanged();
    }
}

void Customer::setLoyaltyPoints(int loyaltyPoints)
{
    if (m_loyaltyPoints != loyaltyPoints) {
        m_loyaltyPoints = loyaltyPoints;
        emit loyaltyPointsChanged(m_loyaltyPoints);
        emit customerChanged();
    }
}

void Customer::setRegistrationDate(const QDateTime& date)
{
    if (m_registrationDate != date) {
        m_registrationDate = date;
        emit customerChanged();
    }
}

void Customer::setLastVisit(const QDateTime& date)
{
    if (m_lastVisit != date) {
        m_lastVisit = date;
        emit customerChanged();
    }
}

void Customer::addLoyaltyPoints(int points)
{
    if (points > 0) {
        setLoyaltyPoints(m_loyaltyPoints + points);
        qDebug() << "客户" << m_name << "获得积分:" << points 
                 << "总积分:" << m_loyaltyPoints;
    }
}

bool Customer::useLoyaltyPoints(int points)
{
    if (points <= 0) {
        qWarning() << "尝试使用无效数量的积分:" << points;
        return false;
    }
    
    if (m_loyaltyPoints < points) {
        qWarning() << "积分不足，无法使用。当前积分:" << m_loyaltyPoints 
                   << "尝试使用:" << points;
        return false;
    }
    
    setLoyaltyPoints(m_loyaltyPoints - points);
    qDebug() << "客户" << m_name << "使用积分:" << points 
             << "剩余积分:" << m_loyaltyPoints;
    return true;
}

void Customer::updateLastVisit()
{
    setLastVisit(QDateTime::currentDateTime());
}

bool Customer::isValid() const
{
    return m_customerId >= 0 && 
           !m_name.isEmpty() &&
           m_loyaltyPoints >= 0;
}

QString Customer::toString() const
{
    return QString("Customer[ID:%1, Name:%2, Contact:%3, Points:%4, LastVisit:%5]")
           .arg(m_customerId)
           .arg(m_name)
           .arg(m_contactInfo)
           .arg(m_loyaltyPoints)
           .arg(m_lastVisit.toString("yyyy-MM-dd hh:mm:ss"));
}
