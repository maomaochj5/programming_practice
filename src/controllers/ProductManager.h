#ifndef PRODUCTMANAGER_H
#define PRODUCTMANAGER_H

#include <QObject>
#include <QList>
#include <memory>

// 前向声明
class Product;
class DatabaseManager;

/**
 * @brief ProductManager类 - 商品管理器
 * 
 * 负责商品的增删改查业务逻辑
 */
class ProductManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    explicit ProductManager(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~ProductManager();

    /**
     * @brief 添加商品
     * @param product 商品对象指针
     * @return 如果添加成功返回true
     */
    bool addProduct(Product* product);

    /**
     * @brief 更新商品
     * @param product 商品对象指针
     * @return 如果更新成功返回true
     */
    bool updateProduct(Product* product);

    /**
     * @brief 删除商品
     * @param productId 商品ID
     * @return 如果删除成功返回true
     */
    bool deleteProduct(int productId);

    /**
     * @brief 根据ID获取商品
     * @param productId 商品ID
     * @return 商品指针，如果未找到返回nullptr
     */
    Product* getProduct(int productId);

    /**
     * @brief 根据ID获取商品（别名方法）
     * @param productId 商品ID
     * @return 商品指针，如果未找到返回nullptr
     */
    Product* getProductById(int productId);

    /**
     * @brief 根据条码获取商品
     * @param barcode 条形码
     * @return 商品指针，如果未找到返回nullptr
     */
    Product* getProductByBarcode(const QString& barcode);

    /**
     * @brief 获取所有商品
     * @return 商品指针列表
     */
    QList<Product*> getAllProducts();

    /**
     * @brief 根据分类获取商品
     * @param category 商品分类
     * @return 商品指针列表
     */
    QList<Product*> getProductsByCategory(const QString& category);

    /**
     * @brief 搜索商品
     * @param keyword 搜索关键词（在名称和描述中搜索）
     * @return 商品指针列表
     */
    QList<Product*> searchProducts(const QString& keyword);

    /**
     * @brief 获取低库存商品
     * @param threshold 库存阈值（默认5）
     * @return 低库存商品列表
     */
    QList<Product*> getLowStockProducts(int threshold = 5);

    /**
     * @brief 更新商品库存
     * @param productId 商品ID
     * @param newStock 新库存数量
     * @return 如果更新成功返回true
     */
    bool updateProductStock(int productId, int newStock);

    /**
     * @brief 批量更新商品价格
     * @param categoryOrAll 分类名称，或"ALL"表示所有商品
     * @param adjustmentType 调整类型："percentage"（百分比）或"fixed"（固定金额）
     * @param adjustmentValue 调整值
     * @return 如果更新成功返回true
     */
    bool batchUpdatePrices(const QString& categoryOrAll, 
                          const QString& adjustmentType, 
                          double adjustmentValue);

    /**
     * @brief 验证条码唯一性
     * @param barcode 条形码
     * @param excludeProductId 排除的商品ID（用于更新时检查）
     * @return 如果条码唯一返回true
     */
    bool isBarcodeUnique(const QString& barcode, int excludeProductId = -1);

    /**
     * @brief 验证商品数据
     * @param product 商品对象指针
     * @return 如果数据有效返回true
     */
    bool validateProduct(Product* product);

    /**
     * @brief 获取所有商品分类
     * @return 分类名称列表
     */
    QStringList getAllCategories();

    /**
     * @brief 设置低库存阈值
     * @param threshold 阈值
     */
    void setLowStockThreshold(int threshold);

    /**
     * @brief 获取低库存阈值
     * @return 当前阈值
     */
    int getLowStockThreshold() const;

    /**
     * @brief 导入商品数据（从CSV文件）
     * @param filePath CSV文件路径
     * @return 成功导入的商品数量
     */
    int importProductsFromCSV(const QString& filePath);

    /**
     * @brief 导出商品数据（到CSV文件）
     * @param filePath CSV文件路径
     * @return 如果导出成功返回true
     */
    bool exportProductsToCSV(const QString& filePath);

signals:
    /**
     * @brief 商品被添加时发射的信号
     * @param product 新添加的商品
     */
    void productAdded(Product* product);

    /**
     * @brief 商品被更新时发射的信号
     * @param product 被更新的商品
     */
    void productUpdated(Product* product);

    /**
     * @brief 商品被删除时发射的信号
     * @param productId 被删除商品的ID
     */
    void productDeleted(int productId);

    /**
     * @brief 商品库存发生变化时发射的信号
     * @param productId 商品ID
     * @param newStock 新库存数量
     */
    void stockChanged(int productId, int newStock);

    /**
     * @brief 低库存警告信号
     * @param productId 商品ID
     * @param currentStock 当前库存
     */
    void lowStockWarning(int productId, int currentStock);

    /**
     * @brief 操作错误时发射的信号
     * @param errorMessage 错误消息
     */
    void errorOccurred(const QString& errorMessage);

private:
    /**
     * @brief 检查并发出低库存警告
     * @param product 商品对象
     */
    void checkLowStockWarning(Product* product);

    /**
     * @brief 记录操作日志
     * @param operation 操作类型
     * @param productInfo 商品信息
     */
    void logOperation(const QString& operation, const QString& productInfo);

    /**
     * @brief 刷新商品缓存
     */
    void refreshProductCache();

private:
    DatabaseManager* m_databaseManager;     ///< 数据库管理器
    QList<Product*> m_productCache;         ///< 商品缓存
    int m_lowStockThreshold;                ///< 低库存阈值
    bool m_cacheDirty;                      ///< 缓存是否脏
};

#endif // PRODUCTMANAGER_H
