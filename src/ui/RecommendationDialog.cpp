#include "RecommendationDialog.h"
#include "../models/Product.h"
#include <QStandardItem>
#include <QDebug>
#include <QCheckBox>

RecommendationDialog::RecommendationDialog(QWidget *parent)
    : QDialog(parent)
    , m_mainLayout(nullptr)
    , m_titleLabel(nullptr)
    , m_recommendationTable(nullptr)
    , m_buttonLayout(nullptr)
    , m_addSelectedButton(nullptr)
    , m_addAllButton(nullptr)
    , m_closeButton(nullptr)
    , m_model(nullptr)
{
    setWindowTitle("AI商品推荐");
    setModal(true);
    resize(600, 400);
    
    qDebug() << "RecommendationDialog 构造函数开始";
    qDebug() << "创建推荐对话框实例，父窗口:" << (parent ? "有效" : "空");
    qDebug() << "设置窗口标题:" << windowTitle();
    qDebug() << "设置窗口大小:" << size();
    
    initializeUI();
    setupStyleSheet();
    
    qDebug() << "RecommendationDialog 初始化完成，窗口准备就绪";
}

RecommendationDialog::~RecommendationDialog()
{
    qDebug() << "RecommendationDialog 析构";
}

void RecommendationDialog::initializeUI()
{
    // 创建主布局
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(15);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);

    // 创建标题标签
    m_titleLabel = new QLabel("AI为您推荐以下商品：", this);
    m_titleLabel->setObjectName("titleLabel");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_mainLayout->addWidget(m_titleLabel);

    // 创建推荐商品表格
    m_recommendationTable = new QTableView(this);
    m_recommendationTable->setObjectName("recommendationTable");
    m_recommendationTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_recommendationTable->setAlternatingRowColors(true);
    m_recommendationTable->setSortingEnabled(false);
    m_recommendationTable->horizontalHeader()->setStretchLastSection(true);
    m_recommendationTable->verticalHeader()->setVisible(false);
    m_mainLayout->addWidget(m_recommendationTable);

    // 创建数据模型
    m_model = new QStandardItemModel(0, ColumnCount, this);
    m_model->setHeaderData(CheckboxColumn, Qt::Horizontal, "选择");
    m_model->setHeaderData(ProductNameColumn, Qt::Horizontal, "商品名称");
    m_model->setHeaderData(PriceColumn, Qt::Horizontal, "价格");
    m_recommendationTable->setModel(m_model);

    // 设置列宽
    m_recommendationTable->setColumnWidth(CheckboxColumn, 80);
    m_recommendationTable->setColumnWidth(ProductNameColumn, 300);

    // 创建按钮布局
    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->setSpacing(10);

    // 创建按钮
    m_addSelectedButton = new QPushButton("添加选中商品", this);
    m_addSelectedButton->setObjectName("addSelectedButton");
    m_addSelectedButton->setMinimumHeight(35);

    m_addAllButton = new QPushButton("全部添加", this);
    m_addAllButton->setObjectName("addAllButton");
    m_addAllButton->setMinimumHeight(35);

    m_closeButton = new QPushButton("关闭", this);
    m_closeButton->setObjectName("closeButton");
    m_closeButton->setMinimumHeight(35);

    // 添加按钮到布局
    m_buttonLayout->addWidget(m_addSelectedButton);
    m_buttonLayout->addWidget(m_addAllButton);
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_closeButton);

    m_mainLayout->addLayout(m_buttonLayout);

    // 连接信号和槽
    connect(m_addSelectedButton, &QPushButton::clicked, this, &RecommendationDialog::onAddSelectedItems);
    connect(m_addAllButton, &QPushButton::clicked, this, &RecommendationDialog::onAddAllItems);
    connect(m_closeButton, &QPushButton::clicked, this, &RecommendationDialog::onCloseDialog);
}

void RecommendationDialog::setupStyleSheet()
{
    setStyleSheet(R"(
        QDialog {
            background-color: #f5f5f5;
        }
        
        QLabel#titleLabel {
            font-size: 16px;
            font-weight: bold;
            color: #2c3e50;
            padding: 10px;
            background-color: #ecf0f1;
            border-radius: 5px;
        }
        
        QTableView#recommendationTable {
            background-color: white;
            border: 1px solid #bdc3c7;
            border-radius: 5px;
            gridline-color: #ecf0f1;
            selection-background-color: #3498db;
            selection-color: white;
        }
        
        QTableView#recommendationTable::item {
            padding: 8px;
            border-bottom: 1px solid #ecf0f1;
        }
        
        QHeaderView::section {
            background-color: #34495e;
            color: white;
            padding: 8px;
            border: none;
            font-weight: bold;
        }
        
        QPushButton {
            background-color: #3498db;
            color: white;
            border: none;
            border-radius: 5px;
            padding: 8px 16px;
            font-weight: bold;
        }
        
        QPushButton:hover {
            background-color: #2980b9;
        }
        
        QPushButton:pressed {
            background-color: #21618c;
        }
        
        QPushButton#closeButton {
            background-color: #95a5a6;
        }
        
        QPushButton#closeButton:hover {
            background-color: #7f8c8d;
        }
        
        QPushButton#addAllButton {
            background-color: #27ae60;
        }
        
        QPushButton#addAllButton:hover {
            background-color: #229954;
        }
    )");
}

