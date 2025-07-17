#ifndef RECOMMENDATIONDIALOG_H
#define RECOMMENDATIONDIALOG_H

#include <QDialog>
#include <QList>

// Forward declarations
class Product;
class ProductManager;
class QStandardItemModel;
class QVBoxLayout;
class QLabel;
class QTextEdit;
class QTableView;
class QHBoxLayout;
class QPushButton;

class RecommendationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RecommendationDialog(ProductManager* productManager, const QList<int>& productIds, QWidget *parent = nullptr);
    ~RecommendationDialog();

    QList<int> getSelectedProductIds() const;
    void setRecommendationText(const QString& text);

private slots:
    void onAddSelectedItems();
    void onAddAllItems();
    void onCloseDialog();

private:
    void initializeUI();
    void setupStyleSheet();
    void setRecommendations(const QList<Product*>& products);
    QList<int> getAllProductIds() const;

    enum Column {
        CheckboxColumn,
        ProductNameColumn,
        PriceColumn,
        ColumnCount
    };

    ProductManager* m_productManager;
    QStandardItemModel* m_model;
    
    // UI elements
    QVBoxLayout* m_mainLayout;
    QLabel* m_titleLabel;
    QTextEdit* m_responseTextEdit;
    QTableView* m_recommendationTable;
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_addSelectedButton;
    QPushButton* m_addAllButton;
    QPushButton* m_closeButton;
};

#endif // RECOMMENDATIONDIALOG_H 