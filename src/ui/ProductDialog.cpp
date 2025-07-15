#include "ProductDialog.h"
#include <QRegularExpression>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QRandomGenerator>
#include <QDebug> // Added for qDebug

ProductDialog::ProductDialog(QWidget *parent)
    : QDialog(parent), m_editMode(false)
{
    qDebug() << "ProductDialog 构造(无参数), parent指针:" << parent;
    setWindowTitle(tr("添加商品"));
    setupUI();
    setupValidators();
    setupConnections();
    validateInput(); // 初始验证
    resize(600, 700);
}

ProductDialog::ProductDialog(const Product* product, QWidget *parent)
    : QDialog(parent)
{
    qDebug() << "ProductDialog 构造(有product), product指针:" << product;
    setWindowTitle(tr("编辑商品"));
    setupUI();
    setupValidators();
    setupConnections();
    if (product) {
        setProduct(product);
    }
    validateInput(); // 初始验证
    resize(600, 700);
}

ProductDialog::~ProductDialog()
{
    qDebug() << "ProductDialog 析构";
}

void ProductDialog::setupUI()
{
    qDebug() << "ProductDialog::setupUI 开始";
    auto *mainLayout = new QVBoxLayout(this);

    // 基本信息组
    auto *basicGroup = new QGroupBox(tr("基本信息"), this);
    auto *basicLayout = new QFormLayout(basicGroup);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText(tr("请输入商品名称"));
    basicLayout->addRow(tr("商品名称 *:"), m_nameEdit);

    m_barcodeEdit = new QLineEdit(this);
    m_barcodeEdit->setPlaceholderText(tr("请输入或生成条形码"));
    auto *barcodeLayout = new QHBoxLayout();
    barcodeLayout->addWidget(m_barcodeEdit);
    m_generateBarcodeButton = new QPushButton(tr("生成"), this);
    m_generateBarcodeButton->setMaximumWidth(80);
    barcodeLayout->addWidget(m_generateBarcodeButton);
    basicLayout->addRow(tr("条形码 *:"), barcodeLayout);

    m_categoryCombo = new QComboBox(this);
    m_categoryCombo->addItems({tr("食品"), tr("饮料"), tr("日用品"), tr("服装"), tr("电子产品"), tr("图书"), tr("其他")});
    m_categoryCombo->setEditable(true);
    basicLayout->addRow(tr("商品类别:"), m_categoryCombo);

    m_supplierEdit = new QLineEdit(this);
    m_supplierEdit->setPlaceholderText(tr("请输入供应商名称"));
    basicLayout->addRow(tr("供应商:"), m_supplierEdit);

    mainLayout->addWidget(basicGroup);

    // 价格和库存组
    auto *priceStockGroup = new QGroupBox(tr("价格与库存"), this);
    auto *priceStockLayout = new QFormLayout(priceStockGroup);

    m_priceSpinBox = new QDoubleSpinBox(this);
    m_priceSpinBox->setRange(0.01, 999999.99);
    m_priceSpinBox->setDecimals(2);
    m_priceSpinBox->setSuffix(" 元");
    priceStockLayout->addRow(tr("单价 *:"), m_priceSpinBox);

    m_stockSpinBox = new QSpinBox(this);
    m_stockSpinBox->setRange(0, 999999);
    m_stockSpinBox->setSuffix(" 件");
    priceStockLayout->addRow(tr("当前库存:"), m_stockSpinBox);

    m_minStockSpinBox = new QSpinBox(this);
    m_minStockSpinBox->setRange(0, 9999);
    m_minStockSpinBox->setSuffix(" 件");
    m_minStockSpinBox->setValue(10);
    priceStockLayout->addRow(tr("最低库存:"), m_minStockSpinBox);

    mainLayout->addWidget(priceStockGroup);

    // 图片组
    auto *imageGroup = new QGroupBox(tr("商品图片"), this);
    auto *imageLayout = new QVBoxLayout(imageGroup);

    auto *imageSelectLayout = new QHBoxLayout();
    m_imagePathEdit = new QLineEdit(this);
    m_imagePathEdit->setPlaceholderText(tr("请选择商品图片"));
    m_imagePathEdit->setReadOnly(true);
    imageSelectLayout->addWidget(m_imagePathEdit);

    m_selectImageButton = new QPushButton(tr("选择图片"), this);
    imageSelectLayout->addWidget(m_selectImageButton);

    imageLayout->addLayout(imageSelectLayout);

    m_imagePreview = new QLabel(this);
    m_imagePreview->setFixedSize(200, 150);
    m_imagePreview->setStyleSheet("border: 1px solid gray;");
    m_imagePreview->setAlignment(Qt::AlignCenter);
    m_imagePreview->setText(tr("无图片"));
    m_imagePreview->setScaledContents(true);
    imageLayout->addWidget(m_imagePreview, 0, Qt::AlignCenter);

    mainLayout->addWidget(imageGroup);

    // 描述组
    auto *descGroup = new QGroupBox(tr("商品描述"), this);
    auto *descLayout = new QVBoxLayout(descGroup);

    m_descriptionEdit = new QTextEdit(this);
    m_descriptionEdit->setPlaceholderText(tr("请输入商品描述（可选）"));
    m_descriptionEdit->setMaximumHeight(100);
    descLayout->addWidget(m_descriptionEdit);

    mainLayout->addWidget(descGroup);

    // 按钮组
    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_cancelButton = new QPushButton(tr("取消"), this);
    buttonLayout->addWidget(m_cancelButton);

    m_okButton = new QPushButton(tr("确定"), this);
    m_okButton->setDefault(true);
    buttonLayout->addWidget(m_okButton);

    mainLayout->addLayout(buttonLayout);
    qDebug() << "ProductDialog::setupUI 完成";
}

