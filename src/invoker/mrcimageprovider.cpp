#include "mrcimageprovider.h"
#include "mrc2img.h"
#include <opencv2/opencv.hpp>
#include <QDebug>

MrcImageProvider::MrcImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
    , m_cacheCounter(0)
{
}

QImage MrcImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    QMutexLocker locker(&m_mutex);

    // id 格式: "cache_key"
    if (m_imageCache.contains(id)) {
        QImage image = m_imageCache.value(id);
        if (size) {
            *size = image.size();
        }
        return image;
    }

    qWarning() << "MrcImageProvider: Image not found in cache:" << id;
    return QImage();
}

QString MrcImageProvider::loadMrcSlice(const QString& filePath, int sliceIndex)
{
    qDebug() << "MrcImageProvider: Loading MRC slice from" << filePath << "index" << sliceIndex;

    // 创建 MrcStack 对象
    util::MrcStack mrcStack;

    // 打开 MRC 文件
    if (!mrcStack.Open(filePath.toStdString().c_str())) {
        qWarning() << "Failed to open MRC file:" << filePath;
        return QString();
    }

    // 检查索引是否有效
    if (sliceIndex < 0 || sliceIndex >= mrcStack.Size()) {
        qWarning() << "Invalid slice index:" << sliceIndex << "Total slices:" << mrcStack.Size();
        mrcStack.Close();
        return QString();
    }

    // 读取指定的切片
    cv::Mat slice = mrcStack.GetStackImage(sliceIndex);

    // 关闭 MRC 文件
    mrcStack.Close();

    // 转换 cv::Mat 到 QImage
    if (slice.empty()) {
        qWarning() << "Failed to read slice from MRC file";
        return QString();
    }

    // 归一化到 0-255 范围
    cv::Mat normalized;
    cv::normalize(slice, normalized, 0, 255, cv::NORM_MINMAX);
    normalized.convertTo(normalized, CV_8UC1);

    // 转换为 QImage
    QImage image(normalized.data, normalized.cols, normalized.rows,
                 normalized.step, QImage::Format_Grayscale8);

    // 深拷贝
    QImage cachedImage = image.copy();

    // 生成缓存 key
    QMutexLocker locker(&m_mutex);
    QString cacheKey = QString("mrc_%1").arg(m_cacheCounter++);
    m_imageCache.insert(cacheKey, cachedImage);

    qDebug() << "MrcImageProvider: Cached image with key:" << cacheKey;
    return cacheKey;
}
