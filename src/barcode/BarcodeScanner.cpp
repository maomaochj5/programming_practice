#include "BarcodeScanner.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QImageReader>
#include <QPixmap>
#include <QTime>
#include <QRegularExpression>

#include "ReadBarcode.h"
#include "BarcodeFormat.h"
#include "Result.h"
#include "ImageView.h"
#include "ReaderOptions.h"

BarcodeScanner::BarcodeScanner(QObject *parent)
    : QObject(parent),
      m_status(Stopped),
      m_imageScanTimer(new QTimer(this)),
      m_scanAnimationTimer(new QTimer(this)),
      m_scanProgress(0.0),
      m_currentImageIndex(0)
{
    m_supportedImageFormats << "jpg" << "jpeg" << "png" << "bmp";
    
    connect(m_imageScanTimer, &QTimer::timeout, this, &BarcodeScanner::onImageScanTimer);
    connect(m_scanAnimationTimer, &QTimer::timeout, this, &BarcodeScanner::onScanAnimationTimer);
    
    qDebug() << "Barcode scanner initialized for image scanning.";
}

BarcodeScanner::~BarcodeScanner()
{
    stopScanning();
    qDebug() << "Barcode scanner destroyed.";
}

void BarcodeScanner::stopScanning()
{
    if (m_status == Stopped) return;
    m_imageScanTimer->stop();
    m_scanAnimationTimer->stop();
    setStatus(Stopped);
    qDebug() << "All scanning activities stopped.";
}

bool BarcodeScanner::scanImageFromFile(const QString& filePath)
{
    if (filePath.isEmpty() || !QFileInfo::exists(filePath)) {
        emit scannerError(tr("Image file not found: %1").arg(filePath));
        return false;
    }

    QImage image(filePath);
    if (image.isNull()) {
        emit scannerError(tr("Failed to load image: %1").arg(filePath));
        return false;
    }

    m_currentImage = QPixmap::fromImage(image);
    emit imageLoaded(m_currentImage, filePath);
    setStatus(ScanningImage);

    m_scanProgress = 0.0;
    m_scanAnimationTimer->start(15);

    QTimer::singleShot(100, this, [this, image]() {
        QString barcode = decodeImageBarcode(image);
        m_scanAnimationTimer->stop();
        emit scanAnimationFinished();
        
        bool isLastImageInFolder = (m_currentImageIndex >= m_currentFolderImages.size() - 1);

        if (!barcode.isEmpty()) {
            emit barcodeDetected(barcode);
            if(m_imageScanTimer->isActive()) {
                m_imageScanTimer->stop();
                setStatus(Stopped);
                 qDebug() << "Barcode found, folder scan stopped.";
            } else {
                setStatus(Stopped);
            }
        } else {
            if (!m_imageScanTimer->isActive()) {
                 emit scannerError(tr("No barcode found in the image."));
                 setStatus(Stopped);
            } else if (isLastImageInFolder) {
                qDebug() << "Finished scanning folder, no barcode found in last image.";
                setStatus(Stopped);
            }
        }

        if (m_imageScanTimer->isActive() && barcode.isEmpty()) {
            QTimer::singleShot(500, this, &BarcodeScanner::onImageScanTimer);
        }
    });

    return true;
}

bool BarcodeScanner::scanImageFromFolder(const QString& folderPath)
{
    m_currentFolderImages = getImageFilesFromFolder(folderPath);
    if (m_currentFolderImages.isEmpty()) {
        emit scannerError(tr("No supported image files found in the folder."));
        return false;
    }
    
    m_currentImageIndex = -1; // Will be incremented to 0 by the timer
    m_imageScanTimer->start(10); // Start the scanning loop
    return true;
}

QString BarcodeScanner::decodeImageBarcode(const QImage& image)
{
    if (image.isNull()) {
        return QString();
    }

    try {
        QImage convertedImage = image;
        if (image.format() != QImage::Format_ARGB32) {
             convertedImage = image.convertToFormat(QImage::Format_ARGB32);
        }
        
        ZXing::ImageView imageView{
            convertedImage.constBits(),
            convertedImage.width(),
            convertedImage.height(),
            ZXing::ImageFormat::BGRX
        };

        ZXing::ReaderOptions options;
        options.setTryHarder(true);
        options.setTryRotate(true);
        options.setFormats(ZXing::BarcodeFormat::Any);

        auto results = ZXing::ReadBarcodes(imageView, options);

        if (!results.empty() && results[0].isValid()) {
            return QString::fromStdString(results[0].text());
        }
    }
    catch (const std::exception& e) {
        qWarning() << "ZXing-C++ exception: " << e.what();
    }
    return QString();
}

QStringList BarcodeScanner::getImageFilesFromFolder(const QString& folderPath) const
{
    QDir dir(folderPath);
    QStringList nameFilters;
    for (const QString &format : m_supportedImageFormats) {
        nameFilters << "*." + format;
    }
    
    QStringList imagePaths;
    for(const QFileInfo &fileInfo : dir.entryInfoList(nameFilters, QDir::Files)) {
        imagePaths << fileInfo.absoluteFilePath();
    }
    return imagePaths;
}

void BarcodeScanner::setSupportedImageFormats(const QStringList& formats)
{
    m_supportedImageFormats = formats;
}

void BarcodeScanner::onImageScanTimer()
{
    m_currentImageIndex++;
    if (m_currentImageIndex < m_currentFolderImages.size()) {
        scanImageFromFile(m_currentFolderImages[m_currentImageIndex]);
    } else {
        m_imageScanTimer->stop();
        setStatus(Stopped);
        qDebug() << "Finished scanning all images in the folder.";
    }
}

void BarcodeScanner::onScanAnimationTimer()
{
    m_scanProgress += 0.02;
    if (m_scanProgress > 1.0) {
        m_scanProgress = 0.0;
    }
    emit scanProgressUpdated(m_scanProgress);
}

void BarcodeScanner::setStatus(ScannerStatus status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged(status);
    }
}





