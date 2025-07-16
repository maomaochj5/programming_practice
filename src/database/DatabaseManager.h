#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMutex>
#include <QFutureWatcher>
#include <memory>

// 前向声明
class Product;
class Customer;
class Sale;

/**
 * @brief DatabaseManager类 - 数据库管理器（单例模式）
 * 
 * 负责管理SQLite数据库的连接和所有数据持久化操作
 */
class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 获取单例实例
     * @return DatabaseManager的单例引用
     */
    static DatabaseManager& getInstance();

    // 禁止拷贝和赋值
    DatabaseManager(const DatabaseManager&) = delete;
    void operator=(const DatabaseManager&) = delete;

    /**
     * @brief 打开数据库连接
     * @param path 数据库文件路径
     * @return 如果成功返回true
     */
    bool openDatabase(const QString& path);

    /**
     * @brief 关闭数据库连接
     */
    void closeDatabase();

    /**
     * @brief 检查数据库是否已连接
     * @return 如果已连接返回true
     */
    bool isConnected() const;

    // 商品相关操作
    /**
     * @brief 保存商品到数据库
     * @param product 商品对象
     * @return 如果成功返回true
     */
    void saveProduct(const Product& product);

    /**
     * @brief 根据ID获取商品
     * @param productId 商品ID
     * @return 商品智能指针，如果未找到返回nullptr
     */
    std::unique_ptr<Product> getProduct(int productId);

    /**
     * @brief 根据条码获取商品
     * @param barcode 条形码
     * @return 商品智能指针，如果未找到返回nullptr
     */
    void getProductByBarcode(const QString& barcode);

    /**
     * @brief 获取所有商品
     * @return 商品指针列表（由调用方负责释放）
     */
    void getAllProducts();

    /**
     * @brief 删除商品
     * @param productId 商品ID
     * @return 如果成功返回true
     */
    void deleteProduct(int productId);

    /**
     * @brief 更新商品库存
     * @param productId 商品ID
     * @param newStock 新库存数量
     * @return 如果成功返回true
     */
    bool updateProductStock(int productId, int newStock);

    // 客户相关操作
    /**
     * @brief 保存客户到数据库
     * @param customer 客户对象
     * @return 如果成功返回true
     */
    bool saveCustomer(Customer* customer);

    /**
     * @brief 根据ID获取客户
     * @param customerId 客户ID
     * @return 客户智能指针，如果未找到返回nullptr
     */
    std::unique_ptr<Customer> getCustomer(int customerId);

    /**
     * @brief 获取所有客户
     * @return 客户智能指针列表
     */
    QList<std::unique_ptr<Customer>> getAllCustomers();

    /**
     * @brief 删除客户
     * @param customerId 客户ID
     * @return 如果成功返回true
     */
    bool deleteCustomer(int customerId);

    // 交易相关操作
    /**
     * @brief 保存交易到数据库
     * @param sale 交易对象
     * @return 如果成功返回交易ID，失败返回-1
     */
    int saveTransaction(Sale* sale);

    /**
     * @brief 根据ID获取交易
     * @param transactionId 交易ID
     * @return 交易智能指针，如果未找到返回nullptr
     */
    std::unique_ptr<Sale> getTransaction(int transactionId);

    /**
     * @brief 获取客户的交易历史
     * @param customerId 客户ID
     * @param limit 限制返回数量（默认100）
     * @return 交易智能指针列表
     */
    QList<std::unique_ptr<Sale>> getCustomerTransactions(int customerId, int limit = 100);

    /**
     * @brief 获取日期范围内的交易
     * @param startDate 开始日期
     * @param endDate 结束日期
     * @return 交易智能指针列表
     */
    QList<std::unique_ptr<Sale>> getTransactionsByDateRange(const QDateTime& startDate, 
                                                           const QDateTime& endDate);

    // 报表和统计相关
    /**
     * @brief 获取商品销售统计
     * @param days 统计天数
     * @return 商品ID和销售数量的映射
     */
    QHash<int, int> getProductSalesStats(int days = 30);

    /**
     * @brief 获取收入统计
     * @param days 统计天数
     * @return 指定天数内的总收入
     */
    double getRevenueStats(int days = 30);

    /**
     * @brief 获取热门商品
     * @param limit 返回数量限制
     * @param days 统计天数
     * @return 商品ID列表，按销量排序
     */
    QList<int> getPopularProducts(int limit = 10, int days = 30);

signals:
    /**
     * @brief 数据库连接状态改变时发射的信号
     * @param connected 是否已连接
     */
    void connectionStatusChanged(bool connected);

    /**
     * @brief 数据库错误时发射的信号
     * @param error 错误信息
     */
    void databaseError(const QString& error);

    void productsRead(const QList<Product*>& products);
    void productReadByBarcode(Product* product, const QString& barcode);
    void productSaved(bool success, int productId);
    void productDeleted(bool success, int productId);

private slots:
    void handleProductsRead();
    void handleProductSaved();

private:
    void handleProductReadByBarcode(Product* product, const QString& barcode);
    void handleProductDeleted(bool success, int productId);
    /**
     * @brief 私有构造函数（单例模式）
     * @param parent 父对象指针
     */
    explicit DatabaseManager(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~DatabaseManager();

    /**
     * @brief 初始化数据库表结构
     * @return 如果成功返回true
     */
    bool initializeTables();

    /**
     * @brief 执行SQL查询
     * @param query SQL查询
     * @return 如果成功返回true
     */
    bool executeQuery(QSqlQuery& query);

    /**
     * @brief 记录数据库错误
     * @param context 错误上下文
     * @param error SQL错误对象
     */
    void logError(const QString& context, const QSqlError& error);

    QSqlDatabase m_db;          ///< 数据库连接
    static QMutex s_mutex;      ///< 线程安全的互斥锁
    bool m_connected;           ///< 连接状态标志
};

#endif // DATABASEMANAGER_H
