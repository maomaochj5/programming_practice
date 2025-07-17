#ifndef RECOMMENDATIONDIALOG_H
#define RECOMMENDATIONDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableView>
#include <QPushButton>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QList>

// Forward declarations
class Product;

/**
 * @brief RecommendationDialog类 - AI推荐商品展示对话框
 * 
 * 显示AI推荐的商品列表，允许用户选择要添加到购物车的商品
 */
class RecommendationDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit RecommendationDialog(QWidget *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~RecommendationDialog();

    /**
     * @brief 设置推荐商品列表
     * @param products 推荐的商品列表
     */
    void setRecommendations(const QList<Product*>& products);

signals:
    /**
     * @brief 用户选择商品加入购物车时发射的信号
     * @param productIds 选中的商品ID列表
     */
    void itemsSelectedForCart(const QList<int>& productIds);

private slots:
    /**
     * @brief 添加选中商品到购物车
     */
    void onAddSelectedItems();

    /**
     * @brief 添加全部商品到购物车
     */
    void onAddAllItems();

    /**
     * @brief 关闭对话框
     */
    void onCloseDialog();

private:
    /**
     * @brief 初始化用户界面
     */
    void initializeUI();

    /**
     * @brief 设置样式表
     */
    void setupStyleSheet();

    /**
     * @brief 获取选中的商品ID列表
     * @return 选中的商品ID列表
     */
    QList<int> getSelectedProductIds() const;

    /**
     * @brief 获取所有商品ID列表
     * @return 所有商品ID列表
     */
    QList<int> getAllProductIds() const;

private:
    // UI组件
    QVBoxLayout* m_mainLayout;           ///< 主布局
    QLabel* m_titleLabel;                ///< 标题标签
    QTableView* m_recommendationTable;   ///< 推荐商品表格
    QHBoxLayout* m_buttonLayout;         ///< 按钮布局
    QPushButton* m_addSelectedButton;    ///< 添加选中商品按钮
    QPushButton* m_addAllButton;         ///< 全部添加按钮
    QPushButton* m_closeButton;          ///< 关闭按钮

    // 数据模型
    QStandardItemModel* m_model;         ///< 表格数据模型

    // 列索引枚举
    enum ColumnIndex {
        CheckboxColumn = 0,              ///< 复选框列
        ProductNameColumn = 1,           ///< 商品名称列
        PriceColumn = 2,                 ///< 价格列
        ColumnCount = 3                  ///< 总列数
    };
};

#endif // RECOMMENDATIONDIALOG_H 