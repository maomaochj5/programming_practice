#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <QObject>
#include <QString>
#include <QDateTime>

/**
 * @brief Customer类 - 客户数据模型
 * 
 * 代表一个客户的信息，用于会员管理和AI推荐系统
 */
class Customer : public QObject
{
    Q_OBJECT
    
public:
    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    explicit Customer(QObject *parent = nullptr);
    
    /**
     * @brief 参数化构造函数
     * @param customerId 客户ID
     * @param name 客户姓名
     * @param contactInfo 联系信息
     * @param loyaltyPoints 积分
     * @param parent 父对象指针
     */
    Customer(int customerId, const QString& name, const QString& contactInfo,
            int loyaltyPoints = 0, QObject *parent = nullptr);
    
    // Getter 方法
    int getCustomerId() const { return m_customerId; }
    QString getName() const { return m_name; }
    QString getContactInfo() const { return m_contactInfo; }
    int getLoyaltyPoints() const { return m_loyaltyPoints; }
    QDateTime getRegistrationDate() const { return m_registrationDate; }
    QDateTime getLastVisit() const { return m_lastVisit; }
    
    // Setter 方法
    void setCustomerId(int customerId);
    void setName(const QString& name);
    void setContactInfo(const QString& contactInfo);
    void setLoyaltyPoints(int loyaltyPoints);
    void setRegistrationDate(const QDateTime& date);
    void setLastVisit(const QDateTime& date);
    
    /**
     * @brief 增加积分
     * @param points 要增加的积分数
     */
    void addLoyaltyPoints(int points);
    
    /**
     * @brief 使用积分
     * @param points 要使用的积分数
     * @return 如果积分足够且使用成功返回true
     */
    bool useLoyaltyPoints(int points);
    
    /**
     * @brief 更新最后访问时间为当前时间
     */
    void updateLastVisit();
    
    /**
     * @brief 检查客户信息是否有效
     * @return 如果客户信息完整返回true
     */
    bool isValid() const;
    
    /**
     * @brief 转换为字符串表示
     * @return 客户信息的字符串表示
     */
    QString toString() const;

signals:
    /**
     * @brief 客户信息发生变化时发射的信号
     */
    void customerChanged();
    
    /**
     * @brief 积分发生变化时发射的信号
     * @param newPoints 新的积分数量
     */
    void loyaltyPointsChanged(int newPoints);

private:
    int m_customerId;           ///< 客户ID（主键）
    QString m_name;             ///< 客户姓名
    QString m_contactInfo;      ///< 联系信息（电话/邮箱）
    int m_loyaltyPoints;        ///< 会员积分
    QDateTime m_registrationDate;   ///< 注册日期
    QDateTime m_lastVisit;      ///< 最后访问时间
};

#endif // CUSTOMER_H
