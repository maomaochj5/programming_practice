#ifndef BARCODESCANNER_H
#define BARCODESCANNER_H

#include <QObject>
#include <QTimer>
#include <QRectF>
// #include <QCamera>
// #include <QVideoWidget>
#include <memory>

// 前向声明
class QCamera;
class QVideoWidget;

/**
 * @brief BarcodeScanner类 - 条形码扫描器
 * 
 * 基于摄像头的实时条形码识别系统
 * 注意：这是一个简化版本，实际项目中需要集成QZXing库
 */
class BarcodeScanner : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 扫描状态枚举
     */
    enum ScannerStatus {
        Stopped = 0,        ///< 已停止
        Starting,           ///< 启动中
        Running,            ///< 运行中
        Error               ///< 错误状态
    };
    Q_ENUM(ScannerStatus)

    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    explicit BarcodeScanner(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~BarcodeScanner();

    /**
     * @brief 启动扫描器
     * @return 如果启动成功返回true
     */
    bool startScanning();

    /**
     * @brief 停止扫描器
     */
    void stopScanning();

    /**
     * @brief 获取当前状态
     * @return 当前扫描器状态
     */
    ScannerStatus getStatus() const { return m_status; }

    /**
     * @brief 检查扫描器是否可用
     * @return 如果可用返回true
     */
    bool isAvailable() const;

    /**
     * @brief 设置视频输出组件
     * @param videoWidget 视频显示组件
     */
    void setVideoWidget(QVideoWidget* videoWidget);

    /**
     * @brief 获取可用的摄像头列表
     * @return 摄像头设备名称列表
     */
    QStringList getAvailableCameras() const;

    /**
     * @brief 选择摄像头
     * @param cameraIndex 摄像头索引
     * @return 如果选择成功返回true
     */
    bool selectCamera(int cameraIndex);

    /**
     * @brief 设置扫描区域
     * @param rect 扫描区域矩形（相对于视频画面的比例，0.0-1.0）
     */
    void setScanRegion(const QRectF& rect);

    /**
     * @brief 设置支持的条码格式
     * @param formats 条码格式列表
     */
    void setSupportedFormats(const QStringList& formats);

    /**
     * @brief 模拟扫描（用于测试和演示）
     * @param barcode 要模拟的条码
     */
    void simulateScan(const QString& barcode);

signals:
    /**
     * @brief 检测到条码时发射的信号
     * @param barcode 条码字符串
     */
    void barcodeDetected(const QString& barcode);

    /**
     * @brief 扫描器状态改变时发射的信号
     * @param status 新状态
     */
    void statusChanged(ScannerStatus status);

    /**
     * @brief 扫描器错误时发射的信号
     * @param errorMessage 错误消息
     */
    void scannerError(const QString& errorMessage);

    /**
     * @brief 视频帧更新时发射的信号（用于UI更新）
     */
    void frameUpdated();

private slots:
    /**
     * @brief 摄像头状态改变槽函数
     * @param status 摄像头状态
     */
    void onCameraStatusChanged(int status);

    /**
     * @brief 摄像头错误槽函数
     * @param error 错误类型
     */
    void onCameraError(int error);

    /**
     * @brief 模拟扫描定时器槽函数（用于演示）
     */
    void onSimulationTimer();

private:
    /**
     * @brief 初始化摄像头
     * @return 如果初始化成功返回true
     */
    bool initializeCamera();

    /**
     * @brief 设置扫描器状态
     * @param status 新状态
     */
    void setStatus(ScannerStatus status);

    /**
     * @brief 处理帧数据（在实际实现中会进行条码解码）
     * @param frameData 帧数据
     */
    void processFrame(const QByteArray& frameData);

    /**
     * @brief 验证条码格式
     * @param barcode 条码字符串
     * @return 如果格式有效返回true
     */
    bool validateBarcodeFormat(const QString& barcode) const;

private:
    QCamera* m_camera;                      ///< 摄像头对象
    QVideoWidget* m_videoWidget;            ///< 视频显示组件
    ScannerStatus m_status;                 ///< 当前状态
    QTimer* m_simulationTimer;              ///< 模拟扫描定时器
    
    // 扫描配置
    QRectF m_scanRegion;                    ///< 扫描区域
    QStringList m_supportedFormats;         ///< 支持的条码格式
    int m_currentCameraIndex;               ///< 当前摄像头索引
    
    // 性能优化参数
    int m_scanInterval;                     ///< 扫描间隔（毫秒）
    bool m_enableAutoFocus;                 ///< 是否启用自动对焦
    
    // 演示和测试用的模拟数据
    QStringList m_demoProducts;             ///< 演示商品条码列表
    int m_demoIndex;                        ///< 当前演示商品索引
};

#endif // BARCODESCANNER_H
