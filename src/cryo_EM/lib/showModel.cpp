#include "showModel.h"
void drawModel(QImage& img, const QVector<Object*>& objects, const int& center_z) {
    QPainter painter(&img);  // 只能用一个painter
    for (const auto& obj_ptr : objects) {
        const Object& obj = *obj_ptr;
        if (obj.z_vertexArray.contains(center_z)) {
            const bool drawPoints = obj.m_drawPoints, drawLines = obj.m_drawLines;
            const auto& vertex3s = obj.z_vertexArray[center_z];
            QVector<QPoint> points;
            points.reserve(vertex3s.size());
            for (const Vertex3& vertex : vertex3s) {
                points.emplace_back(vertex.x, vertex.y);
            }
            if (obj.m_closed) {
                if (drawLines && !vertex3s.empty())
                    points.emplace_back(vertex3s[0].x, vertex3s[0].y);
            }
            if (drawPoints) {
                painter.setPen(QPen(QColor(obj.m_point_r, obj.m_point_g, obj.m_point_b), obj.m_radius * 2));  // 乘2是因为这个参数设置的是宽度，这个画的是实心点。如果要花圆圈要用drawEllipse
                painter.drawPoints(points.data(), points.size());
            }
            if (drawLines) {
                painter.setPen(QPen(QColor(obj.m_line_r, obj.m_line_g, obj.m_line_b), obj.m_lineWidth));
                painter.drawPolyline(points.data(), points.size());
            }
        }
    }
    painter.end();
}
