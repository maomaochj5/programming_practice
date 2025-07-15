#include "BarcodeScanner.h"
// #include <QCameraInfo>
#include <QDebug>
#include <QThread>
#include <QRegularExpression>

BarcodeScanner::BarcodeScanner(QObject *parent)
    : QObject(parent)
    , m_camera(nullptr)
    , m_videoWidget(nullptr)
    , m_status(Stopped)
    , m_simulationTimer(new QTimer(this))
    , m_scanRegion(0.2, 0.2, 0.6, 0.6)  // 中央60%区域
    , m_currentCameraIndex(0)
    , m_scanInterval(100)  // 100ms
    , m_enableAutoFocus(true)
    , m_demoIndex(0)
{
    // 设置支持的条码格式（常见的超市商品条码）
    m_supportedFormats << "EAN-13" << "EAN-8" << "UPC-A" << "UPC-E" << "Code-128";
    
    // 初始化演示商品条码列表
    m_demoProducts << "6901028075381"  // 可口可乐
                   << "6920202220010"  // 康师傅方便面
                   << "6943395100109"  // 旺旺雪饼
                   << "6901028110662"  // 蒙牛牛奶
                   << "6925303720018"  // 统一红茶
                   << "6922255451000"  // 双汇火腿肠
                   << "6901028142014"  // 雀巢咖啡
                   << "6925303720025"; // 康师傅绿茶
    
    // 连接模拟定时器
    connect(m_simulationTimer, &QTimer::timeout, this, &BarcodeScanner::onSimulationTimer);
    
    qDebug() << "条形码扫描器初始化完成";
}

BarcodeScanner::~BarcodeScanner()
{
    stopScanning();
    qDebug() << "条形码扫描器析构";
}

bool BarcodeScanner::startScanning()
{
    if (m_status == Running) {
        qDebug() << "扫描器已在运行";
        return true;
    }
    
    setStatus(Starting);
    
    // 在实际项目中，这里会初始化摄像头和QZXing
    // 为了演示，我们使用模拟扫描
    if (!initializeCamera()) {
        setStatus(Error);
        emit scannerError("无法初始化摄像头");
        return false;
    }
    
    setStatus(Running);
    
    // 启动模拟扫描定时器（用于演示）
    m_simulationTimer->start(5000); // 每5秒模拟一次扫描
    
    qDebug() << "扫描器启动成功";
    return true;
}

void BarcodeScanner::stopScanning()
{
    if (m_status == Stopped) {
        return;
    }
    
    m_simulationTimer->stop();
    
    if (m_camera) {
        // m_camera->stop();  // 简化：不调用实际摄像头
    }
    
    setStatus(Stopped);
    qDebug() << "扫描器已停止";
}

bool BarcodeScanner::isAvailable() const
{
    // 简化版本：假设摄像头总是可用
    return true;
}

void BarcodeScanner::setVideoWidget(QVideoWidget* videoWidget)
{
    m_videoWidget = videoWidget;
    // 简化版本：不实际连接摄像头
}

QStringList BarcodeScanner::getAvailableCameras() const
{
    QStringList cameraNames;
    // 简化版本：返回模拟摄像头列表
    cameraNames << "模拟摄像头 1" << "模拟摄像头 2";
    return cameraNames;
}

bool BarcodeScanner::selectCamera(int cameraIndex)
{
    // 简化版本：接受任何索引
    if (cameraIndex < 0 || cameraIndex >= 2) {
        emit scannerError("无效的摄像头索引");
        return false;
    }
    
    m_currentCameraIndex = cameraIndex;
    
    // 如果扫描器正在运行，重新初始化摄像头
    if (m_status == Running) {
        stopScanning();
        return startScanning();
    }
    
    return true;
    
    return true;
}

void BarcodeScanner::setScanRegion(const QRectF& rect)
{
    // 确保区域在有效范围内
    m_scanRegion = QRectF(
        qBound(0.0, rect.x(), 1.0),
        qBound(0.0, rect.y(), 1.0),
        qBound(0.0, rect.width(), 1.0 - rect.x()),
        qBound(0.0, rect.height(), 1.0 - rect.y())
    );
    
    qDebug() << "设置扫描区域:" << m_scanRegion;
}

