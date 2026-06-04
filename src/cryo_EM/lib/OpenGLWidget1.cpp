#include "OpenGLWidget1.h"

#include <QDebug>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QOpenGLDebugLogger>
#include <QPainter>
#include <limits>

OpenGLWidget1::OpenGLWidget1(QWidget *parent)
    : QOpenGLWidget(parent) {
    {  // 设置OpenGL版本,要和那些shader的版本一致
        QSurfaceFormat fmt;
        fmt.setDepthBufferSize(24);
        // Request OpenGL 3.3 core or OpenGL ES 3.0.
        if (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGL) {
            qDebug("Requesting 3.3 core context in LibGL mode");
            fmt.setVersion(3, 3);
            fmt.setProfile(QSurfaceFormat::CoreProfile);  // 早期 OpenGL 版本的功能不可用
        } else {
            qDebug("Requesting 3.0 context in GLES mode may cause crashes on some devices");
            fmt.setVersion(3, 3);
            fmt.setProfile(QSurfaceFormat::CoreProfile);  // 早期 OpenGL 版本的功能不可用
        }
        fmt.setSamples(16);                                  // 设置多重采样, 用于抗锯齿
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
    // m_camera.move(1.752, 1.8, 0.133);//左侧是设置的xyz，实际的xyz：1.8,0.133,-1.752
    m_camera.move(-0.133, 1.752, 1.8);
    m_camera.look(323.2, 37, 0);
    m_camera.update();
    this->setFocusPolicy(Qt::StrongFocus);  // 设置焦点，否则无法在用作小部件的时候接收键盘事件
    installEventFilter(&m_camera);
}

OpenGLWidget1::~OpenGLWidget1() {
}

void OpenGLWidget1::initializeGL() {
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);  // 必须在initializeOpenGLFunctions()之后调用，否则会报错

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(254, 254, 254, 1.0f);
    QImage texture(":/texture/lib/rectangle.png");
    {
        auto _rectangle = new class Rectangle(texture);
        _rectangle->setQOpenGLFunctions(this->context()->extraFunctions());
        _rectangle->setCamera(&m_camera);
        _rectangle->setPos({0, -1, -2});  // 下面
        _rectangle->setRotate({90, 0, 0});
        _rectangle->init();
        m_models << _rectangle;
    }
    {
        auto _rectangle = new class Rectangle(texture);
        _rectangle->setQOpenGLFunctions(this->context()->extraFunctions());
        _rectangle->setCamera(&m_camera);
        _rectangle->setPos({0, 0, -3});  // 背后
        _rectangle->setRotate({0, 0, 0});
        _rectangle->init();
        m_models << _rectangle;
    }

    {
        auto _rectangle = new class Rectangle(texture);
        _rectangle->setQOpenGLFunctions(this->context()->extraFunctions());
        _rectangle->setCamera(&m_camera);
        _rectangle->setPos({-1, 0, -2});  // 左边
        _rectangle->setRotate({0, 90, 0});
        _rectangle->init();
        m_models << _rectangle;
    }

    {
        auto _rectangle = new class Rectangle(texture);
        _rectangle->setQOpenGLFunctions(this->context()->extraFunctions());
        _rectangle->setCamera(&m_camera);
        _rectangle->setPos({1, 0, -2});  // 右边
        _rectangle->setRotate({0, 90, 0});
        _rectangle->init();
        m_models << _rectangle;
    }
    {
        auto _rectangle = new class Rectangle(texture);
        _rectangle->setQOpenGLFunctions(this->context()->extraFunctions());
        _rectangle->setCamera(&m_camera);
        _rectangle->setPos({0, 1, -2});  // 上面
        _rectangle->setRotate({90, 0, 0});
        _rectangle->init();
        m_models << _rectangle;
    }
    {
        auto _rectangle = new class Rectangle(texture);
        _rectangle->setQOpenGLFunctions(this->context()->extraFunctions());
        _rectangle->setCamera(&m_camera);
        _rectangle->setPos({0, 0, -1});    // 必须要往屏幕里面移动，否则设置了perspective就会看不到那些在屏幕外面的点以及恰好和屏幕相交的点
        _rectangle->setRotate({0, 0, 0});  // 正面
        _rectangle->init();
        m_models << _rectangle;
    }  // 上面的正方体的中心是0，0，-1，边长是2
    {
        QImage texture(1, 1, QImage::Format_ARGB32);
        texture.setPixel(0, 0, qRgba(205, 92, 92, 150));
        auto _rectangle = new class Rectangle(texture);
        _rectangle->setQOpenGLFunctions(this->context()->extraFunctions());
        _rectangle->setCamera(&m_camera);
        _rectangle->setPos({0, 0, 0 - 2});
        _rectangle->setRotate({rotate_x_deg, rotate_y_deg, rotate_z_deg});
        _rectangle->setScale(1.3);
        _rectangle->init();
        clip_plane = _rectangle;
    }
}

void OpenGLWidget1::resizeGL(int w, int h) {
    m_projection.setToIdentity();
    m_projection.perspective(60, (float)w / h, 0.001, 1000);
    for (auto model : m_models) {
        model->setProjection(m_projection);
    }
    clip_plane->setProjection(m_projection);
}

void OpenGLWidget1::paintGL() {
    glClearColor(254, 254, 254, 1.0f);                   // 似乎只保留这4行才能正常表达透明效果
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // 清除之前绘制的内容，包含颜色与深度信息
    m_models[0]->paint();  // 下面
    clip_plane->paint();
    m_models[1]->paint();  // 背后
    m_models[2]->paint();  // 左边
    clip_plane->paint();
    m_models[3]->paint();  // 右边
    m_models[4]->paint();  // 上面
    m_models[5]->paint();  // 正面
    clip_plane->paint();
}
void OpenGLWidget1::timerEvent(QTimerEvent *event) {
    m_camera.update();  // 更新摄像机,否则摄像机的位置和角度不会变化
    repaint();          // 重绘
}
