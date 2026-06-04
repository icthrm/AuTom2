#include "OpenGLWidget2.h"

#include <QDebug>
#include <QFont>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QOpenGLDebugLogger>
#include <QPainter>
#include <QPainterPath>
#include <limits>
OpenGLWidget2::OpenGLWidget2(QWidget *parent)
    : QOpenGLWidget(parent) {
    {  // Set OpenGL version, must match shader versions
        QSurfaceFormat fmt;
        fmt.setDepthBufferSize(24);
        // Request OpenGL 3.3 core or OpenGL ES 3.0.
        if (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGL) {
            qDebug("Requesting 3.3 core context in LibGL mode");
            fmt.setVersion(3, 3);
            fmt.setProfile(QSurfaceFormat::CoreProfile);//早期 OpenGL 版本的功能不可用
        } else {
            qDebug("Requesting 3.3 context in GLES mode may cause crashes on some devices");
            fmt.setVersion(3, 0);
            fmt.setProfile(QSurfaceFormat::CoreProfile);//早期 OpenGL 版本的功能不可用
        }
        fmt.setSamples(16);  // 设置多重采样, 用于抗锯齿
        this->setFormat(fmt);
    }
    {
        //显示当前QSurfaceFormat
        auto fmt = this->format();
        qDebug() << "QSurfaceFormat:" << fmt;
        qDebug() << "QSurfaceFormat::Version:" << fmt.version();
        qDebug() << "QSurfaceFormat::Profile:" << fmt.profile();
        qDebug() << "QSurfaceFormat::Options:" << fmt.options();
        qDebug() << "QSurfaceFormat::majorVersion:" << fmt.majorVersion();
        qDebug() << "QSurfaceFormat::minorVersion:" << fmt.minorVersion();

    }
    startTimer(1000 / 60);  // 60fps
                            // m_camera.move(-6, 0, 3);
                            // m_camera.look(0, 30, 0);
                            // m_camera.move(1.752, 1.8, 0.133);//左侧是设置的xyz，实际的xyz：1.8,0.133,-1.752(-z,x,y)
    m_camera.move(61.358, -43.054, 83.125);
    m_camera.look(142.499, 11.9, 0);

    m_camera.update();
    this->setFocusPolicy(Qt::StrongFocus);  // 设置焦点，否则无法在用作小部件的时候接收键盘事件
    installEventFilter(&m_camera);
}

OpenGLWidget2::~OpenGLWidget2() {
}
void OpenGLWidget2::initializeGL() {
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);  // 必须在initializeOpenGLFunctions()之后调用，否则会报错

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(254, 254, 254, 1.0f);
    {
        xy_slice = new class Rectangle();
        xy_slice->setQOpenGLFunctions(this->context()->extraFunctions());
        xy_slice->setCamera(&m_camera);
        xy_slice->init();
    }
    {
        xz_slice = new class Rectangle();
        xz_slice->setQOpenGLFunctions(this->context()->extraFunctions());
        xz_slice->setCamera(&m_camera);  // 左边
        xz_slice->init();
    }
    {
        yz_slice = new class Rectangle();
        yz_slice->setQOpenGLFunctions(this->context()->extraFunctions());
        yz_slice->setCamera(&m_camera);  // 正面
        yz_slice->init();
    }
}

void OpenGLWidget2::resizeGL(int w, int h) {
    qDebug() << "OpenGLWidget2::resizeGL " << w << ' ' << h;
    m_projection.setToIdentity();
    m_projection.perspective(60, (float)w / h, 0.001, 1000);
    for (auto model : m_models) {
        model->setProjection(m_projection);
        model->updateViewPort(w, h);
    }
    xy_slice->setProjection(m_projection);
    xz_slice->setProjection(m_projection);
    yz_slice->setProjection(m_projection);
    viewPort_w = w, viewPort_h = h;
}

void OpenGLWidget2::paintGL() {
    glEnable(GL_DEPTH_TEST);
    glClearColor(254, 254, 254, 1.0f);                   // 似乎只保留这4行才能正常表达透明效果
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // 清除之前绘制的内容，包含颜色与深度信息
    for (auto model : m_models) {
        model->paint();
    }
    if (m_show_slice) {
        xy_slice->paint();
        xz_slice->paint();
        yz_slice->paint();
    }

    glDisable(GL_DEPTH_TEST);
    QPainter _painter(this);  // 开始绘制十字准星
    auto _rect = this->rect();
    _painter.setPen(Qt::green);
    _painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    _painter.drawLine(_rect.center() + QPoint{0, 5}, _rect.center() + QPoint{0, 15});
    _painter.drawLine(_rect.center() + QPoint{0, -5}, _rect.center() + QPoint{0, -15});
    _painter.drawLine(_rect.center() + QPoint{5, 0}, _rect.center() + QPoint{15, 0});
    _painter.drawLine(_rect.center() + QPoint{-5, 0}, _rect.center() + QPoint{-15, 0});
    QString txt1 = QString(u8"Camera Position: (%1, %2, %3)")
                       .arg(m_camera.pos().x(), 0, 'f', 3)
                       .arg(m_camera.pos().y(), 0, 'f', 3)
                       .arg(m_camera.pos().z(), 0, 'f', 3);
    QString txt2 = QString(u8"Camera Angle: (%1, %2, %3)")
                       .arg(m_camera.yaw(), 0, 'f', 3)
                       .arg(m_camera.pitch(), 0, 'f', 3)
                       .arg(m_camera.roll(), 0, 'f', 3);
    QPainterPath path;
    QFont font("Microsoft YaHei", 10);
    path.addText(QPoint{5, 15}, font, txt1);
    path.addText(QPoint{5, 30}, font, txt2);
    _painter.strokePath(path, QPen(Qt::black, 2));
    _painter.fillPath(path, Qt::white);
}
void OpenGLWidget2::timerEvent(QTimerEvent *event) {
    m_camera.update();  // 更新摄像机,否则摄像机的位置和角度不会变化
    repaint();          // 重绘
}
