#ifndef RECEIPTPRINTER_H
#define RECEIPTPRINTER_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QTextDocument>
#include <QPrinter>
#include <QPrintDialog>
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>

#include "../models/Sale.h"
#include "../models/SaleItem.h"
#include "../models/Product.h"

class ReceiptPrinter : public QObject
{
    Q_OBJECT

public:
    enum PrinterType {
        ThermalPrinter,
        StandardPrinter,
        PDFExport,
        EmailSend
    };

    explicit ReceiptPrinter(QObject *parent = nullptr);

    // 打印小票
    bool printReceipt(const Sale &sale, const QList<SaleItem*> &items);
    
    // 设置打印机类型
    void setPrinterType(PrinterType type);
    PrinterType getPrinterType() const;
    
    // 设置商店信息
    void setStoreInfo(const QString &name, const QString &address, const QString &phone);
    
    // 导出小票为PDF
    bool exportToPDF(const Sale &sale, const QList<SaleItem> &items, const QString &fileName = QString());
    
    // 生成小票HTML内容
    QString generateReceiptHTML(const Sale &sale, const QList<SaleItem> &items);
    
    // 生成小票纯文本内容
    QString generateReceiptText(const Sale &sale, const QList<SaleItem> &items);

signals:
    void printStarted();
    void printFinished(bool success);
    void printError(const QString &error);

private slots:
    void onPrintFinished();

private:
    void setupPrinter();
    QString formatCurrency(double amount) const;
    QString formatDateTime(const QDateTime &dateTime) const;
    QString centerText(const QString &text, int width = 40) const;
    QString padRight(const QString &text, int width) const;
    QString repeatChar(char ch, int count) const;

    PrinterType m_printerType;
    QPrinter *m_printer;
    
    // 商店信息
    QString m_storeName;
    QString m_storeAddress;
    QString m_storePhone;
    
    // 设置
    bool m_printLogo;
    QString m_logoPath;
    QString m_footerText;
};

#endif // RECEIPTPRINTER_H
