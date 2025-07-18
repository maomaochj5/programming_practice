#ifndef RECOMMENDATIONITEMWIDGET_H
#define RECOMMENDATIONITEMWIDGET_H

#include <QWidget>

class QLabel;
class QPushButton;
class Product;

class RecommendationItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RecommendationItemWidget(const Product* product, QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent* event) override;

signals:
    void addToCartClicked(int productId);
    void productSelected(int productId);

private:
    void setupUi();

    const Product* m_product;
    
    QLabel* m_imageLabel;
    QLabel* m_nameLabel;
    QLabel* m_priceLabel;
    QPushButton* m_addButton;
};

#endif // RECOMMENDATIONITEMWIDGET_H 