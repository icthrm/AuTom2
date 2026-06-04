#pragma once
#include <QImage>
#include <QPointF>
QImage drawArrow(const QImage& img, int x1 /*line start horizontal position*/, int y1 /*line start vertical position*/, int x2 /*line end horizontal position*/, int y2 /*line end vertical position*/, int r = 255, int g = 255, int b = 255);  // From (x1,y1) to (x2,y2)

QImage drawRectangle(const QImage& img, int x1 /*start horizontal position*/, int y1 /*start vertical position*/, int x2 /*end horizontal position*/, int y2 /*end vertical position*/, int r = 255, int g = 255, int b = 255);  // From (x1,y1) to (x2,y2)

QImage drawPolyline(const QImage& img, const QVector<QPointF>& points, int r = 255, int g = 255, int b = 255,bool isClosed = false);