void ProductDialog::setupValidators()
{
    // 商品名称验证器（不能为空，长度限制）
    m_nameEdit->setMaxLength(100);

    // 条形码验证器（只允许数字，长度8-14位）
    auto *barcodeValidator = new QRegularExpressionValidator(
        QRegularExpression("^[0-9]{8,14}$"), this);
    m_barcodeEdit->setValidator(barcodeValidator);
    m_barcodeEdit->setMaxLength(14);
}

void ProductDialog::setupConnections()
{
    connect(m_okButton, &QPushButton::clicked, this, &ProductDialog::onAccept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_selectImageButton, &QPushButton::clicked, this, &ProductDialog::selectImage);
    connect(m_generateBarcodeButton, &QPushButton::clicked, this, &ProductDialog::generateBarcode);

    // 输入验证
    connect(m_nameEdit, &QLineEdit::textChanged, this, &ProductDialog::validateInput);
    connect(m_barcodeEdit, &QLineEdit::textChanged, this, &ProductDialog::validateInput);
    connect(m_priceSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ProductDialog::validateInput);

    // 图片路径变化时更新预览
    connect(m_imagePathEdit, &QLineEdit::textChanged, this, &ProductDialog::updateImagePreview);
}

void ProductDialog::selectImage()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("选择商品图片"),
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
        tr("图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)")
    );

    if (!fileName.isEmpty()) {
        m_imagePathEdit->setText(fileName);
    }
}

void ProductDialog::generateBarcode()
{
    // 生成一个简单的13位条形码（模拟EAN-13格式）
    QString barcode = "69"; // 中国商品前缀
    
    // 添加10位随机数字
    for (int i = 0; i < 10; ++i) {
        barcode += QString::number(QRandomGenerator::global()->bounded(10));
    }
    
    // 简单的校验位计算（实际EAN-13有复杂的校验算法）
    int sum = 0;
    for (int i = 0; i < 12; ++i) {
        int digit = barcode.at(i).digitValue();
        sum += (i % 2 == 0) ? digit : digit * 3;
    }
    int checkDigit = (10 - (sum % 10)) % 10;
    barcode += QString::number(checkDigit);
    
    m_barcodeEdit->setText(barcode);
}

