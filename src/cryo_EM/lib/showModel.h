#pragma once
#include <QImage>
#include <QPainter>
#include <QPen>
#include <QVector>

#include "Object.h"

// void drawModel(QImage& img, const QVector<Object>& objects, int center_x, int center_y, int center_z, int rotate_x_deg, int rotate_y_deg, int rotate_z_deg);
void drawModel(QImage& img, const QVector<Object*>& objects, const int& center_z);
