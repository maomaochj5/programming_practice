#include "RecommendationItemWidget.h"
#include "../models/Product.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QPixmap>
#include <QMouseEvent>

RecommendationItemWidget::RecommendationItemWidget(const Product* product, QWidget *parent)
    : QWidget(parent), m_product(product)
{
    setupUi();
}

void RecommendationItemWidget::setupUi()
{
    if (!m_product) {
        return;
    }

    setFixedSize(150, 200);
    // This will be handled by a global stylesheet targeting this widget's class
    // setStyleSheet("background-color: white; border-radius: 8px; border: 1px solid #e0e0e0;");

    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(5);
    layout->setContentsMargins(10, 10, 10, 10);

    // Product Image
    m_imageLabel = new QLabel(this);
    m_imageLabel->setFixedSize(130, 100);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    QPixmap pixmap(m_product->getImagePath());
    if (pixmap.isNull()) {
        pixmap.load(":/resources/no_image.png");
    }
    m_imageLabel->setPixmap(pixmap.scaled(m_imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    layout->addWidget(m_imageLabel);
    
    // Product Name
    m_nameLabel = new QLabel(m_product->getName(), this);
    m_nameLabel->setWordWrap(true);
    m_nameLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_nameLabel);

    // Product Price
    m_priceLabel = new QLabel(QString("¥%1").arg(m_product->getPrice(), 0, 'f', 2), this);
    m_priceLabel->setAlignment(Qt::AlignCenter);
    // m_priceLabel->setStyleSheet("font-weight: bold; color: #d32f2f;");
    layout->addWidget(m_priceLabel);

    layout->addStretch();

    // Add to Cart Button
    m_addButton = new QPushButton("添加到购物车", this);
    connect(m_addButton, &QPushButton::clicked, this, [this]() {
        emit addToCartClicked(m_product->getProductId());
    });
    layout->addWidget(m_addButton);
}

void RecommendationItemWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_product) {
        emit productSelected(m_product->getProductId());
    }
    QWidget::mousePressEvent(event);
} 