void ProductDialog::validateInput()
{
    bool valid = true;
    QString tooltip;

    // 检查必填字段
    if (m_nameEdit->text().trimmed().isEmpty()) {
        valid = false;
        tooltip += tr("商品名称不能为空\n");
    }

    if (m_barcodeEdit->text().trimmed().isEmpty()) {
        valid = false;
        tooltip += tr("条形码不能为空\n");
    } else if (!m_barcodeEdit->hasAcceptableInput()) {
        valid = false;
        tooltip += tr("条形码格式不正确（应为8-14位数字）\n");
    }

    if (m_priceSpinBox->value() <= 0) {
        valid = false;
        tooltip += tr("商品价格必须大于0\n");
    }

    m_okButton->setEnabled(valid);
    if (!valid) {
        m_okButton->setToolTip(tooltip.trimmed());
    } else {
        m_okButton->setToolTip("");
    }
}

void ProductDialog::updateImagePreview()
{
    QString imagePath = m_imagePathEdit->text();
    if (imagePath.isEmpty()) {
        m_imagePreview->setText(tr("无图片"));
        m_imagePreview->setPixmap(QPixmap());
        return;
    }

    QPixmap pixmap(imagePath);
    if (pixmap.isNull()) {
        m_imagePreview->setText(tr("无效图片"));
        m_imagePreview->setPixmap(QPixmap());
    } else {
        m_imagePreview->setPixmap(pixmap.scaled(m_imagePreview->size(), 
                                             Qt::KeepAspectRatio, 
                                             Qt::SmoothTransformation));
    }
}

std::unique_ptr<Product> ProductDialog::getProduct() const
{
    qDebug() << "ProductDialog::getProduct called, m_editMode:" << m_editMode;
    auto product = std::make_unique<Product>();
    product->setName(m_nameEdit->text().trimmed());
    product->setBarcode(m_barcodeEdit->text().trimmed());
    product->setDescription(m_descriptionEdit->toPlainText().trimmed());
    product->setPrice(m_priceSpinBox->value());
    product->setStockQuantity(m_stockSpinBox->value());
    product->setCategory(m_categoryCombo->currentText().trimmed());
    if (m_editMode) {
        product->setProductId(m_product.getProductId());
    }
    qDebug() << "ProductDialog::getProduct 返回商品:" << product->getName();
    return product;
}

void ProductDialog::setProduct(const Product *product)
{
    qDebug() << "ProductDialog::setProduct called, product:" << product;
    if (!product) return;

    m_product = *product;
    
    m_nameEdit->setText(m_product.getName());
    m_barcodeEdit->setText(m_product.getBarcode());
    m_priceSpinBox->setValue(m_product.getPrice());
    m_stockSpinBox->setValue(m_product.getStockQuantity());
    
    int categoryIndex = m_categoryCombo->findText(m_product.getCategory());
    if (categoryIndex >= 0) {
        m_categoryCombo->setCurrentIndex(categoryIndex);
    } else {
        m_categoryCombo->setCurrentText(m_product.getCategory());
    }
    
    m_descriptionEdit->setPlainText(m_product.getDescription());
    
    validateInput();
    qDebug() << "ProductDialog::setProduct 完成, 商品名称:" << product->getName();
}

void ProductDialog::onAccept()
{
    validateInput();
    if (m_okButton->isEnabled()) {
        accept();
    } else {
        QMessageBox::warning(this, tr("输入无效"), tr("请检查所有必填字段并更正错误。"));
    }
}

void ProductDialog::accept()
{
    qDebug() << "ProductDialog::accept called";
    validateInput();
    qDebug() << "ProductDialog::accept 验证完成，关闭对话框";
    QDialog::accept();
}

void ProductDialog::reject()
{
    qDebug() << "ProductDialog::reject called";
    QDialog::reject();
}
