#include "ReceiptPrinter.h"
#include "../database/DatabaseManager.h"

ReceiptPrinter::ReceiptPrinter(QObject *parent)
    : QObject(parent)
    , m_printerType(ThermalPrinter)
    , m_printer(nullptr)
    , m_storeName("智能超市")
    , m_storeAddress("北京市朝阳区科技大街123号")
    , m_storePhone("400-123-4567")
    , m_printLogo(false)
    , m_footerText("谢谢惠顾，欢迎再次光临！")
{
    setupPrinter();
}

void ReceiptPrinter::setupPrinter()
{
    if (m_printer) {
        delete m_printer;
    }
    
    m_printer = new QPrinter(QPrinter::HighResolution);
    m_printer->setPageSize(QPageSize::A4);
    m_printer->setPageOrientation(QPageLayout::Portrait);
    
    // 针对热敏打印机的设置
    if (m_printerType == ThermalPrinter) {
        m_printer->setPageSize(QPageSize(QSizeF(80, 200), QPageSize::Millimeter));
        m_printer->setPageMargins(QMarginsF(2, 2, 2, 2), QPageLayout::Millimeter);
    }
}

bool ReceiptPrinter::printReceipt(const Sale &sale, const QList<SaleItem*> &items)
{
    // 简化版本：只输出到调试信息
    qDebug() << "打印收据（模拟）:";
    qDebug() << "总金额:" << sale.getTotalAmount();
    qDebug() << "商品数量:" << items.size();
    
    // 在实际项目中，这里会生成PDF或打印到打印机
    emit printFinished(true);
    return true;
}

bool ReceiptPrinter::exportToPDF(const Sale &sale, const QList<SaleItem> &items, const QString &fileName)
{
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QDir receiptDir(documentsPath + "/SmartPOS/Receipts");
    
    if (!receiptDir.exists()) {
        receiptDir.mkpath(".");
    }
    
    QString fullFileName = fileName.isEmpty() ? 
        QString("receipt_%1_%2.pdf").arg(sale.getTransactionId()).arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")) :
        fileName;
    
    if (!fullFileName.endsWith(".pdf")) {
        fullFileName += ".pdf";
    }
    
    QString filePath = receiptDir.absoluteFilePath(fullFileName);
    
    QPrinter pdfPrinter(QPrinter::HighResolution);
    pdfPrinter.setOutputFormat(QPrinter::PdfFormat);
    pdfPrinter.setOutputFileName(filePath);
    pdfPrinter.setPageSize(QPageSize::A4);
    pdfPrinter.setPageMargins(QMarginsF(10, 10, 10, 10), QPageLayout::Millimeter);
    
    QTextDocument document;
    document.setHtml(generateReceiptHTML(sale, items));
    document.print(&pdfPrinter);
    
    // 打开PDF文件
    QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
    
    return true;
}

QString ReceiptPrinter::generateReceiptHTML(const Sale &sale, const QList<SaleItem> &items)
{
    QString html;
    html += "<html><head><meta charset='UTF-8'></head><body>";
    html += "<div style='font-family: \"Courier New\", monospace; font-size: 12px; line-height: 1.2;'>";
    
    // 店铺信息
    html += "<div style='text-align: center; margin-bottom: 10px;'>";
    html += QString("<h2 style='margin: 0;'>%1</h2>").arg(m_storeName);
    html += QString("<div>%1</div>").arg(m_storeAddress);
    html += QString("<div>电话: %1</div>").arg(m_storePhone);
    html += "</div>";
    
    // 分隔线
    html += "<div style='text-align: center; margin: 10px 0;'>";
    html += repeatChar('=', 40);
    html += "</div>";
    
    // 小票信息
    html += "<div style='margin-bottom: 10px;'>";
    html += QString("<div>小票号: %1</div>").arg(sale.getTransactionId());
    html += QString("<div>收银员: %1</div>").arg(sale.getCashierName());
    html += QString("<div>时间: %1</div>").arg(formatDateTime(sale.getTimestamp()));
    html += "</div>";
    
    // 商品列表
    html += "<div style='margin: 10px 0;'>";
    html += repeatChar('-', 40);
    html += "</div>";
    
    html += "<table style='width: 100%; border-collapse: collapse;'>";
    html += "<tr style='border-bottom: 1px solid #ccc;'>";
    html += "<th style='text-align: left; padding: 2px;'>商品</th>";
    html += "<th style='text-align: center; padding: 2px;'>数量</th>";
    html += "<th style='text-align: right; padding: 2px;'>单价</th>";
    html += "<th style='text-align: right; padding: 2px;'>小计</th>";
    html += "</tr>";
    
    for (const SaleItem& item : items) {
        // 获取商品信息直接从SaleItem
        Product* product = item.getProduct();
        
        html += "<tr>";
        html += QString("<td style='padding: 2px;'>%1</td>").arg(product ? product->getName() : "未知商品");
        html += QString("<td style='text-align: center; padding: 2px;'>%1</td>").arg(item.getQuantity());
        html += QString("<td style='text-align: right; padding: 2px;'>%1</td>").arg(formatCurrency(item.getUnitPrice()));
        html += QString("<td style='text-align: right; padding: 2px;'>%1</td>").arg(formatCurrency(item.getSubtotal()));
        html += "</tr>";
    }
    
    html += "</table>";
    
    // 合计信息
    html += "<div style='margin: 10px 0;'>";
    html += repeatChar('-', 40);
    html += "</div>";
    
    html += "<div style='text-align: right; margin: 5px 0;'>";
    html += QString("<div>商品总数: %1 件</div>").arg(items.size());
    html += QString("<div style='font-size: 14px; font-weight: bold;'>总金额: %1</div>").arg(formatCurrency(sale.getTotalAmount()));
    
    if (sale.getDiscountAmount() > 0) {
        html += QString("<div>优惠金额: -%1</div>").arg(formatCurrency(sale.getDiscountAmount()));
        html += QString("<div style='font-size: 14px; font-weight: bold;'>实付金额: %1</div>").arg(formatCurrency(sale.getTotalAmount() - sale.getDiscountAmount()));
    }
    html += "</div>";
    
    // 支付信息
    html += "<div style='margin: 10px 0;'>";
    html += QString("<div>支付方式: %1</div>").arg(sale.getPaymentMethod());
    html += "</div>";
    
    // 页脚
    html += "<div style='text-align: center; margin-top: 20px;'>";
    html += repeatChar('=', 40);
    html += QString("<div style='margin: 10px 0;'>%1</div>").arg(m_footerText);
    html += "<div style='font-size: 10px;'>www.smartpos.com</div>";
    html += "</div>";
    
    html += "</div></body></html>";
    
    return html;
}

