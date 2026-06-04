#ifndef OPENGLWIDGET2_H
#define OPENGLWIDGET2_H

#include <QOpenGLExtraFunctions>
#include <QOpenGLWidget>

#include "Camera.h"
#include "Line.h"
#include "Object.h"
#include "Points.h"
#include "Rectangle.h"
class OpenGLWidget2 : public QOpenGLWidget, public QOpenGLExtraFunctions {
    Q_OBJECT

public:
    OpenGLWidget2(QWidget *parent = nullptr);
    ~OpenGLWidget2();
    void setMaxXYZ(int x, int y, int z) {
        max_x = x / 2;
        max_y = y / 2;
        max_z = z / 2;
    }
    void setObjects(const QVector<Object *> &objects) {
        qDebug() << "setObjects";
        makeCurrent();
        m_models.clear();//这里没必要delete那些指针，因为这些指针指向的是同一个对象，在删除object的时候就已经对那些无用的对象进行清理了。如果这里进行delete，就可能导致下面这个for循环访问到空指针。
        for (const auto &object : objects) {
            object->setQOpenGLFunctions(this->context()->extraFunctions());
            object->setCamera(&m_camera);
            object->updateData();
            object->setProjection(m_projection);//不要把这行放到updateData前面，否则线条显示会出问题
            object->init();
            object->updateViewPort(viewPort_w, viewPort_h);
            m_models.emplace_back(object);
        }
        doneCurrent();
    }
    void setShowSlice(bool show_slice) {
        m_show_slice = show_slice;
    }
    void updateXYImage(const QImage &image, const int &z) {
        float x = image.width(), y = image.height(), fz = z;
        makeCurrent();
        xy_slice->setTexture(image);
        xy_slice->setVertices({
            // 这里统一规定图片左上角为原点，坐标轴往右边和下边延申，并且交换了y和z坐标轴
            //  顶点    纹理
            {{0, fz, 0}, {0, 1}},  // 左上
            {{0, fz, y}, {0, 0}},  // 左下
            {{x, fz, y}, {1, 0}},  // 右下
            {{x, fz, 0}, {1, 1}}   // 右上
        });
        xy_slice->update();
        doneCurrent();
    }
    void updateXZImage(const QImage &image, const int &y) {
        float x = image.width(), z = image.height(), fy = y;
        makeCurrent();
        xz_slice->setTexture(image);
        xz_slice->setVertices({
            //  顶点    纹理
            {{0, 0, fy}, {0, 1}},  // 左上
            {{0, z, fy}, {0, 0}},  // 左下
            {{x, z, fy}, {1, 0}},  // 右下
            {{x, 0, fy}, {1, 1}}   // 右上
        });
        xz_slice->update();
        doneCurrent();
    }
    void updateYZImage(const QImage &image, const int &x){
        float z = image.width(), y = image.height(), fx = x;
        makeCurrent();
        yz_slice->setTexture(image);
        yz_slice->setVertices({
            //  顶点    纹理
            {{fx, 0, 0}, {0, 1}},  // 左上
            {{fx, 0, y}, {0, 0}},  // 左下
            {{fx, z, y}, {1, 0}},  // 右下
            {{fx, z, 0}, {1, 1}}   // 右上
        });
        yz_slice->update();
        doneCurrent();
    }

protected:
    virtual void initializeGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;
    virtual void timerEvent(QTimerEvent *event);

private:
    QMatrix4x4 m_projection;
    Camera m_camera;
    QVector<Model *> m_models;
    int x = 0, y = 0, z = 0;                                      // 取值范围是-1到1
    float rotate_x_deg = 90, rotate_y_deg = 0, rotate_z_deg = 0;  // 取值范围是0到360
    int max_x = 1, max_y = 1, max_z = 1;
    bool m_show_slice = false;
    class Rectangle *xy_slice = nullptr, *xz_slice = nullptr, *yz_slice = nullptr;
    int viewPort_w=800,  viewPort_h=600;

public slots:

signals:
    void xChanged(int x);
    void yChanged(int y);
    void zChanged(int z);
    void rotateXChanged(int rotate_x_deg);
    void rotateYChanged(int rotate_y_deg);
    void rotateZChanged(int rotate_z_deg);
};

#endif  // OPENGLWIDGET2_H
