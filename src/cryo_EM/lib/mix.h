#ifndef MIX_H
#define MIX_H
#include <QImage>

//采用平均值的方法叠加两张图片，作用于img1
void mix(QImage& img1,const QImage& img2);

#endif // MIX_H
