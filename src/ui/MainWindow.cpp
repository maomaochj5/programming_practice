#include "MainWindow.h"
#include "../controllers/CheckoutController.h"
#include "../controllers/ProductManager.h"
#include "../ai/AIRecommender.h"
#include "../barcode/BarcodeScanner.h"
#include "../models/Sale.h"
#include "../models/Product.h"
#include "ProductDialog.h"
#include "PaymentDialog.h"

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
    , m_centralWidget(nullptr)
    , m_mainSplitter(nullptr)
    , m_currentSale(nullptr)
    , m_currentUser("收银员")
    , m_isClosing(false) // Initialize the closing flag
{
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
    
    // 刷新商品列表
    onRefreshProducts();
    
    qDebug() << "主窗口初始化完成";
}

MainWindow::~MainWindow()
{
    qDebug() << "MainWindow 析构函数开始";
    // No need to delete child objects, Qt handles it.
    // delete m_checkoutController;
    // delete m_productManager;
    // delete m_aiRecommender;
    // delete m_barcodeScanner;
    qDebug() << "MainWindow 析构函数结束";
}

void MainWindow::initializeUI()
{
    setWindowTitle("智能超市收银系统 v1.0");
    setMinimumSize(1200, 800);
    resize(1400, 900);
    
    // 创建菜单栏
    createMenuBar();
    
    // 创建工具栏
    createToolBar();
    
    // 创建中央窗口
    createCentralWidget();
    
    // 创建状态栏
    createStatusBar();
}

