#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QListWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <memory>

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QPushButton;
class QLineEdit;
class QSpinBox;
class QDoubleSpinBox;
class QComboBox;
class QGroupBox;
class QSplitter;
QT_END_NAMESPACE

// 前向声明
class CheckoutController;
class ProductManager;
class AIRecommender;
class BarcodeScanner;
class Product;
class Sale;

/**
 * @brief MainWindow类 - 主窗口界面
 * 
 * 智能POS系统的主界面，整合所有功能模块
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    MainWindow(QWidget *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~MainWindow();

protected:
    /**
     * @brief 窗口关闭事件
     * @param event 关闭事件
     */
    void closeEvent(QCloseEvent *event) override;

private slots:
    // 销售相关槽函数
    void onNewSale();
    void onSaleUpdated();
    void onItemQuantityChanged();
    void onRemoveItem();
    void onProcessPayment();
    void onClearSale();
    
    // 商品管理槽函数
    void onAddProduct();
    void onEditProduct();
    void onDeleteProduct();
    void onRefreshProducts();
    
    // 条码扫描槽函数
    void onBarcodeScanned(const QString& barcode);
    void onManualBarcodeEntry();
    
    // AI推荐槽函数
    void onRecommendationSelected();
    void onRecommendationsUpdated(const QList<int>& productIds);
    
    // 系统槽函数
    void onShowStatistics();
    void onShowSettings();
    void onAbout();
    void updateStatusBar();

private:
    /**
     * @brief 初始化用户界面
     */
    void initializeUI();
    
    /**
     * @brief 创建菜单栏
     */
    void createMenuBar();
    
    /**
     * @brief 创建工具栏
     */
    void createToolBar();
    
    /**
     * @brief 创建状态栏
     */
    void createStatusBar();
    
    /**
     * @brief 创建中央窗口部件
     */
    void createCentralWidget();
    
    /**
     * @brief 创建销售区域
     * @return 销售区域的组件
     */
    QWidget* createSalesArea();
    
    /**
     * @brief 创建商品管理区域
     * @return 商品管理区域的组件
     */
    QWidget* createProductArea();
    
    /**
     * @brief 创建推荐区域
     * @return 推荐区域的组件
     */
    QWidget* createRecommendationArea();
    
    /**
     * @brief 创建条码扫描区域
     * @return 条码扫描区域的组件
     */
    QWidget* createBarcodeScanArea();
    
    /**
     * @brief 连接信号与槽
     */
    void connectSignals();
    
    /**
     * @brief 设置样式表
     */
    void setupStyleSheet();
    
    /**
     * @brief 更新购物车显示
     */
    void updateCartDisplay();
    
    /**
     * @brief 更新商品列表显示
     */
    void updateProductDisplay();
    
    /**
     * @brief 更新推荐商品显示
     */
    void updateRecommendationDisplay();
    
    /**
     * @brief 显示错误消息
     * @param message 错误消息
     */
    void showErrorMessage(const QString& message);
    
    /**
     * @brief 显示成功消息
     * @param message 成功消息
     */
    void showSuccessMessage(const QString& message);

private:
    // 核心控制器
    std::unique_ptr<CheckoutController> m_checkoutController;
    std::unique_ptr<ProductManager> m_productManager;
    std::unique_ptr<AIRecommender> m_aiRecommender;
    std::unique_ptr<BarcodeScanner> m_barcodeScanner;
    
    // 主要UI组件
    QWidget* m_centralWidget;
    QSplitter* m_mainSplitter;
    
    // 销售区域组件
    QGroupBox* m_salesGroup;
    QListWidget* m_cartList;
    QLabel* m_totalLabel;
    QPushButton* m_newSaleButton;
    QPushButton* m_paymentButton;
    QPushButton* m_clearSaleButton;
    QPushButton* m_removeItemButton;
    QSpinBox* m_quantitySpinBox;
    
    // 商品管理区域组件
    QGroupBox* m_productGroup;
    QTableView* m_productTable;
    QStandardItemModel* m_productModel;
    QPushButton* m_addProductButton;
    QPushButton* m_editProductButton;
    QPushButton* m_deleteProductButton;
    QPushButton* m_refreshProductButton;
    
    // 条码扫描区域组件
    QGroupBox* m_barcodeGroup;
    QLabel* m_scannerStatus;
    QLineEdit* m_manualBarcodeEdit;
    QPushButton* m_manualBarcodeButton;
    QWidget* m_cameraWidget;
    
    // 推荐区域组件
    QGroupBox* m_recommendationGroup;
    QListWidget* m_recommendationList;
    QPushButton* m_addRecommendationButton;
    
    // 状态栏组件
    QLabel* m_statusLabel;
    QLabel* m_timeLabel;
    QLabel* m_userLabel;
    QTimer* m_statusTimer;
    
    // 当前数据
    Sale* m_currentSale;
    QString m_currentUser;
};

#endif // MAINWINDOW_H
