#define QT_UI_HEADER
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "../controllers/CheckoutController.h"
#include "../controllers/ProductManager.h"
#include "../ai/AIRecommender.h"
#include "../barcode/BarcodeScanner.h"
#include "../models/Sale.h"
#include "../models/Product.h"
#include "ProductDialog.h"
#include "PaymentDialog.h"
#include "ProductManagementDialog.h"
#include "SalesReportDialog.h"
#include "../utils/ReceiptPrinter.h"
#include "ui/CartDelegate.h"
#include "ui/RecommendationItemWidget.h"

#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QListWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QTimer>
#include <QDateTime>
#include <QMessageBox>
#include <QInputDialog>
#include <QHeaderView>
#include <QCloseEvent>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(std::make_unique<Ui::MainWindow>())
    , m_currentSale(nullptr)
    , m_currentUser("收银员")
    , m_isClosing(false)
{
    qDebug() << "MainWindow 构造函数开始";
    
    // 初始化控制器
    m_checkoutController = std::make_unique<CheckoutController>(this);
    m_productManager = std::make_unique<ProductManager>(this);
    m_aiRecommender = std::make_unique<AIRecommender>(this);
    m_barcodeScanner = std::make_unique<BarcodeScanner>(this);
    
    // 初始化UI
    initializeUI();
    
    // 连接信号与槽
    connectSignals();
    
    // 设置样式
    setupStyleSheet();
    
    // 启动新的销售
    onNewSale();
    
    // 异步刷新商品列表
    m_productManager->getAllProducts();
    
    // Setup timer for clock
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateTime);
    timer->start(1000);
    updateTime(); // Initial call
    
    qDebug() << "主窗口初始化完成";
}

MainWindow::~MainWindow()
{
    qDebug() << "MainWindow 析构函数开始";
    // Clean up the saved last sale to prevent memory leaks
    if (m_lastCompletedSale) {
        delete m_lastCompletedSale;
    }
    qDebug() << "MainWindow 析构函数结束";
}

