#include "light_contrast.h"

QImage lightContrastImage(const QImage &img, const int& light /*亮度*/, const int& contrast /*对比度*/) {
    QImage imgCopy;
    if (img.format() != QImage::Format_Grayscale8) {
        imgCopy = QImage(img).convertToFormat(QImage::Format_Grayscale8);
    } else {
        imgCopy = QImage(img);
    }
    int t;
    double dbd = 0.01 * contrast;
    for (int i = 0; i < img.height(); ++i) {
        uint8_t *line = imgCopy.scanLine(i);
        for (int j = 0; j < img.width(); ++j) {
            t = double(line[j]) * dbd - (255.0 * dbd - 255.0) + light;
            t = qBound(0, t, 255);
            line[j] = t;
        }
    }
    return imgCopy;
}
