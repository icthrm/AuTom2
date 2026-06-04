#include "drawSymbol.h"

#include <QLineF>
#include <QPainter>
#include <QPen>
#include <cmath>
// 求得箭头两点坐标
void CalcVertexes(double startX, double startY, double endX, double endY, double& x1, double& y1, double& x2, double& y2) {
    double arrowLength = 10;    // 箭头长度，一般固定
    double arrowDegrees = 0.5;  // 箭头角度，一般固定
    // 求 y / x 的反正切值
    double angle = atan2(endY - startY, endX - startX) + 3.1415926;
    // 求得箭头点 1 的坐标
    x1 = endX + arrowLength * cos(angle - arrowDegrees);
    y1 = endY + arrowLength * sin(angle - arrowDegrees);
    // 求得箭头点 2 的坐标
    x2 = endX + arrowLength * cos(angle + arrowDegrees);
    y2 = endY + arrowLength * sin(angle + arrowDegrees);
}
QImage drawArrow(const QImage& img, int x1, int y1, int x2, int y2, int r, int g, int b) {
    QImage image = img;
    QPainter painter(&image);
    QPen pen;
    pen.setColor(QColor(r, g, b));
    painter.setPen(pen);
    QLineF line(x1, y1, x2, y2);
    painter.drawLine(line);
    // 箭头的两点坐标
    double x11, y11, x22, y22;
    // 求得箭头两点坐标
    CalcVertexes(x1, y1, x2, y2, x11, y11, x22, y22);
    painter.drawLine(x2, y2, x11, y11);  // 绘制箭头一半
    painter.drawLine(x2, y2, x22, y22);  // 绘制箭头另一半
    painter.end();
    return image;
}

QImage drawRectangle(const QImage& img, int x1, int y1, int x2, int y2, int r, int g, int b){
    QImage image = img;
    QPainter painter(&image);
    QPen pen;
    pen.setColor(QColor(r, g, b));
    painter.setPen(pen);
    painter.drawRect(x1, y1, x2-x1, y2-y1);
    painter.end();
    return image;
}

QImage drawPolyline(const QImage& img, const QVector<QPointF>& points, int r, int g, int b,bool isClosed){
    QVector<QPointF> points2 = points;
    if(isClosed){
        points2.emplaceBack(points2.at(0));
    }
    QImage image = img;
    QPainter painter(&image);
    QPen pen;
    pen.setColor(QColor(r, g, b));
    painter.setPen(pen);
    painter.drawPolyline(points2.data(),points2.size());
    painter.end();
    return image;
}
