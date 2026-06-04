#ifndef LIGHT_CONTRAST_H
#define LIGHT_CONTRAST_H
#include<QImage>

QImage lightContrastImage(const QImage &img,
                          const int &light /*brightness, range [-150,150]*/,
                          const int &contrast /*contrast, range [0,200]*/); // Designed for grayscale images only

#endif // LIGHT_CONTRAST_H
