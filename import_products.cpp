#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // 设置数据库路径
    QString dbPath = "pos_database.db";
    if (argc > 1) {
        dbPath = argv[1];
    }
    
    // 设置products.json路径
    QString jsonPath = "Ai_model/products.json";
    if (argc > 2) {
        jsonPath = argv[2];
    }
    
    qDebug() << "开始导入商品数据...";
    qDebug() << "数据库路径:" << dbPath;
    qDebug() << "JSON文件路径:" << jsonPath;
    
    // 读取JSON文件
    QFile jsonFile(jsonPath);
    if (!jsonFile.open(QIODevice::ReadOnly)) {
        qDebug() << "错误: 无法打开JSON文件:" << jsonPath;
        return 1;
    }
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonFile.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "错误: JSON解析失败:" << parseError.errorString();
        return 1;
    }
    
    if (!doc.isArray()) {
        qDebug() << "错误: JSON文件应该包含一个数组";
        return 1;
    }
    
    QJsonArray productsArray = doc.array();
    qDebug() << "找到" << productsArray.size() << "个商品";
    
    // 连接数据库
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);
    
    if (!db.open()) {
        qDebug() << "错误: 无法打开数据库:" << db.lastError().text();
        return 1;
    }
    
    qDebug() << "数据库连接成功";
    
    // 创建表（如果不存在）
    QSqlQuery query(db);
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS Products (
            product_id INTEGER PRIMARY KEY AUTOINCREMENT,
            barcode TEXT UNIQUE NOT NULL,
            name TEXT NOT NULL,
            description TEXT,
            price REAL NOT NULL CHECK(price >= 0),
            stock_quantity INTEGER NOT NULL CHECK(stock_quantity >= 0),
            category TEXT,
            image_path TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )")) {
        qDebug() << "错误: 无法创建Products表:" << query.lastError().text();
        return 1;
    }
    
    // 清空现有数据（可选）
    if (!query.exec("DELETE FROM Products")) {
        qDebug() << "错误: 无法清空Products表:" << query.lastError().text();
        return 1;
    }
    qDebug() << "已清空现有商品数据";
    
    // 准备插入语句
    query.prepare(R"(
        INSERT INTO Products (barcode, name, description, price, stock_quantity, category, image_path)
        VALUES (?, ?, ?, ?, ?, ?, ?)
    )");
    
    int successCount = 0;
    int errorCount = 0;
    
    // 遍历商品数据
    for (const QJsonValue& value : productsArray) {
        if (!value.isObject()) {
            qDebug() << "警告: 跳过非对象项";
            continue;
        }
        
        QJsonObject product = value.toObject();
        
        // 提取商品信息
        QString productId = product["product_id"].toString();
        QString name = product["product_name"].toString();
        QString category = product["category"].toString();
        double price = product["price"].toDouble();
        QString description = product["description"].toString();
        
        // 生成条形码（使用product_id作为条形码）
        QString barcode = productId;
        
        // 设置默认库存
        int stockQuantity = 100; // 默认库存100件
        
        // 绑定参数
        query.addBindValue(barcode);
        query.addBindValue(name);
        query.addBindValue(description);
        query.addBindValue(price);
        query.addBindValue(stockQuantity);
        query.addBindValue(category);
        query.addBindValue(""); // 图片路径暂时为空
        
        if (query.exec()) {
            successCount++;
            qDebug() << "成功导入商品:" << name << "价格:" << price;
        } else {
            errorCount++;
            qDebug() << "错误: 无法导入商品" << name << ":" << query.lastError().text();
        }
    }
    
    qDebug() << "\n导入完成!";
    qDebug() << "成功导入:" << successCount << "个商品";
    qDebug() << "失败:" << errorCount << "个商品";
    
    // 验证导入结果
    query.exec("SELECT COUNT(*) FROM Products");
    if (query.next()) {
        qDebug() << "数据库中总共有" << query.value(0).toInt() << "个商品";
    }
    
    db.close();
    qDebug() << "数据库连接已关闭";
    
    return 0;
} 