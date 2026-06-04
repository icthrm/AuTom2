#ifndef OPENGLWIDGET1_H
#define OPENGLWIDGET1_H

#include <QOpenGLExtraFunctions>
#include <QOpenGLWidget>

#include "Camera.h"
#include "Rectangle.h"

class OpenGLWidget1 : public QOpenGLWidget, public QOpenGLExtraFunctions {
    Q_OBJECT

public:
    OpenGLWidget1(QWidget *parent = nullptr);
    ~OpenGLWidget1();
    void setMaxXYZ(int x, int y, int z) {
        max_x = x / 2;
        max_y = y / 2;
        max_z = z / 2;
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
    Model *clip_plane = nullptr;
    int x = 0, y = 0, z = 0;                                      // 取值范围是-1到1
    float rotate_x_deg = 90, rotate_y_deg = 0, rotate_z_deg = 0;  // 取值范围是0到360
    int max_x = 1, max_y = 1, max_z = 1;

public slots:
    void setX(int x) {
        if (x != this->x) {
            this->x = x;
            if (clip_plane != nullptr)
                clip_plane->setPos({float((double)x / max_x - 1.0), float((double)y / max_y - 1.0), (float)(double(z) / max_z - 3.0)});
            emit xChanged(x);
        }
    }
    void setY(int y) {
        if (y != this->y) {
            this->y = y;
            if (clip_plane != nullptr)
                clip_plane->setPos({float((double)x / max_x - 1.0), float((double)y / max_y - 1.0), (float)(double(z) / max_z - 3.0)});
            emit yChanged(y);
        }
    }
    void setZ(int z) {
        if (z != this->z) {
            this->z = z;
            if (clip_plane != nullptr)
                clip_plane->setPos({float((double)x / max_x - 1.0), float((double)y / max_y - 1.0), (float)(double(z) / max_z - 3.0)});
            emit zChanged(z);
        }
    }
    void setRotateX(int rotate_x_deg) {
        rotate_x_deg += 90;
        if (rotate_x_deg != this->rotate_x_deg) {
            this->rotate_x_deg = rotate_x_deg;
            if (clip_plane != nullptr)
                clip_plane->setRotate({(float)rotate_x_deg, (float)rotate_y_deg, (float)rotate_z_deg});
            emit rotateXChanged(rotate_x_deg);
        }
    }
    void setRotateY(int rotate_y_deg) {
        if (rotate_y_deg != this->rotate_y_deg) {
            this->rotate_y_deg = rotate_y_deg;
            if (clip_plane != nullptr)
                clip_plane->setRotate({(float)rotate_x_deg, (float)rotate_y_deg, (float)rotate_z_deg});
            emit rotateYChanged(rotate_y_deg);
        }
    }
    void setRotateZ(int rotate_z_deg) {
        if (rotate_z_deg != this->rotate_z_deg) {
            this->rotate_z_deg = rotate_z_deg;
            if (clip_plane != nullptr)
                clip_plane->setRotate({(float)rotate_x_deg, (float)rotate_y_deg, (float)rotate_z_deg});
            emit rotateZChanged(rotate_z_deg);
        }
    }
signals:
    void xChanged(int x);
    void yChanged(int y);
    void zChanged(int z);
    void rotateXChanged(int rotate_x_deg);
    void rotateYChanged(int rotate_y_deg);
    void rotateZChanged(int rotate_z_deg);
};

#endif  // OPENGLWIDGET1_H