void MainWindow::initializeUI()
{
    qDebug() << "MainWindow::initializeUI called";
    setWindowTitle("智能超市收银系统 v1.0");
    setMinimumSize(1200, 800);
    resize(1400, 900);
    
    qDebug() << "MainWindow::initializeUI 准备调用 setupUi(this)";
    try {
        ui->setupUi(this);
        qDebug() << "MainWindow ui.setupUi(this) 完成";
    } catch (...) {
        qDebug() << "MainWindow::initializeUI setupUi(this) 异常";
        return;
    }
    
    qDebug() << "MainWindow::initializeUI 检查 UI 组件";
    qDebug() << "cartTableView:" << ui->cartTableView;
    qDebug() << "productListWidget:" << ui->productListWidget;
    qDebug() << "recommendationListWidget:" << ui->recommendationListWidget;
    
    m_productModel = new QStandardItemModel(this);
    m_productModel->setHorizontalHeaderLabels({"商品名称", "数量", "单价", "小计", "操作"});
    if (ui->cartTableView) {
        ui->cartTableView->setModel(m_productModel);
        ui->cartTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->cartTableView->setAlternatingRowColors(true);
        ui->cartTableView->setSortingEnabled(true);
        connect(ui->cartTableView, &QTableView::doubleClicked, this, &MainWindow::onProductDoubleClicked);

        m_cartDelegate = new CartDelegate(this);
        ui->cartTableView->setItemDelegate(m_cartDelegate);

        ui->cartTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        ui->cartTableView->setColumnWidth(1, 100);
        ui->cartTableView->setColumnWidth(2, 120);
        ui->cartTableView->setColumnWidth(3, 120);
        ui->cartTableView->setColumnWidth(4, 100);
        ui->cartTableView->horizontalHeader()->setStretchLastSection(true);
    }
    if (ui->removeFromCartButton) {
        ui->removeFromCartButton->setVisible(false);
    }
    
    // Configure recommendation list
    if (ui->recommendationListWidget) {
        ui->recommendationListWidget->setFlow(QListView::LeftToRight);
        ui->recommendationListWidget->setWrapping(false);
        ui->recommendationListWidget->setSpacing(10);
        ui->recommendationListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ui->recommendationListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
    
    // 延迟调用这些函数，确保 UI 完全初始化
    QTimer::singleShot(0, this, [this]() {
        // Product display is updated asynchronously when the allProductsChanged signal is emitted.
        updateRecommendationDisplay();
    });
    
    qDebug() << "MainWindow::initializeUI 完成";
}

void MainWindow::connectSignals()
{
    if (ui->newSaleButton) connect(ui->newSaleButton, &QPushButton::clicked, this, &MainWindow::onNewSale);
    if (ui->checkoutButton) connect(ui->checkoutButton, &QPushButton::clicked, this, &MainWindow::onProcessPayment);
    if (ui->clearCartButton) connect(ui->clearCartButton, &QPushButton::clicked, this, &MainWindow::onClearSale);
    if (ui->addToCartButton) connect(ui->addToCartButton, &QPushButton::clicked, this, &MainWindow::onAddToCart);
    if (ui->manageProductsButton) connect(ui->manageProductsButton, &QPushButton::clicked, this, &MainWindow::onManageProducts);
    if (ui->printReceiptButton) connect(ui->printReceiptButton, &QPushButton::clicked, this, &MainWindow::onPrintReceipt);
    if (ui->reportsButton) connect(ui->reportsButton, &QPushButton::clicked, this, &MainWindow::onShowStatistics);
    if (ui->searchLineEdit) connect(ui->searchLineEdit, &QLineEdit::returnPressed, this, &MainWindow::onSearchOrScan);
    if (ui->searchButton) connect(ui->searchButton, &QPushButton::clicked, this, &MainWindow::onSearchOrScan);
    if (ui->startScanButton) connect(ui->startScanButton, &QPushButton::clicked, this, &MainWindow::onStartScan);
    if (ui->stopScanButton) connect(ui->stopScanButton, &QPushButton::clicked, this, &MainWindow::onStopScan);
    connect(m_barcodeScanner.get(), &BarcodeScanner::barcodeDetected, this, &MainWindow::onBarcodeScanned);
    if (ui->refreshRecommendationButton) connect(ui->refreshRecommendationButton, &QPushButton::clicked, this, &MainWindow::onRefreshRecommendations);
    if (ui->discountButton) connect(ui->discountButton, &QPushButton::clicked, this, &MainWindow::onApplyDiscount);

    // This is a robust connection. When a sale is structurally changed (item added/removed),
    // CheckoutController will emit saleUpdated(), which triggers a full refresh.
    connect(m_checkoutController.get(), &CheckoutController::saleUpdated, this, &MainWindow::updateCartDisplay);
    connect(m_checkoutController.get(), &CheckoutController::saleSuccessfullyCompleted, this, &MainWindow::onSaleCompleted);
    
    // Delegate and model signals
    connect(m_cartDelegate, &CartDelegate::removeItem, this, &MainWindow::onRemoveItemFromCart);
    connect(m_productModel, &QStandardItemModel::itemChanged, this, &MainWindow::onItemQuantityChanged);

    // Product Manager signal
    connect(m_productManager.get(), &ProductManager::allProductsChanged, this, &MainWindow::updateProductDisplay);
    connect(m_productManager.get(), &ProductManager::productFoundByBarcode, this, &MainWindow::onProductFoundByBarcode);

    if (ui->actionNewSale) connect(ui->actionNewSale, &QAction::triggered, this, &MainWindow::onNewSale);
    if (ui->actionManageProducts) connect(ui->actionManageProducts, &QAction::triggered, this, &MainWindow::onManageProducts);
    if (ui->actionRefreshProducts) connect(ui->actionRefreshProducts, &QAction::triggered, this, &MainWindow::onRefreshProducts);
    if (ui->actionExit) connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    if (ui->actionAbout) connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::onAbout);
}