void RecommendationDialog::setRecommendations(const QList<Product*>& products)
{
    qDebug() << "RecommendationDialog::setRecommendations 开始设置推荐商品列表";
    qDebug() << "接收到的商品数量:" << products.size();
    qDebug() << "推荐商品详细信息:";
    for (int i = 0; i < products.size(); ++i) {
        const Product* product = products.at(i);
        if (product) {
            qDebug() << QString("  推荐商品[%1]: ID=%2, 名称=%3, 价格=¥%4")
                        .arg(i + 1)
                        .arg(product->getProductId())
                        .arg(product->getName())
                        .arg(product->getPrice(), 0, 'f', 2);
        } else {
            qWarning() << QString("  推荐商品[%1]: 商品指针为空!").arg(i + 1);
        }
    }
    
    // 清空现有数据
    m_model->clear();
    m_model->setRowCount(0);
    m_model->setColumnCount(ColumnCount);
    
    // 重新设置表头
    m_model->setHeaderData(CheckboxColumn, Qt::Horizontal, "选择");
    m_model->setHeaderData(ProductNameColumn, Qt::Horizontal, "商品名称");
    m_model->setHeaderData(PriceColumn, Qt::Horizontal, "价格");
    
    // 添加商品数据
    for (int i = 0; i < products.size(); ++i) {
        Product* product = products.at(i);
        if (!product) {
            qWarning() << "第" << i << "个商品为空指针，跳过";
            continue;
        }
        
        // 创建新行
        QList<QStandardItem*> rowItems;
        
        // 复选框列
        QStandardItem* checkboxItem = new QStandardItem();
        checkboxItem->setCheckable(true);
        checkboxItem->setCheckState(Qt::Unchecked);
        checkboxItem->setData(product->getProductId(), Qt::UserRole); // 存储商品ID
        checkboxItem->setTextAlignment(Qt::AlignCenter);
        rowItems.append(checkboxItem);
        
        // 商品名称列
        QStandardItem* nameItem = new QStandardItem(product->getName());
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable); // 设置为只读
        nameItem->setToolTip(product->getDescription()); // 设置提示信息
        rowItems.append(nameItem);
        
        // 价格列
        QStandardItem* priceItem = new QStandardItem(QString("¥%1").arg(product->getPrice(), 0, 'f', 2));
        priceItem->setFlags(priceItem->flags() & ~Qt::ItemIsEditable); // 设置为只读
        priceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        rowItems.append(priceItem);
        
        // 添加行到模型
        m_model->appendRow(rowItems);
        
        qDebug() << "添加推荐商品:" << product->getName() << "价格:" << product->getPrice();
    }
    
    // 更新按钮状态
    bool hasProducts = products.size() > 0;
    m_addSelectedButton->setEnabled(hasProducts);
    m_addAllButton->setEnabled(hasProducts);
    
    qDebug() << "推荐商品列表设置完成";
}

void RecommendationDialog::onAddSelectedItems()
{
    qDebug() << "RecommendationDialog::onAddSelectedItems 用户点击'添加选中商品'按钮";
    qDebug() << "开始检查用户选择的商品...";
    
    QList<int> selectedIds = getSelectedProductIds();
    qDebug() << "获取到的选中商品ID数量:" << selectedIds.size();
    
    if (selectedIds.isEmpty()) {
        qDebug() << "没有选中任何商品，不发射信号";
        // 可以显示消息提示用户选择商品
        return;
    }
    
    qDebug() << "用户选中的商品ID详细列表:" << selectedIds;
    qDebug() << "即将发射 itemsSelectedForCart 信号，包含" << selectedIds.size() << "个商品ID";
    
    emit itemsSelectedForCart(selectedIds);
    qDebug() << "itemsSelectedForCart 信号已发射，商品ID:" << selectedIds;
    qDebug() << "关闭推荐对话框 (accept)";
    accept(); // 关闭对话框
}

void RecommendationDialog::onAddAllItems()
{
    qDebug() << "RecommendationDialog::onAddAllItems 用户点击'全部添加'按钮";
    qDebug() << "获取所有推荐商品ID...";
    
    QList<int> allIds = getAllProductIds();
    qDebug() << "获取到的所有商品ID数量:" << allIds.size();
    qDebug() << "所有商品ID详细列表:" << allIds;
    qDebug() << "即将发射 itemsSelectedForCart 信号，包含所有" << allIds.size() << "个商品";
    
    emit itemsSelectedForCart(allIds);
    qDebug() << "itemsSelectedForCart 信号已发射，包含所有商品ID:" << allIds;
    qDebug() << "关闭推荐对话框 (accept)";
    accept(); // 关闭对话框
}

void RecommendationDialog::onCloseDialog()
{
    qDebug() << "RecommendationDialog::onCloseDialog 关闭对话框";
    reject(); // 关闭对话框，不发射信号
}

QList<int> RecommendationDialog::getSelectedProductIds() const
{
    QList<int> selectedIds;
    
    for (int row = 0; row < m_model->rowCount(); ++row) {
        QStandardItem* checkboxItem = m_model->item(row, CheckboxColumn);
        if (checkboxItem && checkboxItem->checkState() == Qt::Checked) {
            int productId = checkboxItem->data(Qt::UserRole).toInt();
            selectedIds.append(productId);
        }
    }
    
    return selectedIds;
}

QList<int> RecommendationDialog::getAllProductIds() const
{
    QList<int> allIds;
    
    for (int row = 0; row < m_model->rowCount(); ++row) {
        QStandardItem* checkboxItem = m_model->item(row, CheckboxColumn);
        if (checkboxItem) {
            int productId = checkboxItem->data(Qt::UserRole).toInt();
            allIds.append(productId);
        }
    }
    
    return allIds;
} 