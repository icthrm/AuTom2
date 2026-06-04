#include "svgimageprovider.h"
#include <QSvgRenderer>
#include <QPainter>
#include <QFile>
#include <QDebug>

SvgImageProvider::SvgImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
    , m_cacheCounter(0)
{
}

SvgImageProvider::~SvgImageProvider()
{
}

QImage SvgImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
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

    qWarning() << "SvgImageProvider: Image not found in cache:" << id;
    return QImage();
}

QString SvgImageProvider::loadSvg(const QString& filePath, int width, int height)
{
    qDebug() << "SvgImageProvider: Loading SVG from" << filePath << "size:" << width << "x" << height;

    // Check if file exists
    if (!QFile::exists(filePath)) {
        qWarning() << "SVG file does not exist:" << filePath;
        return QString();
    }

    // Create SVG renderer
    QSvgRenderer renderer(filePath);
    if (!renderer.isValid()) {
        qWarning() << "Failed to load SVG file:" << filePath;
        return QString();
    }

    // Get default size if width/height not specified
    QSize renderSize;
    if (width <= 0 || height <= 0) {
        renderSize = renderer.defaultSize();
        qDebug() << "Using default SVG size:" << renderSize;
    } else {
        renderSize = QSize(width, height);
    }

    // Create image with appropriate size
    QImage image(renderSize, QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    // Render SVG to image
    QPainter painter(&image);
    renderer.render(&painter);
    painter.end();

    if (image.isNull()) {
        qWarning() << "Failed to render SVG to image";
        return QString();
    }

    // Generate cache key
    QMutexLocker locker(&m_mutex);
    QString cacheKey = QString("svg_%1").arg(m_cacheCounter++);
    m_imageCache.insert(cacheKey, image);

    qDebug() << "SvgImageProvider: Cached image with key:" << cacheKey << "size:" << image.size();
    return cacheKey;
}