void MainWindow::setupStyleSheet()
{
    this->setStyleSheet(
        "/* ---- Global ---- */"
        "QWidget { font-family: -apple-system, sans-serif; color: #333; }"
        "QMainWindow, QDialog { background-color: #f4f5f7; }"

        "/* ---- Typography ---- */"
        "QLabel { background-color: transparent; color: #333; }"
        "QLabel#totalValueLabel { font-size: 20px; font-weight: bold; color: #d32f2f; }"
        "QLabel#subtotalValueLabel, QLabel#discountValueLabel { font-size: 16px; }"

        "/* ---- Buttons ---- */"
        "QPushButton { background-color: #ffffff; border: 1px solid #dcdcdc; border-radius: 4px; padding: 8px 12px; }"
        "QPushButton:hover { background-color: #f0f0f0; }"
        "QPushButton:pressed { background-color: #e0e0e0; border-style: inset; }"
        "QPushButton:disabled { color: #b0b0b0; background-color: #f5f5f5; }"
        "QPushButton#okButton, QPushButton#processButton, QPushButton#checkoutButton { background-color: #28a745; color: white; font-weight: bold; }"
        "QPushButton#okButton:hover, QPushButton#processButton:hover, QPushButton#checkoutButton:hover { background-color: #218838; }"

        "/* ---- Inputs & Views ---- */"
        "QLineEdit, QComboBox, QListWidget, QTableView, QTextEdit, QSpinBox, QDoubleSpinBox { background-color: #ffffff; color: #333; border: 1px solid #dcdcdc; border-radius: 4px; padding: 5px; }"
        "QLineEdit:focus, QComboBox:focus, QTableView:focus, QTextEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus { border-color: #80bdff; }"
        "QComboBox::drop-down { border: none; }"
        "QComboBox QAbstractItemView { border: 1px solid #dcdcdc; background-color: white; color: #333; selection-background-color: #007bff; selection-color: white; }"
        "QTableView { gridline-color: #e0e0e0; alternate-background-color: #f9f9f9; }"
        "QTableView::corner-button {background-color: white;border: none;} "
        "QHeaderView::section { background-color: #f8f9fa; padding: 6px; border: 1px solid #dee2e6; font-weight: bold; }"
        "QHeaderView { background-color: #f8f9fa; }"

        "/* ---- Menus & Toolbars ---- */"
        "QToolBar { background-color: #343a40; border-bottom: 1px solid #212529; padding: 2px; }"
        "QToolButton { color: white; background-color: transparent; padding: 5px; border-radius: 4px; }"
        "QToolButton:hover { background-color: #495057; }"
        "QToolButton:pressed { background-color: #212529; }"
        "QToolBar::separator { width: 1px; background-color: #495057; margin: 4px 6px; }"
        "QMenuBar { background-color: #e9ecef; border-bottom: 1px solid #dcdcdc; }"
        "QMenuBar::item { color: #333; padding: 4px 10px; }"
        "QMenuBar::item:selected, QMenuBar::item:pressed { background-color: #d4dae0; color: #000; }"
        "QMenu { background-color: #ffffff; border: 1px solid #ced4da; }"
        "QMenu::item:selected { background-color: #007bff; color: white; }"

        "/* ---- StatusBar ---- */"
        "QStatusBar { background-color: #343a40; color: #ffffff; font-weight: bold; }"
        "QStatusBar::item { border: none; }"

        "/* ---- Containers ---- */"
        "QGroupBox { background-color: #ffffff; border: 1px solid #dee2e6; border-radius: 5px; margin-top: 10px; }"
        "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 10px; color: #495057; }"
        
        "/* ---- Custom Widgets ---- */"
        "RecommendationItemWidget { background-color: white; border-radius: 8px; border: 1px solid #e0e0e0; }"
        "RecommendationItemWidget QLabel { font-size: 13px; }"
        "RecommendationItemWidget QPushButton { font-size: 12px; padding: 5px; }"

        "/* ---- ScrollBars ---- */"
        "QScrollBar:vertical, QScrollBar:horizontal { border: none; background: #f0f0f0; width: 12px; margin: 0px; }"
        "QScrollBar::handle { background: #c0c0c0; border-radius: 6px; min-height: 20px; }"
        "QScrollBar::handle:hover { background: #a0a0a0; }"
    );
}

