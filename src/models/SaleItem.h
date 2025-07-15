#ifndef SALEITEM_H
#define SALEITEM_H

#include <QObject>
#include "Product.h"

/**
 * @brief SaleItem类 - 销售项目数据模型
 * 
 * 代表销售交易中的一个项目（一种商品及其数量）
 */
class SaleItem : public QObject
{
    Q_OBJECT
    
public:
    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    explicit SaleItem(QObject *parent = nullptr);
    
    /**
     * @brief 参数化构造函数
     * @param product 商品指针
     * @param quantity 数量
     * @param unitPrice 单价（销售时的价格）
     * @param parent 父对象指针
     */
    SaleItem(Product* product, int quantity, double unitPrice, QObject *parent = nullptr);
    
    // Getter 方法
    Product* getProduct() const { return m_product; }
    int getQuantity() const { return m_quantity; }
    double getUnitPrice() const { return m_unitPrice; }
    double getSubtotal() const { return m_subtotal; }
    
    // Setter 方法
    void setProduct(Product* product);
    void setQuantity(int quantity);
    void setUnitPrice(double unitPrice);
    
    /**
     * @brief 计算小计
     */
    void calculateSubtotal();
    
    /**
     * @brief 检查销售项目是否有效
     * @return 如果商品存在且数量大于0返回true
     */
    bool isValid() const;
    
    /**
     * @brief 转换为字符串表示
     * @return 销售项目信息的字符串表示
     */
    QString toString() const;

signals:
    /**
     * @brief 销售项目发生变化时发射的信号
     */
    void itemChanged();
    
    /**
     * @brief 小计发生变化时发射的信号
     * @param newSubtotal 新的小计金额
     */
    void subtotalChanged(double newSubtotal);

private:
    Product* m_product;     ///< 商品指针
    int m_quantity;         ///< 数量
    double m_unitPrice;     ///< 单价（销售时的价格）
    double m_subtotal;      ///< 小计金额
};

#endif // SALEITEM_H
