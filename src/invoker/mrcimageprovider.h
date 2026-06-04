#ifndef MRCIMAGEPROVIDER_H
#define MRCIMAGEPROVIDER_H

#include <QQuickImageProvider>
#include <QImage>
#include <QMutex>
#include <QMap>

class MrcImageProvider : public QQuickImageProvider
{
public:
    MrcImageProvider();

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

    // 加载 MRC 切片并缓存
    QString loadMrcSlice(const QString& filePath, int sliceIndex);

private:
    QMap<QString, QImage> m_imageCache;
    QMutex m_mutex;
    int m_cacheCounter;
};

#endif // MRCIMAGEPROVIDER_H
