#ifndef Object_H
#define Object_H

#include <QImage>
#include <QMap>
#include <QVector>

#include "Line.h"
#include "Model.h"
#include "Points.h"
/*
Object类仅仅用于显示一个object的数据
并且提供仅显示散点、仅显示线、仅显示网格的功能，以及他们的组合
并且提供点的颜色、线的颜色、网格的颜色的设置
并且提供点的大小、线的宽度的设置
并且提供设置是否闭合的功能

对于顶点的增删，应该在mainwindow中进行，然后每次切换到3d视图再实例化一次object或者调用object的函数
*/
class Object : public Model {
public:
    Object(const float& lineWidth = 1.0 /*线的宽度*/, const float& radius = 1 /*点的半径*/, const bool& closed = false /*是否闭合*/, const int& r = 252, const int& g = 220, const int& b = 77);

    ~Object() {}
    void setColor(const int& r, const int& g, const int& b) {  // 设置mesh的颜色
        m_r = r, m_g = g, m_b = b;
    }
    void setPointColor(const int& r, const int& g, const int& b) {  // 设置点的颜色
        m_point_r = r, m_point_g = g, m_point_b = b;
        m_points.setColor(r, g, b);
    }
    void setPointRadius(const float& radius) {  // 设置点的半径
        m_radius = radius;
        m_points.setRadius(radius);
    }
    void setLineColor(const int& r, const int& g, const int& b) {  // 设置线的颜色
        if (m_line_r == r && m_line_g == g && m_line_b == b)       // 因为更新线的代价比较大，所以如果颜色没有变化就不更新了
            return;
        m_line_r = r, m_line_g = g, m_line_b = b;
        updateLines();
    }
    void setLineWidth(const float& lineWidth) {
        if (m_lineWidth == lineWidth)  // 因为更新线的代价比较大，所以如果线宽没有变化就不更新了
            return;
        m_lineWidth = lineWidth;
        updateLines();
    }
    void setLineClosed(const bool& closed) {
        if (m_closed == closed)  // 因为更新线的代价比较大，所以如果闭合状态没有变化就不更新了
            return;
        m_closed = closed;
        updateLines();
    }
    void setData(const QMap<int, QVector<Vertex3>>& z_vertexArray) {
        this->z_vertexArray = z_vertexArray;
        if (m_drawMesh)
            toMesh();
        if (m_drawLines)
            toLines();
        if (m_drawPoints)
            toPoints();
    }
    void setDrawMesh(const bool& drawMesh) {
        m_drawMesh = drawMesh;
        if (m_drawMesh)
            toMesh();
    }
    void setDrawLines(const bool& drawLines) {
        m_drawLines = drawLines;
        if (m_drawLines) {
            toLines();
        }
    }
    void setDrawPoints(const bool& drawPoints) {
        m_drawPoints = drawPoints;
        if (m_drawPoints)
            toPoints();
    }
    void updateData() {  // 用于从2d编辑界面切换到3d视图时更新数据
        if (m_drawMesh)
            toMesh();
        if (m_drawLines)
            toLines();
        if (m_drawPoints)
            toPoints();
    }

public:

    virtual void setProjection(const QMatrix4x4& projection) override {  // 如果是通过基类指针调用，那么必须要加上virtual，否则只会调用基类的函数
        m_projection = projection;
        m_points.setProjection(projection);
        updateLines();

    }
    virtual void init() override;
    virtual void update() override;
    virtual void paint() override;
    virtual void setQOpenGLFunctions(const QOpenGLExtraFunctions* f) override {
        this->f = (QOpenGLExtraFunctions*)f;
        m_points.setQOpenGLFunctions(f);
        if (m_drawLines) {
            for (auto b = z_line.constBegin(); b != z_line.constEnd(); ++b) {
                b.value()->setQOpenGLFunctions(f);
            }
        }
        m_additional_line_1.setQOpenGLFunctions(f);  // 这里不能放进那个if里面
        m_additional_line_2.setQOpenGLFunctions(f);
    }
    virtual void updateViewPort(int w, int h) override {
        Q_ASSERT(f != nullptr);
        if (m_drawLines) {
            qDebug() << "updateViewPort 1";
            for (auto b = z_line.constBegin(); b != z_line.constEnd(); ++b) {
                b.value()->updateViewPort(w, h);
            }
            qDebug() << "updateViewPort 2";
            m_additional_line_1.updateViewPort(w, h);
            qDebug() << "updateViewPort 3";
            m_additional_line_2.updateViewPort(w, h);
            qDebug() << "updateViewPort 4";
        }
    }

private:
    void toMesh();
    void toLines();
    void toPoints();
    void updateLines();

private:
    const int VertexFloatCount = 3;
    bool inited = false;
    // 储存OpenGL绘制用到的数据
    QVector<Vertex3> m_meshVertexs;                                                          // 用于存储mesh的顶点,先存triangle_srtip,再存triangle_fan
    QVector<int> m_triangle_srtip_sizes, m_triangle_fan_sizes /*,m_additional_line_sizes*/;  // 用于存储mesh的顶点的个数
    QMap<int, Line*> z_line;                                                                 // key:z的取值 value:这个z值对应的线
    Points m_points;
    Line m_additional_line_1, m_additional_line_2;  // 用于存储mesh的边界线

public:
    float m_lineWidth = 1.0f;                                          // 线的宽度
    bool m_closed = false;                                             // 是否闭合
    float m_radius = 1.0f;                                             // 点的半径
    int m_r = 252, m_g = 220, m_b = 77;                                // mesh的颜色
    int m_line_r = 154, m_line_g = 3, m_line_b = 30;                   // 线的颜色
    int m_point_r = 95, m_point_g = 15, m_point_b = 64;                // 点的颜色
    QMap<int, QVector<Vertex3>> z_vertexArray;                         // 储存用户手动创建的点,key:z的取值 value:这个z值对应的顶点数组
    bool m_drawMesh = false, m_drawPoints = true, m_drawLines = true;  // 是否绘制mesh、点、线
};

#endif  // Object_H