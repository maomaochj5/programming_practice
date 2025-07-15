#ifndef TESTCOMMON_H
#define TESTCOMMON_H

#include <QObject>
#include <QTest>
#include <QDebug>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QDir>

/**
 * @brief 测试公共基类
 * 提供测试用的通用功能和工具方法
 */
class TestCommon : public QObject
{
    Q_OBJECT

public:
    explicit TestCommon(QObject *parent = nullptr);
    virtual ~TestCommon();

protected:
    /**
     * @brief 创建临时测试数据库
     * @return 数据库文件路径
     */
    QString createTestDatabase();

    /**
     * @brief 清理测试数据库
     * @param dbPath 数据库文件路径
     */
    void cleanupTestDatabase(const QString& dbPath);

    /**
     * @brief 验证信号是否被发射
     * @param spy 信号监听器
     * @param expectedCount 期望的信号发射次数
     * @param timeout 超时时间（毫秒）
     * @return 是否验证成功
     */
    bool verifySignalEmitted(QSignalSpy& spy, int expectedCount, int timeout = 1000);

    /**
     * @brief 等待信号发射
     * @param spy 信号监听器
     * @param timeout 超时时间（毫秒）
     * @return 是否成功接收到信号
     */
    bool waitForSignal(QSignalSpy& spy, int timeout = 1000);

    /**
     * @brief 比较浮点数是否相等（考虑精度误差）
     * @param a 第一个数
     * @param b 第二个数
     * @param epsilon 允许的误差
     * @return 是否相等
     */
    bool doubleEquals(double a, double b, double epsilon = 0.001);

    /**
     * @brief 生成测试用的条码
     * @param seed 种子值
     * @return 条码字符串
     */
    QString generateTestBarcode(int seed);

protected:
    QTemporaryDir m_tempDir;
    QString m_testDbPath;
};

// 测试宏定义
#define VERIFY_SIGNAL_EMITTED(spy, count) \
    QVERIFY2(verifySignalEmitted(spy, count), \
             QString("Expected %1 signals, got %2").arg(count).arg(spy.count()).toLocal8Bit())

#define VERIFY_DOUBLE_EQUALS(actual, expected) \
    QVERIFY2(doubleEquals(actual, expected), \
             QString("Expected %1, got %2").arg(expected).arg(actual).toLocal8Bit())

#endif // TESTCOMMON_H