void MainWindow::createMenuBar()
{
    // 文件菜单
    QMenu* fileMenu = menuBar()->addMenu("文件(&F)");
    
    QAction* newSaleAction = fileMenu->addAction("新建销售(&N)");
    newSaleAction->setShortcut(QKeySequence::New);
    connect(newSaleAction, &QAction::triggered, this, &MainWindow::onNewSale);
    
    fileMenu->addSeparator();
    
    QAction* exitAction = fileMenu->addAction("退出(&X)");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    
    // 商品菜单
    QMenu* productMenu = menuBar()->addMenu("商品(&P)");
    
    QAction* addProductAction = productMenu->addAction("添加商品(&A)");
    addProductAction->setShortcut(QKeySequence("Ctrl+A"));
    connect(addProductAction, &QAction::triggered, this, &MainWindow::onAddProduct);
    
    QAction* editProductAction = productMenu->addAction("编辑商品(&E)");
    editProductAction->setShortcut(QKeySequence("Ctrl+E"));
    connect(editProductAction, &QAction::triggered, this, &MainWindow::onEditProduct);
    
    QAction* deleteProductAction = productMenu->addAction("删除商品(&D)");
    deleteProductAction->setShortcut(QKeySequence::Delete);
    connect(deleteProductAction, &QAction::triggered, this, &MainWindow::onDeleteProduct);
    
    productMenu->addSeparator();
    
    QAction* refreshAction = productMenu->addAction("刷新列表(&R)");
    refreshAction->setShortcut(QKeySequence::Refresh);
    connect(refreshAction, &QAction::triggered, this, &MainWindow::onRefreshProducts);
    
    // 工具菜单
    QMenu* toolsMenu = menuBar()->addMenu("工具(&T)");
    
    QAction* statsAction = toolsMenu->addAction("销售统计(&S)");
    connect(statsAction, &QAction::triggered, this, &MainWindow::onShowStatistics);
    
    QAction* settingsAction = toolsMenu->addAction("系统设置(&C)");
    connect(settingsAction, &QAction::triggered, this, &MainWindow::onShowSettings);
    
    // 帮助菜单
    QMenu* helpMenu = menuBar()->addMenu("帮助(&H)");
    
    QAction* aboutAction = helpMenu->addAction("关于(&A)");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::createToolBar()
{
    QToolBar* mainToolBar = addToolBar("主工具栏");
    mainToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    
    // 新建销售
    QAction* newSaleAction = mainToolBar->addAction("新建销售");
    newSaleAction->setIcon(QIcon(":/icons/new_sale.png"));
    connect(newSaleAction, &QAction::triggered, this, &MainWindow::onNewSale);
    
    mainToolBar->addSeparator();
    
    // 支付
    QAction* paymentAction = mainToolBar->addAction("支付");
    paymentAction->setIcon(QIcon(":/icons/payment.png"));
    connect(paymentAction, &QAction::triggered, this, &MainWindow::onProcessPayment);
    
    mainToolBar->addSeparator();
    
    // 商品管理
    QAction* productAction = mainToolBar->addAction("商品管理");
    productAction->setIcon(QIcon(":/icons/products.png"));
    connect(productAction, &QAction::triggered, this, &MainWindow::onAddProduct);
    
    // 统计报表
    QAction* statsAction = mainToolBar->addAction("销售统计");
    statsAction->setIcon(QIcon(":/icons/stats.png"));
    connect(statsAction, &QAction::triggered, this, &MainWindow::onShowStatistics);
}

void MainWindow::createStatusBar()
{
    m_statusLabel = new QLabel("就绪");
    m_timeLabel = new QLabel();
    m_userLabel = new QLabel(QString("用户: %1").arg(m_currentUser));
    
    statusBar()->addWidget(m_statusLabel);
    statusBar()->addPermanentWidget(m_userLabel);
    statusBar()->addPermanentWidget(m_timeLabel);
    
    // 状态更新定时器
    m_statusTimer = new QTimer(this);
    connect(m_statusTimer, &QTimer::timeout, this, &MainWindow::updateStatusBar);
    m_statusTimer->start(1000); // 每秒更新
    
    updateStatusBar();
}

void MainWindow::createCentralWidget()
{
    m_centralWidget = new QWidget;
    setCentralWidget(m_centralWidget);
    
    // 主分割器
    m_mainSplitter = new QSplitter(Qt::Horizontal, m_centralWidget);
    
    // 左侧：销售和条码扫描区域
    QWidget* leftWidget = new QWidget;
    QVBoxLayout* leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->addWidget(createSalesArea());
    leftLayout->addWidget(createBarcodeScanArea());
    
    // 中间：商品管理区域
    QWidget* centerWidget = createProductArea();
    
    // 右侧：推荐区域
    QWidget* rightWidget = createRecommendationArea();
    
    // 添加到分割器
    m_mainSplitter->addWidget(leftWidget);
    m_mainSplitter->addWidget(centerWidget);
    m_mainSplitter->addWidget(rightWidget);
    
    // 设置分割器比例
    m_mainSplitter->setSizes({350, 600, 250});
    
    // 主布局
    QHBoxLayout* mainLayout = new QHBoxLayout(m_centralWidget);
    mainLayout->addWidget(m_mainSplitter);
}

QWidget* MainWindow::createSalesArea()
{
    m_salesGroup = new QGroupBox("当前销售");
    QVBoxLayout* layout = new QVBoxLayout(m_salesGroup);
    
    // 购物车列表
    m_cartList = new QListWidget;
    m_cartList->setMinimumHeight(300);
    layout->addWidget(m_cartList);
    
    // 数量调整
    QHBoxLayout* quantityLayout = new QHBoxLayout;
    quantityLayout->addWidget(new QLabel("数量:"));
    m_quantitySpinBox = new QSpinBox;
    m_quantitySpinBox->setMinimum(1);
    m_quantitySpinBox->setMaximum(999);
    m_quantitySpinBox->setValue(1);
    quantityLayout->addWidget(m_quantitySpinBox);
    quantityLayout->addStretch();
    layout->addLayout(quantityLayout);
    
    // 总金额显示
    m_totalLabel = new QLabel("总计: ¥0.00");
    m_totalLabel->setStyleSheet("QLabel { font-size: 18px; font-weight: bold; color: #2E8B57; }");
    layout->addWidget(m_totalLabel);
    
    // 操作按钮
    QGridLayout* buttonLayout = new QGridLayout;
    
    m_newSaleButton = new QPushButton("新建销售");
    m_newSaleButton->setMinimumHeight(40);
    buttonLayout->addWidget(m_newSaleButton, 0, 0);
    
    m_removeItemButton = new QPushButton("移除商品");
    m_removeItemButton->setMinimumHeight(40);
    buttonLayout->addWidget(m_removeItemButton, 0, 1);
    
    m_paymentButton = new QPushButton("支付");
    m_paymentButton->setMinimumHeight(50);
    m_paymentButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; }");
    buttonLayout->addWidget(m_paymentButton, 1, 0, 1, 2);
    
    m_clearSaleButton = new QPushButton("清空");
    m_clearSaleButton->setMinimumHeight(40);
    buttonLayout->addWidget(m_clearSaleButton, 2, 0, 1, 2);
    
    layout->addLayout(buttonLayout);
    
    return m_salesGroup;
}