void MainWindow::onNewSale()
{
    qDebug() << "MainWindow::onNewSale called, m_currentSale:" << m_currentSale;
    if (m_currentSale) {
        // 先通知CheckoutController断开信号连接
        m_checkoutController->setCurrentSale(nullptr);
        m_currentSale->deleteLater(); // Use deleteLater for safety with QObjects
        qDebug() << "MainWindow::onNewSale scheduled old m_currentSale for deletion";
    }
    m_currentSale = new Sale(this);
    qDebug() << "MainWindow::onNewSale created new m_currentSale:" << m_currentSale;
    m_checkoutController->setCurrentSale(m_currentSale);
    qDebug() << "MainWindow::onNewSale calling updateCartDisplay";
    updateCartDisplay();
    qDebug() << "MainWindow::onNewSale calling showSuccessMessage";
    showSuccessMessage("开始新的销售");
    qDebug() << "MainWindow::onNewSale completed";
}

void MainWindow::onSaleUpdated()
{
    // This slot is now disconnected and unused, but kept for safety.
    // The main connection is now directly from CheckoutController::saleUpdated to updateCartDisplay.
    qDebug() << "onSaleUpdated (legacy) called. This should not happen frequently.";
}

void MainWindow::onBarcodeScanned(const QString& barcode)
{
    qDebug() << "扫描到条码:" << barcode;
    
    if (!m_currentSale) {
        onNewSale();
    }
    
    // Asynchronously get product by barcode
    m_productManager->getProductByBarcode(barcode);
}

void MainWindow::onProductFoundByBarcode(Product* product, const QString& barcode)
{
    if (product) {
        m_checkoutController->addItemToSale(product, 1);
        showSuccessMessage(QString("添加商品: %1").arg(product->getName()));
    } else {
        showErrorMessage(QString("未找到条码为 %1 的商品").arg(barcode));
    }
}

void MainWindow::onSearchProduct()
{
    qDebug() << "onSearchProduct triggered";
    QString searchText = ui->searchLineEdit->text().trimmed();
    if (!searchText.isEmpty()) {
        // 搜索商品逻辑
        auto products = m_productManager->searchProducts(searchText);
        updateProductDisplay(products);
        showSuccessMessage(QString("找到 %1 个商品").arg(products.size()));
    } else {
        onRefreshProducts();
    }
}

void MainWindow::onSearchOrScan()
{
    QString input = ui->searchLineEdit->text().trimmed();
    if (input.isEmpty()) {
        onRefreshProducts(); // If search is cleared, show all products
        return;
    }

    // Heuristic to decide if it's a barcode or search term
    bool isLikelyBarcode = input.toLongLong() > 0 && (input.length() == 8 || input.length() == 12 || input.length() == 13);

    if (isLikelyBarcode) {
        onBarcodeScanned(input);
        ui->searchLineEdit->clear(); // Clear after successful scan
    } else {
        onSearchProduct(); // It's a search
    }
}

void MainWindow::onStartScan()
{
    qDebug() << "onStartScan triggered";
    if (m_barcodeScanner->startScanning()) {
        ui->startScanButton->setEnabled(false);
        ui->stopScanButton->setEnabled(true);
        if(ui->scanStatusLabel) ui->scanStatusLabel->setText("扫描状态: 扫描中");
        showSuccessMessage("开始扫描条码");
    } else {
        showErrorMessage("启动扫描器失败");
    }
}

void MainWindow::onStopScan()
{
    qDebug() << "onStopScan triggered";
    m_barcodeScanner->stopScanning();
    ui->startScanButton->setEnabled(true);
    ui->stopScanButton->setEnabled(false);
    if(ui->scanStatusLabel) ui->scanStatusLabel->setText("扫描状态: 已停止");
    showSuccessMessage("停止扫描");
}

void MainWindow::onAddToCart()
{
    qDebug() << "onAddToCart triggered";
    // 从商品列表中选择商品添加到购物车
    auto currentItem = ui->productListWidget->currentItem();
    if (currentItem) {
        bool ok;
        int productId = currentItem->data(Qt::UserRole).toInt(&ok);
        if(ok) {
            auto product = m_productManager->getProductById(productId);
            if (product && m_currentSale) {
                bool quantityOk;
                int quantity = QInputDialog::getInt(this, "输入数量", 
                                                    QString("为商品“%1”输入数量:").arg(product->getName()),
                                                    1, 1, product->getStockQuantity(), 1, &quantityOk);

                if (quantityOk) {
                    m_checkoutController->addItemToSale(product, quantity);
                    showSuccessMessage(QString("添加 %1 个“%2”到购物车").arg(quantity).arg(product->getName()));
                }
            }
        }
    }
}

