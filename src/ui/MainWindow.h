#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QListWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <memory>
#include <QCloseEvent>

#include "CartDelegate.h"
#include "RecommendationItemWidget.h"

// UI文件头文件，在编译时生成
#ifdef QT_UI_HEADER
#include "ui_MainWindow.h"
#endif

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
namespace Ui {
class MainWindow;
}

// 前向声明
class CheckoutController;
class ProductManager;
class AIRecommender;
class BarcodeScanner;
class Product;
class Sale;
class CartDelegate;
class AIClient; // New AI Client

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
    void onItemQuantityChanged(QStandardItem *item);
    void onRemoveItem();
    void onProcessPayment();
    void onClearSale();
    void onRemoveItemFromCart(const QModelIndex &index);
    
    // 商品管理槽函数
    void onManageProducts();
    void onProductDoubleClicked(const QModelIndex &index);
    
    // 条码扫描槽函数
    void onBarcodeScanned(const QString& barcode);
    void onProductFoundByBarcode(Product* product, const QString& barcode);
    void onSearchProduct();
    
    // 图片扫描槽函数
    void onSelectImage();
    void onSelectFolder();
    void onImageLoaded(const QPixmap& image, const QString& filePath);
    void onScanProgressUpdated(double progress);
    void onScanAnimationFinished();
    
    // AI推荐槽函数
    void onRecommendationSelected();
    void onRecommendationsUpdated(const QList<int>& productIds);
    void onRefreshRecommendations();
    void onRecommendationAddToCart(int productId);
    void onRecommendationProductSelected(int productId);
    
    // 新的推荐控制器槽函数
    void onCartUpdated();
    void onAiSearchClicked();
    void onUserQueryRecommendationsReady(const QString& responseText, const QList<int>& productIds);  // 用户查询推荐（显示在左下角）
    void addRecommendedItemsToCart(const QList<int>& productIds);
    
    // Unified search/scan slot
    void onSearchOrScan();

    // 购物车和支付槽函数
    void onAddToCart();
    void onApplyDiscount();
    void onPrintReceipt();
    void onRefreshProducts();
    
    // 系统槽函数
    void onShowStatistics();
    void onShowSettings();
    void updateTime();
    void onSaleCompleted(Sale* sale);

private:
    /**
     * @brief 初始化用户界面
     */
    void initializeUI();
    
    /**
     * @brief 设置信号槽连接
     */
    void setupConnections();

    /**
     * @brief 更新扫描动画
     * @param progress 扫描进度 (0.0 to 1.0)
     */
    void updateScanAnimation(double progress);
    
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
    void updateProductDisplay(const QList<Product*>& products);
    
    /**
     * @brief 更新推荐商品显示
     */
    void updateRecommendationDisplay();
    
    /**
     * @brief 更新推荐商品显示（带参数）
     */
    void updateRecommendationDisplay(const QList<int>& productIds);
    
    /**
     * @brief 更新推荐商品显示（带Product对象列表）
     */
    void updateRecommendationDisplay(const QList<Product*>& products);
    
    /**
     * @brief 更新总计显示
     */
    void updateTotals();

    /**
     * @brief 显示错误消息
     * @param message 错误消息
     */
    void showErrorMessage(const QString& message);
    
    /**
     * @brief 显示成功消息
     * @param message 成功消息
     * @param timeout 消息停留时间（毫秒），默认3000毫秒
     */
    void showSuccessMessage(const QString& message, int timeout = 3000);

private:
    // UI
    std::unique_ptr<Ui::MainWindow> ui;

    // 扫描动画覆盖层
    QLabel* m_scanAnimationLabel;

    // 定时器
    QTimer* m_timeUpdateTimer;

    // 核心控制器
    std::unique_ptr<CheckoutController> m_checkoutController;
    std::unique_ptr<ProductManager> m_productManager;
    std::unique_ptr<AIClient> m_aiClient; // New AI Client
    std::unique_ptr<BarcodeScanner> m_barcodeScanner;
    
    // UI文件中的组件引用（通过UI文件自动生成）
    QStandardItemModel* m_productModel;
    CartDelegate *m_cartDelegate;
    
    // 额外的UI组件（程序化创建）
    QPushButton* m_aiSearchButton;
    
    // 状态保存
    Sale* m_lastCompletedSale = nullptr;

    // 退出标志
    bool m_isClosing = false;

    // 当前销售和当前用户
    Sale* m_currentSale = nullptr;
    QString m_currentUser = QStringLiteral("收银员");
};

#endif // MAINWINDOW_H