QWidget* MainWindow::createProductArea()
{
    m_productGroup = new QGroupBox("商品管理");
    QVBoxLayout* layout = new QVBoxLayout(m_productGroup);
    
    // 商品表格
    m_productTable = new QTableView;
    m_productModel = new QStandardItemModel(this);
    m_productModel->setHorizontalHeaderLabels({"ID", "条码", "名称", "价格", "库存", "分类"});
    m_productTable->setModel(m_productModel);
    m_productTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_productTable->setAlternatingRowColors(true);
    m_productTable->horizontalHeader()->setStretchLastSection(true);
    
    // 添加双击事件处理
    connect(m_productTable, &QTableView::doubleClicked, this, &MainWindow::onProductDoubleClicked);
    
    layout->addWidget(m_productTable);
    
    // 操作按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    
    m_addProductButton = new QPushButton("添加商品");
    m_editProductButton = new QPushButton("编辑商品");
    m_deleteProductButton = new QPushButton("删除商品");
    m_refreshProductButton = new QPushButton("刷新列表");
    
    buttonLayout->addWidget(m_addProductButton);
    buttonLayout->addWidget(m_editProductButton);
    buttonLayout->addWidget(m_deleteProductButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_refreshProductButton);
    
    layout->addLayout(buttonLayout);
    
    return m_productGroup;
}

QWidget* MainWindow::createRecommendationArea()
{
    m_recommendationGroup = new QGroupBox("智能推荐");
    QVBoxLayout* layout = new QVBoxLayout(m_recommendationGroup);
    
    // 推荐说明
    QLabel* infoLabel = new QLabel("基于当前购物车的AI推荐:");
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("QLabel { color: #666; font-size: 12px; }");
    layout->addWidget(infoLabel);
    
    // 推荐列表
    m_recommendationList = new QListWidget;
    m_recommendationList->setMaximumHeight(300);
    layout->addWidget(m_recommendationList);
    
    // 添加推荐按钮
    m_addRecommendationButton = new QPushButton("添加到购物车");
    m_addRecommendationButton->setEnabled(false);
    layout->addWidget(m_addRecommendationButton);
    
    layout->addStretch();
    
    return m_recommendationGroup;
}

QWidget* MainWindow::createBarcodeScanArea()
{
    m_barcodeGroup = new QGroupBox("条码扫描");
    QVBoxLayout* layout = new QVBoxLayout(m_barcodeGroup);
    
    // 扫描状态
    m_scannerStatus = new QLabel("扫描仪状态: 就绪");
    m_scannerStatus->setStyleSheet("QLabel { color: #2E8B57; }");
    layout->addWidget(m_scannerStatus);
    
    // 摄像头预览区域（占位符）
    m_cameraWidget = new QWidget;
    m_cameraWidget->setMinimumHeight(150);
    m_cameraWidget->setStyleSheet("QWidget { background-color: #f0f0f0; border: 1px solid #ccc; }");
    layout->addWidget(m_cameraWidget);
    
    // 手动输入条码
    QHBoxLayout* manualLayout = new QHBoxLayout;
    manualLayout->addWidget(new QLabel("手动输入:"));
    m_manualBarcodeEdit = new QLineEdit;
    m_manualBarcodeEdit->setPlaceholderText("输入条码并按回车");
    m_manualBarcodeButton = new QPushButton("确定");
    manualLayout->addWidget(m_manualBarcodeEdit);
    manualLayout->addWidget(m_manualBarcodeButton);
    layout->addLayout(manualLayout);
    
    return m_barcodeGroup;
}