void MainWindow::onApplyDiscount()
{
    if (!m_currentSale || m_currentSale->isEmpty()) {
        showErrorMessage("购物车为空，无法应用折扣");
        return;
    }
    
    bool ok;
    double discountPercent = QInputDialog::getDouble(this, "应用折扣", 
                                                   "请输入折扣百分比 (0-100):", 
                                                   0.0, 0.0, 100.0, 2, &ok);
    if (ok) {
        m_currentSale->setDiscountAmount(m_currentSale->getTotalAmount() * discountPercent / 100.0);
        updateCartDisplay();
        showSuccessMessage(QString("应用折扣: %1%%").arg(discountPercent));
    }
}

void MainWindow::onPrintReceipt()
{
    if (m_lastCompletedSale) {
        ReceiptPrinter printer;
        if(printer.printReceipt(*m_lastCompletedSale, m_lastCompletedSale->getItems())) {
            showSuccessMessage("小票已打印");
        } else {
            showErrorMessage("打印小票失败");
        }
    } else {
        showErrorMessage("没有可以打印的上一笔交易");
    }
}

void MainWindow::onRefreshRecommendations()
{
    if (m_currentSale && !m_currentSale->isEmpty()) {
        QList<int> productIds;
        for (auto* item : m_currentSale->getItems()) {
            productIds.append(item->getProduct()->getProductId());
        }
        auto recommendations = m_aiRecommender->getRecommendations(productIds);
        updateRecommendationDisplay(recommendations);
    } else {
        // If cart is empty, get popular recommendations
        auto recommendations = m_aiRecommender->getPopularRecommendations(5);
        updateRecommendationDisplay(recommendations);
    }
}

void MainWindow::onRecommendationAddToCart(int productId)
{
    auto product = m_productManager->getProductById(productId);
    if (product && m_currentSale) {
        m_checkoutController->addItemToSale(product, 1);
        showSuccessMessage(QString("添加推荐商品: %1").arg(product->getName()));
    }
}

void MainWindow::onRecommendationSelected()
{
    // This function is required to exist to fix a linker error,
    // but its functionality is handled by onRecommendationAddToCart.
    // A recommendation item widget is not selectable in the same way
    // a regular list item is; it's activated by clicking the "Add to Cart" button.
}


void MainWindow::onProcessPayment()
{
    qDebug() << "onProcessPayment triggered";
    if (!m_currentSale || m_currentSale->isEmpty()) {
        showErrorMessage("购物车为空，无法结算");
        return;
    }

    PaymentDialog dialog(m_currentSale->getFinalAmount(), m_checkoutController.get(), this);
    if (dialog.exec() == QDialog::Accepted) {
        if (m_checkoutController->completeSale()) {
            showSuccessMessage("支付成功，交易完成！");
            // Defer starting a new sale to avoid use-after-free issues
            // with objects tied to the dialog that just closed.
            QTimer::singleShot(0, this, &MainWindow::onNewSale);
        } else {
            showErrorMessage("完成销售失败，请检查日志");
        }
    } else {
        showErrorMessage("支付已取消");
    }
}

void MainWindow::updateCartDisplay()
{
    qDebug() << "MainWindow::updateCartDisplay called, m_productTable:" << ui->cartTableView << ", m_totalLabel:" << ui->totalValueLabel;
    
    if (!ui->cartTableView || !m_productModel) {
        qDebug() << "MainWindow::updateCartDisplay cartTableView or m_productModel is null, skipping";
        return;
    }
    
    m_productModel->clear();
    m_productModel->setHorizontalHeaderLabels({"商品名称", "数量", "单价", "小计", "操作"});
    
    if (m_currentSale) {
        for (auto* item : m_currentSale->getItems()) {
            if (item && item->getProduct()) {
                QList<QStandardItem *> rowItems;
                auto nameItem = new QStandardItem(item->getProduct()->getName());
                nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);

                auto quantityItem = new QStandardItem(QString::number(item->getQuantity()));
                
                auto priceItem = new QStandardItem(QString("¥%1").arg(item->getProduct()->getPrice(), 0, 'f', 2));
                priceItem->setFlags(priceItem->flags() & ~Qt::ItemIsEditable);
                
                auto subtotalItem = new QStandardItem(QString("¥%1").arg(item->getSubtotal(), 0, 'f', 2));
                subtotalItem->setFlags(subtotalItem->flags() & ~Qt::ItemIsEditable);

                auto actionItem = new QStandardItem();
                actionItem->setFlags(actionItem->flags() & ~Qt::ItemIsEditable);

                rowItems << nameItem << quantityItem << priceItem << subtotalItem << actionItem;
                m_productModel->appendRow(rowItems);
            }
        }
    }
    
    updateTotals();
}

