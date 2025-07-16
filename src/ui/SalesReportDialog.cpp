#include "SalesReportDialog.h"
#include "ui_SalesReportDialog.h"
#include "../models/Sale.h"
#include "../database/DatabaseManager.h"
#include <QStandardItemModel>
#include <QHeaderView>
#include <QFutureWatcher>

SalesReportDialog::SalesReportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SalesReportDialog),
    m_model(new QStandardItemModel(this))
{
    ui->setupUi(this);
    setupModel();
    
    auto watcher = new QFutureWatcher<QList<Sale*>>(this);
    connect(watcher, &QFutureWatcher<QList<Sale*>>::finished, this, [this, watcher]() {
        const QList<Sale*> sales = watcher->result();
        populateTable(sales);
        ui->statusbar->clearMessage();
        if(sales.isEmpty()){
            ui->statusbar->showMessage("没有找到任何销售记录。", 5000);
        } else {
            ui->statusbar->showMessage(QString("成功加载 %1 条销售记录。").arg(sales.count()), 5000);
        }
        qDeleteAll(sales); // Clean up the objects
    });
    connect(watcher, &QFutureWatcher<QList<Sale*>>::finished, watcher, &QObject::deleteLater);

    watcher->setFuture(DatabaseManager::getInstance().getAllTransactions());

    // 可以添加一个加载中的提示
    ui->statusbar->showMessage("正在加载销售数据...");
}

SalesReportDialog::~SalesReportDialog()
{
    // sales list is now managed by DatabaseManager's signal, which we assume is handled elsewhere
    delete ui;
}

void SalesReportDialog::setupModel()
{
    setWindowTitle("销售报表");
    setMinimumSize(800, 600);

    ui->tableView->setModel(m_model);
    m_model->setHorizontalHeaderLabels({ "交易ID", "日期和时间", "收银员", "商品总数", "折扣", "总金额" });

    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setAlternatingRowColors(true);
}

void SalesReportDialog::populateTable(const QList<Sale*>& sales)
{
    m_model->removeRows(0, m_model->rowCount());

    for (const Sale* sale : sales) {
        if (!sale) continue;

        QList<QStandardItem*> row;
        row.append(new QStandardItem(QString::number(sale->getTransactionId())));
        row.append(new QStandardItem(sale->getTimestamp().toString("yyyy-MM-dd hh:mm:ss")));
        row.append(new QStandardItem(sale->getCashierName()));
        row.append(new QStandardItem(QString::number(sale->getTotalItemCount())));
        row.append(new QStandardItem(QString::asprintf("%.2f", sale->getDiscountAmount())));
        row.append(new QStandardItem(QString::asprintf("%.2f", sale->getFinalAmount())));

        for (auto item : row) {
            item->setTextAlignment(Qt::AlignCenter);
        }

        m_model->appendRow(row);
    }
} 