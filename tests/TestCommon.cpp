#include "TestCommon.h"
#include <QStandardPaths>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QUuid>
#include <cmath>

TestCommon::TestCommon(QObject *parent)
    : QObject(parent)
{
    // 确保临时目录有效
    if (!m_tempDir.isValid()) {
        qWarning() << "Failed to create temporary directory for tests";
    }
}

TestCommon::~TestCommon()
{
    // 清理可能遗留的数据库连接
    if (!m_testDbPath.isEmpty()) {
        cleanupTestDatabase(m_testDbPath);
    }
}

QString TestCommon::createTestDatabase()
{
    if (!m_tempDir.isValid()) {
        return QString();
    }
    
    // 生成唯一的数据库文件名
    QString dbName = QString("test_db_%1.db").arg(QUuid::createUuid().toString().remove('{').remove('}').remove('-'));
    m_testDbPath = m_tempDir.filePath(dbName);
    
    qDebug() << "Created test database:" << m_testDbPath;
    return m_testDbPath;
}

void TestCommon::cleanupTestDatabase(const QString& dbPath)
{
    if (dbPath.isEmpty()) {
        return;
    }
    
    // 关闭数据库连接
    QSqlDatabase db = QSqlDatabase::database();
    if (db.isOpen()) {
        db.close();
    }
    
    // 移除数据库文件
    QFile::remove(dbPath);
    
    // 移除连接
    QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
    
    qDebug() << "Cleaned up test database:" << dbPath;
}

bool TestCommon::verifySignalEmitted(QSignalSpy& spy, int expectedCount, int timeout)
{
    if (spy.count() < expectedCount) {
        spy.wait(timeout);
    }
    
    bool result = spy.count() == expectedCount;
    if (!result) {
        qDebug() << "Signal verification failed. Expected:" << expectedCount << "Got:" << spy.count();
        for (int i = 0; i < spy.count(); ++i) {
            qDebug() << "Signal" << i << ":" << spy.at(i);
        }
    }
    
    return result;
}

bool TestCommon::waitForSignal(QSignalSpy& spy, int timeout)
{
    if (spy.isEmpty()) {
        return spy.wait(timeout);
    }
    return true;
}

bool TestCommon::doubleEquals(double a, double b, double epsilon)
{
    return std::abs(a - b) < epsilon;
}

QString TestCommon::generateTestBarcode(int seed)
{
    // 生成13位测试条码
    QString barcode = QString::number(1000000000000LL + (seed % 9000000000000LL));
    return barcode.leftJustified(13, '0');
}
