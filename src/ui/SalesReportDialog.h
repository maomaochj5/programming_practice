#ifndef SALESREPORTDIALOG_H
#define SALESREPORTDIALOG_H

#include <QDialog>
#include <QList>

namespace Ui {
class SalesReportDialog;
}

class Sale;
class QStandardItemModel;

class SalesReportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SalesReportDialog(QWidget *parent = nullptr);
    ~SalesReportDialog();

private:
    void setupModel();
    void populateTable(const QList<Sale*>& sales);

    Ui::SalesReportDialog *ui;
    QStandardItemModel* m_model;
};

#endif // SALESREPORTDIALOG_H 