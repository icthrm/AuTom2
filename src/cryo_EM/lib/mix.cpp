#include "mix.h"

void mix(QImage& img1, const QImage& img2) {
    int w = img1.width(), h = img1.height();
    const QImage img3 = img2.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    for (int i = 0; i < h; i++) {
        uchar* line1 = img1.scanLine(i);
        const uchar* line2 = img2.scanLine(i);
        for (int j = 0; j < w; j++) {
            line1[j] >>= 1;  // 分开计算，防止溢出
            line1[j] += (line2[j] >> 1);
        }
    }
}