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
    void onStartScan();
    void onStopScan();
    
    // AI推荐槽函数
    void onRecommendationSelected();
    void onRecommendationsUpdated(const QList<int>& productIds);
    void onRefreshRecommendations();
    void onRecommendationAddToCart(int productId);
    
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
    void onAbout();
    void updateTime();

private:
    /**
     * @brief 初始化用户界面
     */
    void initializeUI();
    
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
     */
    void showSuccessMessage(const QString& message);

private:
    // UI
    std::unique_ptr<Ui::MainWindow> ui;

    // 核心控制器
    std::unique_ptr<CheckoutController> m_checkoutController;
    std::unique_ptr<ProductManager> m_productManager;
    std::unique_ptr<AIRecommender> m_aiRecommender;
    std::unique_ptr<BarcodeScanner> m_barcodeScanner;
    CartDelegate* m_cartDelegate;
    
    // UI文件中的组件引用（通过UI文件自动生成）
    QStandardItemModel* m_productModel;

    // 退出标志
    bool m_isClosing = false;

    // 当前销售和当前用户
    Sale* m_currentSale = nullptr;
    QString m_currentUser = QStringLiteral("收银员");
};

#endif // MAINWINDOW_H