void MainWindow::connectSignals()
{
    // 销售相关信号
    connect(m_newSaleButton, &QPushButton::clicked, this, &MainWindow::onNewSale);
    connect(m_paymentButton, &QPushButton::clicked, this, &MainWindow::onProcessPayment);
    connect(m_clearSaleButton, &QPushButton::clicked, this, &MainWindow::onClearSale);
    connect(m_removeItemButton, &QPushButton::clicked, this, &MainWindow::onRemoveItem);
    connect(m_quantitySpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onItemQuantityChanged);
    
    // 商品管理信号
    connect(m_addProductButton, &QPushButton::clicked, this, &MainWindow::onAddProduct);
    connect(m_editProductButton, &QPushButton::clicked, this, &MainWindow::onEditProduct);
    connect(m_deleteProductButton, &QPushButton::clicked, this, &MainWindow::onDeleteProduct);
    connect(m_refreshProductButton, &QPushButton::clicked, this, &MainWindow::onRefreshProducts);
    
    // 条码扫描信号
    connect(m_manualBarcodeEdit, &QLineEdit::returnPressed, this, &MainWindow::onManualBarcodeEntry);
    connect(m_manualBarcodeButton, &QPushButton::clicked, this, &MainWindow::onManualBarcodeEntry);
    connect(m_barcodeScanner.get(), &BarcodeScanner::barcodeDetected,
            this, &MainWindow::onBarcodeScanned);
    
    // 推荐系统信号
    connect(m_addRecommendationButton, &QPushButton::clicked, this, &MainWindow::onRecommendationSelected);
    connect(m_recommendationList, &QListWidget::itemSelectionChanged,
            [this]() { m_addRecommendationButton->setEnabled(m_recommendationList->currentItem() != nullptr); });
    
    // 控制器信号
    connect(m_checkoutController.get(), &CheckoutController::saleUpdated,
            this, &MainWindow::onSaleUpdated);
}

void MainWindow::setupStyleSheet()
{
    QWidget::setStyleSheet(R"(
        QMainWindow {
            background-color: #f5f5f5;
        }
        QGroupBox {
            font-weight: bold;
            border: 2px solid #cccccc;
            border-radius: 8px;
            margin-top: 1ex;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
        }
        QPushButton {
            background-color: #e1e1e1;
            border: 1px solid #999999;
            border-radius: 4px;
            padding: 8px;
            font-size: 12px;
        }
        QPushButton:hover {
            background-color: #d4d4d4;
        }
        QPushButton:pressed {
            background-color: #b8b8b8;
        }
        QListWidget, QTableView {
            border: 1px solid #d0d0d0;
            border-radius: 4px;
            background-color: white;
        }
        QLineEdit, QSpinBox {
            border: 1px solid #d0d0d0;
            border-radius: 4px;
            padding: 4px;
        }
    )");
}