void MainWindow::updateTotals()
{
    if (!m_currentSale) {
        if (ui->subtotalValueLabel) ui->subtotalValueLabel->setText("¥0.00");
        if (ui->discountValueLabel) ui->discountValueLabel->setText("¥0.00");
        if (ui->totalValueLabel) ui->totalValueLabel->setText("¥0.00");
        return;
    }

    if (ui->subtotalValueLabel) {
        ui->subtotalValueLabel->setText(QString("¥%1").arg(m_currentSale->getTotalAmount(), 0, 'f', 2));
    }
    if (ui->discountValueLabel) {
        ui->discountValueLabel->setText(QString("-¥%1").arg(m_currentSale->getDiscountAmount(), 0, 'f', 2));
    }
    if (ui->totalValueLabel) {
        ui->totalValueLabel->setText(QString("¥%1").arg(m_currentSale->getFinalAmount(), 0, 'f', 2));
    }
}

void MainWindow::onProductDoubleClicked(const QModelIndex& index)
{
    if (!index.isValid() || !m_currentSale) {
        return;
    }

    int row = index.row();
    if (row >= 0 && row < m_currentSale->getItems().size()) {
        SaleItem* item = m_currentSale->getItems().at(row);
        Product* product = item->getProduct();
        
        bool ok;
        int newQuantity = QInputDialog::getInt(this, "修改商品数量",
            QString("请输入“%1”的新数量:").arg(product->getName()),
            item->getQuantity(), 1, product->getStockQuantity(), 1, &ok);

        if (ok && newQuantity != item->getQuantity()) {
            m_checkoutController->updateItemQuantity(product->getProductId(), newQuantity);
        }
    }
}

void MainWindow::updateProductDisplay(const QList<Product*>& products)
{
    qDebug() << "updateProductDisplay called with products count:" << products.size();
    if (!ui->productListWidget) {
        qDebug() << "updateProductDisplay: productListWidget is null";
        return;
    }
    
    ui->productListWidget->clear();
    
    for (auto* product : products) {
        if (product) {
            QListWidgetItem* item = new QListWidgetItem(product->getName(), ui->productListWidget);
            item->setData(Qt::UserRole, product->getProductId());
        }
    }
}

void MainWindow::showErrorMessage(const QString& message)
{
    if (ui->statusbar) {
        ui->statusbar->setStyleSheet("background-color: #dc3545; color: white; font-weight: bold;");
        ui->statusbar->showMessage(QString("错误: %1").arg(message), 5000);
    }
    
    // QMessageBox::warning(this, "错误", message);
}

void MainWindow::showSuccessMessage(const QString& message)
{
    qDebug() << "MainWindow::showSuccessMessage called:" << message;
    if (ui->statusbar) {
        ui->statusbar->setStyleSheet("background-color: #28a745; color: white; font-weight: bold;");
        ui->statusbar->showMessage(message, 3000);
    }
}

void MainWindow::onRefreshProducts()
{
    m_productManager->getAllProducts();
    showSuccessMessage("正在刷新商品列表...");
}

// 占位符实现（其他槽函数）
void MainWindow::onItemQuantityChanged(QStandardItem *item)
{
    if (!m_currentSale || !item || item->column() != 1) {
        return; 
    }

    bool ok;
    int newQuantity = item->text().toInt(&ok);
    int row = item->row();
    SaleItem* saleItem = m_currentSale->getItems().at(row);

    if (!ok || newQuantity < 0) {
        // Revert invalid edits without triggering a full refresh
        QSignalBlocker blocker(m_productModel);
        item->setText(QString::number(saleItem->getQuantity()));
        return;
    }

    // This directly updates the model, which triggers signals that bubble up.
    m_checkoutController->updateItemQuantity(saleItem->getProduct()->getProductId(), newQuantity);
    
    // We only need to update totals, as the view itself is already edited.
    updateTotals();
}

