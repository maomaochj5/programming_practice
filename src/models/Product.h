#ifndef PRODUCT_H
#define PRODUCT_H

#include <QObject>
#include <QString>

/**
 * @brief Product类 - 商品数据模型
 * 
 * 代表超市中的一个商品，包含所有必要的商品信息
 */
class Product : public QObject
{
    Q_OBJECT
    
public:
    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    explicit Product(QObject *parent = nullptr);
    
    /**
     * @brief 参数化构造函数
     * @param productId 商品ID
     * @param barcode 条形码
     * @param name 商品名称
     * @param description 商品描述
     * @param price 价格
     * @param stockQuantity 库存数量
     * @param category 商品分类
     * @param parent 父对象指针
     */
    Product(int productId, const QString& barcode, const QString& name,
           const QString& description, double price, int stockQuantity,
           const QString& category, QObject *parent = nullptr);
           
    // 拷贝构造函数
    Product(const Product& other);
    
    // 赋值操作符
    Product& operator=(const Product& other);
    
    // Getter 方法
    int getProductId() const { return m_productId; }
    QString getBarcode() const { return m_barcode; }
    QString getName() const { return m_name; }
    QString getDescription() const { return m_description; }
    double getPrice() const { return m_price; }
    int getStockQuantity() const { return m_stockQuantity; }
    QString getCategory() const { return m_category; }
    
    // Setter 方法
    void setProductId(int productId);
    void setBarcode(const QString& barcode);
    void setName(const QString& name);
    void setDescription(const QString& description);
    void setPrice(double price);
    void setStockQuantity(int stockQuantity);
    void setCategory(const QString& category);
    
    /**
     * @brief 检查商品是否有效
     * @return 如果商品信息完整且有效返回true
     */
    bool isValid() const;
    
    /**
     * @brief 检查是否有库存
     * @return 如果库存大于0返回true
     */
    bool isInStock() const;
    
    /**
     * @brief 减少库存
     * @param quantity 要减少的数量
     * @return 如果操作成功返回true
     */
    bool decreaseStock(int quantity);
    
    /**
     * @brief 增加库存
     * @param quantity 要增加的数量
     */
    void increaseStock(int quantity);
    
    /**
     * @brief 转换为字符串表示
     * @return 商品信息的字符串表示
     */
    QString toString() const;

signals:
    /**
     * @brief 商品信息发生变化时发射的信号
     */
    void productChanged();
    
    /**
     * @brief 库存发生变化时发射的信号
     * @param newStock 新的库存数量
     */
    void stockChanged(int newStock);

private:
    int m_productId;        ///< 商品ID（主键）
    QString m_barcode;      ///< 条形码
    QString m_name;         ///< 商品名称
    QString m_description;  ///< 商品描述
    double m_price;         ///< 价格
    int m_stockQuantity;    ///< 库存数量
    QString m_category;     ///< 商品分类
};

#endif // PRODUCT_H
