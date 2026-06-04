#ifndef SVGIMAGEPROVIDER_H
#define SVGIMAGEPROVIDER_H

#include <QQuickImageProvider>
#include <QImage>
#include <QMutex>
#include <QMap>

class SvgImageProvider : public QQuickImageProvider
{
public:
    SvgImageProvider();
    ~SvgImageProvider() override;

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

    // Load SVG file and render to image
    // width/height: output size in pixels, 0 means use original size
    QString loadSvg(const QString& filePath, int width = 0, int height = 0);

private:
    QMap<QString, QImage> m_imageCache;
    QMutex m_mutex;
    int m_cacheCounter;
};

#endif // SVGIMAGEPROVIDER_H
