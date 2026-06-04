#ifndef MODEL_H
#define MODEL_H

#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include "Camera.h"
struct Vertex {
    QVector3D pos;
    QVector2D texture;
    QVector3D normal;
};
struct Vertex3{
    float x, z, y;//这样子的顺序是为了适应OpenGL中的坐标系
    Vertex3(const float& x=0.f, const float& y=0.f, const float& z=0.f) : x(x), y(y), z(z) {}
    bool operator==(const Vertex3& v) const {
        return x == v.x && y == v.y && z == v.z;
    }
};

class Model{
public:
    Model();
    ~Model();

public:
    void setScale(float val) { m_scale = val; }
    void setRotate(const QVector3D &rotate) { m_rotate = rotate; }
    void setPos(const QVector3D &pos) { m_pos = pos; }
    float scale() { return m_scale; }
    QVector3D rotate() { return m_rotate; }
    QVector3D pos() { return m_pos; }

public:
    void setVertices(const QVector<Vertex> &vertices) { m_vertices = vertices; }
    virtual void setTexture(QOpenGLTexture *texture, int index = -1);
    void setShaderProgram(QOpenGLShaderProgram *program) { m_program = program; }
    virtual void setQOpenGLFunctions(const QOpenGLExtraFunctions* f) {
        this->f = (QOpenGLExtraFunctions*)f;
    }

public:
    virtual void setCamera(Camera *camera) { m_camera = camera; }
	virtual void setProjection(const QMatrix4x4 &projection) { m_projection = projection; }
public:
    QMatrix4x4 model();

public:
    virtual void init();
    virtual void update();
    virtual void paint();
    virtual void updateViewPort(int w, int h) {}

protected:
    QVector3D m_pos{0, 0, 0};
    QVector3D m_rotate{0, 0, 0};
    float m_scale = 1;

    QVector<Vertex> m_vertices;
    QMap<int, QOpenGLTexture *> m_textures;

    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;
    QOpenGLShaderProgram *m_program = nullptr;

    QMatrix4x4 m_projection;
	Camera *m_camera = nullptr;
    QOpenGLExtraFunctions* f = nullptr;

};
#endif  // MODEL_H