void MainWindow::onRemoveItem()
{
    // This slot is now obsolete and replaced by onRemoveItemFromCart.
    // The button is hidden in initializeUI.
}

void MainWindow::onRemoveItemFromCart(const QModelIndex &index)
{
    if (!index.isValid() || !m_currentSale) {
        return;
    }

    int row = index.row();
    if (row >= 0 && row < m_currentSale->getItems().size()) {
        SaleItem* item = m_currentSale->getItems().at(row);
        m_checkoutController->removeItemFromSale(item->getProduct()->getProductId());
        showSuccessMessage("商品已移除");
    }
}

void MainWindow::onClearSale()
{
    qDebug() << "onClearSale triggered";
    if (m_currentSale) {
        m_checkoutController->cancelSale();
        updateCartDisplay();
        if (ui->recommendationListWidget) {
            ui->recommendationListWidget->clear();
        }
        showSuccessMessage("购物车已清空");
    }
}

void MainWindow::onManageProducts()
{
    ProductManagementDialog dialog(m_productManager.get(), this);
    dialog.exec();
    // Refresh product list in main window after dialog is closed
    onRefreshProducts();
}

void MainWindow::onRecommendationsUpdated(const QList<int>& productIds)
{
    qDebug() << "onRecommendationsUpdated triggered, productIds:" << productIds;
    updateRecommendationDisplay(productIds);
}

void MainWindow::updateRecommendationDisplay()
{
    // Update with popular items if the cart is empty
    if (m_currentSale == nullptr || m_currentSale->isEmpty()) {
        updateRecommendationDisplay(m_aiRecommender->getPopularRecommendations(5));
    } else {
        // This case will be triggered when a sale is updated.
        // The list of product IDs should be passed from onSaleUpdated -> onRecommendationsUpdated
    }
}

void MainWindow::updateRecommendationDisplay(const QList<int>& productIds)
{
    if (!ui->recommendationListWidget) {
        qDebug() << "updateRecommendationDisplay: recommendationListWidget is null";
        return;
    }
    
    ui->recommendationListWidget->clear();
    
    for (int productId : productIds) {
        auto product = m_productManager->getProductById(productId);
        if (product) {
            auto* itemWidget = new RecommendationItemWidget(product);
            auto* listItem = new QListWidgetItem(ui->recommendationListWidget);
            listItem->setSizeHint(itemWidget->sizeHint());
            ui->recommendationListWidget->addItem(listItem);
            ui->recommendationListWidget->setItemWidget(listItem, itemWidget);

            connect(itemWidget, &RecommendationItemWidget::addToCartClicked, this, &MainWindow::onRecommendationAddToCart);
        }
    }
}

void MainWindow::onShowStatistics()
{
    auto* dialog = new SalesReportDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}

void MainWindow::onShowSettings()
{
    QMessageBox::information(this, "系统设置", "此功能正在开发中。");
}

void MainWindow::onAbout() {
    QMessageBox::about(this, "关于", "智能超市收银系统 v1.0\n基于C++/Qt开发\n集成条码识别和AI推荐功能");
}

void MainWindow::onSaleCompleted(Sale* sale)
{
    if (!sale) return;

    // Delete the previous last completed sale if it exists
    if (m_lastCompletedSale) {
        delete m_lastCompletedSale;
        m_lastCompletedSale = nullptr;
    }

    // Create a deep copy of the completed sale.
    // This requires Sale to have a proper copy constructor.
    m_lastCompletedSale = new Sale(*sale);
    qDebug() << "Last completed sale has been stored. Transaction ID:" << m_lastCompletedSale->getTransactionId();
}

void MainWindow::updateTime()
{
    if (ui->currentTimeLabel) {
        ui->currentTimeLabel->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_isClosing) {
        event->accept();
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认退出", "确定要退出系统吗？",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        m_isClosing = true;
        // Perform any cleanup here if necessary
        qDebug() << "Application is closing.";
        event->accept();
        QApplication::quit(); // Ensure the application quits properly
    } else {
        event->ignore();
    }
}
