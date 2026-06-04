#ifndef Line_H
#define Line_H

#include <QImage>

#include "Model.h"

class Line : public Model {
public:
    Line(const float& lineWidth = 1.0 /*line width*/, const bool& closed = false /*whether closed*/, const int& r = 205, const int& g = 92, const int& b = 92);

    ~Line() {}
    void setVertexOnly(const QVector<Vertex3>& points) {
        m_points_open = m_points_closed = points;
        // Must have at least 1 point
        if (!points.empty()) {
            auto fir = points.front(), las = points.back();
            m_points_closed.emplaceBack(fir);
            m_points_closed.emplaceBack(fir);
            m_points_closed.emplaceFront(las);
            m_points_closed.emplaceFront(las);

            m_points_open.emplaceFront(fir);
            m_points_open.emplaceBack(las);
        }
        if (m_closed)
            m_points = m_points_closed;
        else
            m_points = m_points_open;
        m_vertexCount = m_points.size();
        this->update();
    }
    void setColor(const int& r, const int& g, const int& b) {
        m_r = (float)r / 255.0f, m_g = (float)g / 255.0f, m_b = (float)b / 255.0f;
    }
    void setLineWidth(const float& lineWidth) {
        m_lineWidth = lineWidth;
    }
    void setClosed(const bool& closed) {
        if (m_closed == closed) return;
        if (closed)
            m_points = m_points_closed;
        else
            m_points = m_points_open;
        m_vertexCount = m_points.size();
        m_closed = closed;
        this->update();
    }

public:
    virtual void init() override;
    virtual void update() override;
    virtual void paint() override;
    virtual void updateViewPort(int w, int h) override {
        Q_ASSERT(f != nullptr);
        Q_ASSERT(m_program != nullptr);
        //if(!inited) this->init();
        m_program->bind();
        m_program->setUniformValue("Viewport", float(w), float(h));
        m_program->release();
    }

private:
    int m_vertexCount = 0;
    const int VertexFloatCount = 3;
    float m_lineWidth = 1.0f;
    bool m_closed = false;
    bool inited = false;
    QVector<Vertex3> m_points, m_points_closed, m_points_open;
    float m_r = 0.0f, m_g = 0.0f, m_b = 0.0f;
};

#endif  // Line_H