void BarcodeScanner::setSupportedFormats(const QStringList& formats)
{
    m_supportedFormats = formats;
    qDebug() << "设置支持的条码格式:" << formats;
}

void BarcodeScanner::simulateScan(const QString& barcode)
{
    if (barcode.isEmpty()) {
        return;
    }
    
    if (validateBarcodeFormat(barcode)) {
        qDebug() << "模拟扫描条码:" << barcode;
        emit barcodeDetected(barcode);
    } else {
        qWarning() << "无效的条码格式:" << barcode;
    }
}

void BarcodeScanner::onCameraStatusChanged(int status)
{
    qDebug() << "摄像头状态改变:" << status;
    
    // 简化版本：根据整数状态值来处理
    switch (status) {
        case 1: // ActiveStatus
            if (m_status == Starting) {
                setStatus(Running);
            }
            break;
        case 0: // UnloadedStatus or StoppedStatus
            if (m_status != Stopped) {
                setStatus(Stopped);
            }
            break;
        default:
            break;
    }
}

void BarcodeScanner::onCameraError(int error)
{
    QString errorMessage;
    
    // 简化版本：根据整数错误码处理
    switch (error) {
        case 0: // NoError
            return;
        case 1: // CameraError
            errorMessage = "摄像头硬件错误";
            break;
        case 2: // InvalidRequestError
            errorMessage = "摄像头请求无效";
            break;
        case 3: // ServiceMissingError
            errorMessage = "摄像头服务不可用";
            break;
        case 4: // NotSupportedFeatureError
            errorMessage = "摄像头功能不支持";
            break;
        default:
            errorMessage = "未知摄像头错误";
            break;
    }
    
    qCritical() << "摄像头错误:" << errorMessage;
    setStatus(Error);
    emit scannerError(errorMessage);
}

void BarcodeScanner::onSimulationTimer()
{
    // 模拟扫描演示商品
    if (!m_demoProducts.isEmpty()) {
        QString barcode = m_demoProducts[m_demoIndex];
        m_demoIndex = (m_demoIndex + 1) % m_demoProducts.size();
        
        qDebug() << "模拟扫描演示商品:" << barcode;
        emit barcodeDetected(barcode);
    }
}

bool BarcodeScanner::initializeCamera()
{
    // 简化版本：总是返回成功
    qDebug() << "初始化摄像头（模拟模式）";
    return true;
}

void BarcodeScanner::setStatus(ScannerStatus status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged(status);
        qDebug() << "扫描器状态改变:" << status;
    }
}

void BarcodeScanner::processFrame(const QByteArray& frameData)
{
    Q_UNUSED(frameData)
    
    // 在实际实现中，这里会：
    // 1. 从frameData中提取图像
    // 2. 根据m_scanRegion裁剪感兴趣区域
    // 3. 调用QZXing进行条码解码
    // 4. 如果解码成功，发射barcodeDetected信号
    
    // 示例伪代码：
    /*
    QImage image = convertFrameDataToImage(frameData);
    QRect scanRect = calculateScanRect(image.size(), m_scanRegion);
    QImage croppedImage = image.copy(scanRect);
    
    QString barcode = m_zxingDecoder.decode(croppedImage);
    if (!barcode.isEmpty() && validateBarcodeFormat(barcode)) {
        emit barcodeDetected(barcode);
    }
    */
    
    emit frameUpdated();
}

bool BarcodeScanner::validateBarcodeFormat(const QString& barcode) const
{
    if (barcode.isEmpty()) {
        return false;
    }
    
    // 基本的条码格式验证
    // EAN-13: 13位数字
    if (barcode.length() == 13) {
        return barcode.toULongLong() > 0; // 简单检查是否为数字
    }
    
    // EAN-8: 8位数字
    if (barcode.length() == 8) {
        return barcode.toULongLong() > 0;
    }
    
    // UPC-A: 12位数字
    if (barcode.length() == 12) {
        return barcode.toULongLong() > 0;
    }
    
    // Code-128: 可变长度，包含字母和数字
    if (barcode.length() >= 6 && barcode.length() <= 20) {
        // 简单验证：包含至少一些数字
        return barcode.contains(QRegularExpression("[0-9]+"));
    }
    
    return false;
}