void MainWindow::onNewSale()
{
    qDebug() << "MainWindow::onNewSale called, m_currentSale:" << m_currentSale;
    if (m_currentSale) {
        // 先通知CheckoutController断开信号连接
        m_checkoutController->setCurrentSale(nullptr);
        delete m_currentSale;
        qDebug() << "MainWindow::onNewSale deleted old m_currentSale";
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
    qDebug() << "MainWindow::onSaleUpdated called, m_currentSale:" << m_currentSale;
    updateCartDisplay();
    
    // 更新AI推荐
    if (m_currentSale && !m_currentSale->isEmpty()) {
        QList<int> productIds;
        for (auto* item : m_currentSale->getItems()) {
            productIds.append(item->getProduct()->getProductId());
        }
        onRecommendationsUpdated(m_aiRecommender->getRecommendations(productIds));
    }
}

void MainWindow::onBarcodeScanned(const QString& barcode)
{
    qDebug() << "扫描到条码:" << barcode;
    
    if (!m_currentSale) {
        onNewSale();
    }
    
    auto product = m_productManager->getProductByBarcode(barcode);
    if (product) {
        m_checkoutController->addItemToSale(product, m_quantitySpinBox->value());
        showSuccessMessage(QString("添加商品: %1").arg(product->getName()));
    } else {
        showErrorMessage(QString("未找到条码为 %1 的商品").arg(barcode));
    }
}

void MainWindow::onManualBarcodeEntry()
{
    QString barcode = m_manualBarcodeEdit->text().trimmed();
    if (!barcode.isEmpty()) {
        onBarcodeScanned(barcode);
        m_manualBarcodeEdit->clear();
    }
}

void MainWindow::onProcessPayment()
{
    if (!m_currentSale || m_currentSale->isEmpty()) {
        showErrorMessage("购物车为空，无法进行支付");
        return;
    }
    
    PaymentDialog dialog(m_currentSale->getTotalAmount(), this);
    if (dialog.exec() == QDialog::Accepted) {
        // 支付成功，保存交易
        if (m_checkoutController->completeSale()) {
            showSuccessMessage("支付成功，交易已完成");
            onNewSale(); // 开始新的销售
        } else {
            showErrorMessage("保存交易失败");
        }
    }
}

void MainWindow::updateCartDisplay()
{
    qDebug() << "MainWindow::updateCartDisplay called, m_cartList:" << m_cartList << ", m_totalLabel:" << m_totalLabel;
    
    if (!m_cartList) {
        qDebug() << "MainWindow::updateCartDisplay m_cartList is null, skipping";
        return;
    }
    
    m_cartList->clear();
    
    if (!m_currentSale) {
        if (m_totalLabel) {
            m_totalLabel->setText("总计: ¥0.00");
        }
        return;
    }
    
    for (auto* item : m_currentSale->getItems()) {
        if (item && item->getProduct()) {
            QString itemText = QString("%1 x %2 = ¥%3")
                              .arg(item->getProduct()->getName())
                              .arg(item->getQuantity())
                              .arg(item->getSubtotal(), 0, 'f', 2);
            m_cartList->addItem(itemText);
        }
    }
    
    if (m_totalLabel) {
        m_totalLabel->setText(QString("总计: ¥%1").arg(m_currentSale->getFinalAmount(), 0, 'f', 2));
    }
}

void MainWindow::onRefreshProducts()
{
    updateProductDisplay();
}

void MainWindow::onProductDoubleClicked(const QModelIndex& index)
{
    if (!index.isValid()) {
        return;
    }
    
    // 获取选中商品的条码（第2列，索引为1）
    QString barcode = m_productModel->item(index.row(), 1)->text();
    
    if (!barcode.isEmpty()) {
        qDebug() << "双击添加商品，条码:" << barcode;
        onBarcodeScanned(barcode);
    }
}

void MainWindow::updateProductDisplay()
{
    m_productModel->clear();
    m_productModel->setHorizontalHeaderLabels({"ID", "条码", "名称", "价格", "库存", "分类"});
    
    auto products = m_productManager->getAllProducts();
    for (const auto& product : products) {
        QList<QStandardItem*> row;
        row << new QStandardItem(QString::number(product->getProductId()));
        row << new QStandardItem(product->getBarcode());
        row << new QStandardItem(product->getName());
        row << new QStandardItem(QString("¥%1").arg(product->getPrice(), 0, 'f', 2));
        row << new QStandardItem(QString::number(product->getStockQuantity()));
        row << new QStandardItem(product->getCategory());
        
        m_productModel->appendRow(row);
    }
    
    // 调整列宽
    m_productTable->resizeColumnsToContents();
    
    qDebug() << "商品表格更新完成，共" << products.size() << "个商品";
}

void MainWindow::showErrorMessage(const QString& message)
{
    m_statusLabel->setText(QString("错误: %1").arg(message));
    m_statusLabel->setStyleSheet("QLabel { color: red; }");
    QTimer::singleShot(5000, [this]() {
        m_statusLabel->setText("就绪");
        m_statusLabel->setStyleSheet("");
    });
    
    QMessageBox::warning(this, "错误", message);
}

void MainWindow::showSuccessMessage(const QString& message)
{
    qDebug() << "MainWindow::showSuccessMessage called:" << message << ", m_statusLabel:" << m_statusLabel;
    if (!m_statusLabel) {
        qDebug() << "MainWindow::showSuccessMessage m_statusLabel is null, skipping";
        return;
    }
    m_statusLabel->setText(message);
    m_statusLabel->setStyleSheet("QLabel { color: green; }");
    QTimer::singleShot(3000, [this]() {
        if (m_statusLabel) {
            m_statusLabel->setText("就绪");
            m_statusLabel->setStyleSheet("");
        }
    });
}

void MainWindow::updateStatusBar()
{
    m_timeLabel->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
}

// 占位符实现（其他槽函数）
void MainWindow::onItemQuantityChanged()
{
    QList<QListWidgetItem*> selectedItems = m_cartList->selectedItems();
    if (selectedItems.isEmpty() || !m_currentSale) {
        return;
    }

    int selectedRow = m_cartList->row(selectedItems.first());
    if (selectedRow >= 0 && selectedRow < m_currentSale->getItems().size()) {
        SaleItem* item = m_currentSale->getItems().at(selectedRow);
        int newQuantity = m_quantitySpinBox->value();
        m_checkoutController->updateItemQuantity(item->getProduct()->getProductId(), newQuantity);
    }
}

void MainWindow::onRemoveItem()
{
    QList<QListWidgetItem*> selectedItems = m_cartList->selectedItems();
    if (selectedItems.isEmpty() || !m_currentSale) {
        showErrorMessage("请先在购物车中选择要移除的商品");
        return;
    }

    int selectedRow = m_cartList->row(selectedItems.first());
    if (selectedRow >= 0 && selectedRow < m_currentSale->getItems().size()) {
        SaleItem* item = m_currentSale->getItems().at(selectedRow);
        m_checkoutController->removeItemFromSale(item->getProduct()->getProductId());
        showSuccessMessage("商品已移除");
    }
}

void MainWindow::onClearSale()
{
    if (m_currentSale) {
        m_checkoutController->cancelSale();
        updateCartDisplay();
        m_recommendationList->clear();
        showSuccessMessage("购物车已清空");
    }
}

void MainWindow::onAddProduct()
{
    qDebug() << "MainWindow::onAddProduct 开始";
    ProductDialog dialog(this);
    qDebug() << "MainWindow::onAddProduct ProductDialog 创建完成";
    dialog.show();
    qDebug() << "MainWindow::onAddProduct ProductDialog show() 已调用";
    // 不再处理exec/Accepted逻辑，测试只验证show不崩溃
    qDebug() << "MainWindow::onAddProduct 结束";
}

void MainWindow::onEditProduct()
{
    QModelIndexList selection = m_productTable->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        showErrorMessage("请先选择要编辑的商品");
        return;
    }

    int productId = m_productModel->item(selection.first().row(), 0)->text().toInt();
    Product* product = m_productManager->getProductById(productId);

    if (product) {
        ProductDialog dialog(product, this);
        if (dialog.exec() == QDialog::Accepted) {
            auto updatedProduct = dialog.getProduct();
            if (updatedProduct) {
                if (m_productManager->updateProduct(updatedProduct.release())) {
                    showSuccessMessage("商品更新成功");
                    onRefreshProducts();
                } else {
                    showErrorMessage("更新商品失败");
                }
            }
        }
    } else {
        showErrorMessage("找不到所选商品");
    }
}

