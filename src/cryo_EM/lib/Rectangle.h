#ifndef Rectangle_H
#define Rectangle_H

#include <QImage>

#include "Model.h"

class Rectangle : public Model {
public:
    Rectangle(const QImage& texture=QImage(1,1,QImage::Format_Grayscale8));

    ~Rectangle() {}
    void setTexture(const QImage& texture);

public:
    virtual void init() override;
    virtual void update() override;
    virtual void paint() override;
    virtual void setTexture(QOpenGLTexture *texture, int index = -1) override;

private:
    float* m_vertexBuffer = nullptr;
    int m_vertexCount = 0;
    const int VertexFloatCount = 5;
};

#endif  // Rectangle_H