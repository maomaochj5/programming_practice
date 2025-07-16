#ifndef BARCODESCANNER_H
#define BARCODESCANNER_H

#include <QObject>
#include <QTimer>
#include <QRectF>
#include <QPixmap>
#include <QStringList>
// #include <QCamera>
// #include <QVideoWidget>
#include <memory>

// 前向声明
class QCamera;
class QVideoWidget;

/**
 * @brief BarcodeScanner类 - 条形码扫描器
 * 
 * 基于图片文件的条码识别系统
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
        LoadingImage,       ///< 正在加载图片
        ScanningImage       ///< 正在扫描图片
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
     * @brief 获取当前状态
     * @return 当前扫描器状态
     */
    ScannerStatus getStatus() const { return m_status; }

    /**
     * @brief 停止所有扫描活动
     */
    void stopScanning();

    /**
     * @brief 从图片文件扫描条码
     * @param filePath 图片文件路径
     * @return 如果成功开始扫描返回true
     */
    bool scanImageFromFile(const QString& filePath);

    /**
     * @brief 从文件夹选择图片进行扫描
     * @param folderPath 文件夹路径
     * @return 如果成功选择文件夹返回true
     */
    bool scanImageFromFolder(const QString& folderPath);

    /**
     * @brief 获取当前显示的图片
     * @return 当前图片的QPixmap
     */
    QPixmap getCurrentImage() const { return m_currentImage; }

    /**
     * @brief 获取文件夹中所有图片文件
     * @param folderPath 文件夹路径
     * @return 图片文件路径列表
     */
    QStringList getImageFilesFromFolder(const QString& folderPath) const;

    /**
     * @brief 设置支持的图片格式
     * @param formats 图片格式列表（如 "jpg", "png", "bmp"）
     */
    void setSupportedImageFormats(const QStringList& formats);

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

    /**
     * @brief 图片加载完成时发射的信号
     * @param image 加载的图片
     * @param filePath 图片文件路径
     */
    void imageLoaded(const QPixmap& image, const QString& filePath);

    /**
     * @brief 扫描动画进度更新信号
     * @param progress 进度值（0.0-1.0）
     */
    void scanProgressUpdated(double progress);

    /**
     * @brief 扫描动画完成信号
     */
    void scanAnimationFinished();

private slots:
    /**
     * @brief 图片扫描定时器槽函数
     */
    void onImageScanTimer();

    /**
     * @brief 扫描动画定时器槽函数
     */
    void onScanAnimationTimer();

private:
    /**
     * @brief 设置扫描器状态
     * @param status 新状态
     */
    void setStatus(ScannerStatus status);

    /**
     * @brief 从图片中解码条码
     * @param image 要扫描的图片
     * @return 解码到的条码字符串，如果没有找到返回空字符串
     */
    QString decodeImageBarcode(const QImage& image);

    // 状态
    ScannerStatus m_status;
    
    // 图片处理
    QPixmap m_currentImage;
    QStringList m_currentFolderImages;
    int m_currentImageIndex;
    QStringList m_supportedImageFormats;

    // 定时器
    QTimer* m_imageScanTimer;
    QTimer* m_scanAnimationTimer;

    // 扫描动画
    double m_scanProgress;
};

#endif // BARCODESCANNER_H
