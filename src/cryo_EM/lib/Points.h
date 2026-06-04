#ifndef Points_H
#define Points_H

#include <QImage>
#include <QVector>

#include "Model.h"
class Points : public Model {
public:
    Points(const float& radius = 1, const int& r = 205, const int& g = 92, const int& b = 92);

    ~Points() {
        if (m_vertexBuffer != nullptr)
            delete[] m_vertexBuffer;
    }
    void setOffsets(const QVector<Vertex3>& points) {
        m_points = points;
        num_instances = points.size();
        this->update();
    }
    void setVertexOnly(const QVector<Vertex3>& points) {
        this->setOffsets(points);
    }
    void setRadius(const float& radius) {
        m_radius = radius;
    }
    void setColor(const int& r, const int& g, const int& b) {
        if (m_textures.contains(0))
            delete m_textures[0];
        QImage texture(1, 1, QImage::Format_BGR888);
        texture.setPixel(0, 0, qRgb(r, g, b));
        auto _texture = new QOpenGLTexture(texture);
        _texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
        _texture->setMagnificationFilter(QOpenGLTexture::Linear);
        m_textures[0] = _texture;
    }
    

public:
    virtual void init() override;
    virtual void update() override;
    virtual void paint() override;

private:
    void initSphere();

private:
    float* m_vertexBuffer = nullptr;
    int m_vertexCount = 0;
    QOpenGLBuffer m_instance_vbo;
    int num_instances = 0;
    const int VertexFloatCount = 5;
    // 地球仪画法用到的
    int m_row = 0;
    int m_col = 0;
    float m_radius = -1;
    bool inited = false;
    QVector<Vertex3> m_points;
};

#endif  // Points_H