QString ReceiptPrinter::generateReceiptText(const Sale &sale, const QList<SaleItem> &items)
{
    QString text;
    
    // 店铺信息
    text += centerText(m_storeName) + "\n";
    text += centerText(m_storeAddress) + "\n";
    text += centerText(QString("电话: %1").arg(m_storePhone)) + "\n";
    text += repeatChar('=', 40) + "\n";
    
    // 小票信息
    text += QString("小票号: %1\n").arg(sale.getTransactionId());
    text += QString("收银员: %1\n").arg(sale.getCashierName());
    text += QString("时间: %1\n").arg(formatDateTime(sale.getTimestamp()));
    text += repeatChar('-', 40) + "\n";
    
    // 商品列表表头
    text += padRight("商品", 20) + padRight("数量", 6) + padRight("单价", 8) + "小计\n";
    text += repeatChar('-', 40) + "\n";
    
    // auto& db = DatabaseManager::getInstance(); // Not needed for this implementation
    for (const SaleItem& item : items) {
        Product* product = item.getProduct();
        
        QString name = product ? product->getName() : "未知商品";
        if (name.length() > 18) {
            name = name.left(18) + "..";
        }
        
        text += padRight(name, 20);
        text += padRight(QString::number(item.getQuantity()), 6);
        text += padRight(formatCurrency(item.getUnitPrice()), 8);
        text += formatCurrency(item.getSubtotal()) + "\n";
    }
    
    // 合计信息
    text += repeatChar('-', 40) + "\n";
    text += padRight(QString("商品总数: %1 件").arg(items.size()), 25);
    text += QString("总金额: %1\n").arg(formatCurrency(sale.getTotalAmount()));
    
    if (sale.getDiscountAmount() > 0) {
        text += padRight("优惠金额:", 25) + QString("-%1\n").arg(formatCurrency(sale.getDiscountAmount()));
        text += padRight("实付金额:", 25) + QString("%1\n").arg(formatCurrency(sale.getTotalAmount() - sale.getDiscountAmount()));
    }
    
    text += QString("支付方式: %1\n").arg(sale.getPaymentMethod());
    
    // 页脚
    text += repeatChar('=', 40) + "\n";
    text += centerText(m_footerText) + "\n";
    text += centerText("www.smartpos.com") + "\n";
    
    return text;
}

void ReceiptPrinter::setPrinterType(PrinterType type)
{
    m_printerType = type;
    setupPrinter();
}

ReceiptPrinter::PrinterType ReceiptPrinter::getPrinterType() const
{
    return m_printerType;
}

void ReceiptPrinter::setStoreInfo(const QString &name, const QString &address, const QString &phone)
{
    m_storeName = name;
    m_storeAddress = address;
    m_storePhone = phone;
}

void ReceiptPrinter::onPrintFinished()
{
    emit printFinished(true);
}

QString ReceiptPrinter::formatCurrency(double amount) const
{
    return QString("¥%1").arg(amount, 0, 'f', 2);
}

QString ReceiptPrinter::formatDateTime(const QDateTime &dateTime) const
{
    return dateTime.toString("yyyy-MM-dd hh:mm:ss");
}

QString ReceiptPrinter::centerText(const QString &text, int width) const
{
    if (text.length() >= width) return text;
    
    int padding = (width - text.length()) / 2;
    return QString(padding, ' ') + text + QString(width - text.length() - padding, ' ');
}

QString ReceiptPrinter::padRight(const QString &text, int width) const
{
    if (text.length() >= width) return text.left(width);
    return text + QString(width - text.length(), ' ');
}

QString ReceiptPrinter::repeatChar(char ch, int count) const
{
    return QString(count, ch);
}