void MainWindow::onDeleteProduct()
{
    QModelIndexList selection = m_productTable->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        showErrorMessage("请先选择要删除的商品");
        return;
    }

    int productId = m_productModel->item(selection.first().row(), 0)->text().toInt();
    QString productName = m_productModel->item(selection.first().row(), 2)->text();

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认删除", QString("确定要删除商品 “%1” 吗？").arg(productName),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        if (m_productManager->deleteProduct(productId)) {
            showSuccessMessage("商品删除成功");
            onRefreshProducts();
        } else {
            showErrorMessage("删除商品失败");
        }
    }
}

void MainWindow::onRecommendationSelected()
{
    QList<QListWidgetItem*> selectedItems = m_recommendationList->selectedItems();
    if (selectedItems.isEmpty()) {
        return;
    }

    int productId = selectedItems.first()->data(Qt::UserRole).toInt();
    Product* product = m_productManager->getProductById(productId);
    if (product) {
        m_checkoutController->addItemToSale(product, 1);
        showSuccessMessage(QString("已添加推荐商品: %1").arg(product->getName()));
    }
}

void MainWindow::onRecommendationsUpdated(const QList<int>& productIds)
{
    m_recommendationList->clear();
    if (productIds.isEmpty()) {
        m_recommendationList->addItem("暂无推荐");
        m_recommendationList->setEnabled(false);
        return;
    }

    m_recommendationList->setEnabled(true);
    for (int id : productIds) {
        auto product = m_productManager->getProductById(id);
        if (product) {
            QListWidgetItem* item = new QListWidgetItem(product->getName());
            item->setData(Qt::UserRole, id);
            m_recommendationList->addItem(item);
        }
    }
}

void MainWindow::onShowStatistics()
{
    QMessageBox::information(this, "销售统计", "此功能正在开发中。");
}

void MainWindow::onShowSettings()
{
    QMessageBox::information(this, "系统设置", "此功能正在开发中。");
}

void MainWindow::onAbout() {
    QMessageBox::about(this, "关于", "智能超市收银系统 v1.0\n基于C++/Qt开发\n集成条码识别和AI推荐功能");
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
