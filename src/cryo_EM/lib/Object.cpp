#include "Object.h"

Object::Object(const float& lineWidth /*线的宽度*/, const float& radius /*点的半径*/, const bool& closed, const int& r, const int& g, const int& b) {
    m_lineWidth = lineWidth;
    m_closed = closed;
    setColor(r, g, b);
    setPointRadius(radius);
}

void Object::init() {
    if (inited) return;
    inited = true;
    Q_ASSERT(f != nullptr);

    const char* vertexShaderSource =  // 顶点着色器
        u8R"(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main(){
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";
    const char* fragmentShaderSource =  // 片段着色器
        u8R"(
#version 330 core
out vec4 fragColor;
uniform vec4 color;
void main()
{
fragColor = color;
}
)";

    auto _program = new QOpenGLShaderProgram();
    _program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    _program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    setShaderProgram(_program);

    if (!m_vao.isCreated())
        m_vao.create();
    if (!m_vbo.isCreated())
        m_vbo.create();
    if (!m_program->isLinked())
        m_program->link();

    if (m_drawMesh)
        this->update();
    m_additional_line_1.setQOpenGLFunctions(f);
    m_additional_line_2.setQOpenGLFunctions(f);
    m_additional_line_1.setCamera(m_camera);
    m_additional_line_2.setCamera(m_camera);
    m_additional_line_1.setColor(m_line_r, m_line_g, m_line_b);
    m_additional_line_2.setColor(m_line_r, m_line_g, m_line_b);
    m_additional_line_1.setLineWidth(m_lineWidth);
    m_additional_line_2.setLineWidth(m_lineWidth);
    m_additional_line_1.setClosed(m_closed);
    m_additional_line_2.setClosed(m_closed);
    m_additional_line_1.setProjection(m_projection);
    m_additional_line_2.setProjection(m_projection);
    m_additional_line_1.init();
    m_additional_line_2.init();
}

void Object::update() {  // 可能还是因为那个上下文导致没能把数据绑定到当前的上下文
    if (!inited) this->init();
    m_vao.bind();
    m_vbo.bind();
    m_vbo.allocate(m_meshVertexs.data(), sizeof(float) * m_meshVertexs.size() * 3);

    m_program->bind();
    // 绑定顶点坐标信息, 从0 * sizeof(float)字节开始读取3个float, 因为一个顶点有3个float数据, 所以下一个数据需要偏移3 * sizeof(float)个字节
    m_program->setAttributeBuffer("aPos", GL_FLOAT, 0 * sizeof(float), 3, VertexFloatCount * sizeof(float));
    m_program->enableAttributeArray("aPos");

    m_program->release();
}

void Object::paint() {
    Q_ASSERT(m_camera != nullptr);
    if (!inited) this->init();
    Q_ASSERT(f != nullptr);
    if (m_drawMesh) {
        m_vao.bind();
        m_program->bind();
        // 绑定变换矩阵
        m_program->setUniformValue("projection", m_projection);
        m_program->setUniformValue("view", m_camera->view());
        m_program->setUniformValue("model", model());
        m_program->setUniformValue("color", (float)m_r / 255.f, (float)m_g / 255.f, (float)m_b / 255.f, 1.0f);
        // 绘制
        int offset = 0;
        for (const int& size : m_triangle_srtip_sizes) {
            f->glDrawArrays(GL_TRIANGLE_STRIP, offset, size);
            offset += size;
        }
        for (const int& size : m_triangle_fan_sizes) {
            f->glDrawArrays(GL_TRIANGLE_FAN, offset, size);
            offset += size;
        }
        m_program->release();
        m_vao.release();
        if (m_drawLines) {
            m_additional_line_1.paint();
            m_additional_line_2.paint();
        }
    }
    if (m_drawLines) {
        for (auto b = z_line.constBegin(); b != z_line.constEnd(); ++b) {  // 避免创建list的消耗
            b.value()->paint();
        }
    }
    if (m_drawPoints) {
        m_points.paint();
    }
}

void Object::toMesh() {
    m_meshVertexs.clear();
    m_triangle_srtip_sizes.clear(), m_triangle_fan_sizes.clear();
    QVector<Vertex3> triangle_srtip_vertexs, triangle_fan_vertexs, additional_line_vertexs;

    QVector<int> zs = z_vertexArray.keys();  // 按照升序排列
    int size = 0;
    QVector<Vertex3> tmp;
    if (zs.empty())
        return;
    if (m_closed) {  // 闭合
        tmp = z_vertexArray[zs[0]];
        if (tmp.size() >= 3) {
            triangle_fan_vertexs.append(tmp);
            m_triangle_fan_sizes.append(tmp.size());
            auto first = tmp.constFirst();
            auto it = tmp.constBegin() + 1;
            while (it != tmp.constEnd()) {
                additional_line_vertexs.append(first);
                additional_line_vertexs.append(*it);
                ++it ;
            }
        }
        auto i1 = zs.constBegin(), i2 = zs.constBegin() + 1;
        while (i2 != zs.constEnd()) {
            auto ii1 = z_vertexArray[*i1].constBegin(), ii2 = z_vertexArray[*i2].constBegin(), ii3 = z_vertexArray[*i1].constEnd(), ii4 = z_vertexArray[*i2].constEnd();
            size = 0;
            while (ii1 != ii3 && ii2 != ii4) {
                triangle_srtip_vertexs.append(*ii1);
                triangle_srtip_vertexs.append(*ii2);
                ++ii1, ++ii2, size += 2;
            }
            //  现在，有一个是end了，另一个未必是end
            if (ii1 == ii3) {  // ii1到头了,要把ii2消耗完,注意不能把这里的if else省略,否则会死循环
                while (ii1 == ii3 && ii2 != ii4) {
                    ii1 = z_vertexArray[*i1].constBegin();
                    while (ii1 != ii3 && ii2 != ii4) {
                        triangle_srtip_vertexs.append(*ii1);
                        triangle_srtip_vertexs.append(*ii2);
                        ++ii1, ++ii2, size += 2;
                    }
                }
            } else if (ii2 == ii4) {  // ii2到头了,要把ii1消耗完
                while (ii2 == ii4 && ii1 != ii3) {
                    ii2 = z_vertexArray[*i2].constBegin();
                    while (ii1 != ii3 && ii2 != ii4) {
                        triangle_srtip_vertexs.append(*ii1);
                        triangle_srtip_vertexs.append(*ii2);
                        ++ii1, ++ii2, size += 2;
                    }
                }
            }
            //  现在，两个都到头了，要闭合
            triangle_srtip_vertexs.append(z_vertexArray[*i1].first());
            triangle_srtip_vertexs.append(z_vertexArray[*i2].first());
            size += 2;
            m_triangle_srtip_sizes.append(size);
            size = 0;
            ++i1, ++i2;
        }
        tmp = z_vertexArray[zs.last()];
        if (tmp.size() >= 3) {
            triangle_fan_vertexs.append(tmp);
            m_triangle_fan_sizes.append(tmp.size());
            auto first = tmp.constFirst();
            auto it = tmp.constBegin() + 1;
            while (it != tmp.constEnd()) {
                additional_line_vertexs.append(first);
                additional_line_vertexs.append(*it);
                ++it ;
            }
        }

    } else {  // 不闭合
        tmp = z_vertexArray[zs[0]];
        if (tmp.size() >= 3) {
            triangle_srtip_vertexs.append(tmp);
            m_triangle_srtip_sizes.append(tmp.size());
        }
        auto i1 = zs.constBegin(), i2 = zs.constBegin() + 1;
        while (i2 != zs.constEnd()) {
            auto ii1 = z_vertexArray[*i1].constBegin(), ii2 = z_vertexArray[*i2].constBegin(), ii3 = z_vertexArray[*i1].constEnd(), ii4 = z_vertexArray[*i2].constEnd();
            size = 0;
            while (ii1 != ii3 && ii2 != ii4) {
                triangle_srtip_vertexs.append(*ii1);
                triangle_srtip_vertexs.append(*ii2);
                ++ii1, ++ii2, size += 2;
            }
            m_triangle_srtip_sizes.append(size);
            size = 0;
            // 现在，有一个是end了，另一个未必是end
            if (ii1 == ii3) {  // ii1到头了
                --ii1, --ii2;
                auto first = *ii1;
                triangle_fan_vertexs.append(*ii1);
                ++size;
                while (ii2 != ii4) {
                    triangle_fan_vertexs.append(*ii2);
                    additional_line_vertexs.append(first);
                    additional_line_vertexs.append(*ii2);
                    ++ii2, ++size;
                }
                m_triangle_fan_sizes.append(size);
                size = 0;
            } else if (ii2 == ii4) {  // ii2到头了
                --ii1, --ii2;
                auto first = *ii2;
                triangle_fan_vertexs.append(*ii2);
                ++size;
                while (ii1 != ii3) {
                    triangle_fan_vertexs.append(*ii1);
                    additional_line_vertexs.append(first);
                    additional_line_vertexs.append(*ii1);
                    ++ii1, ++size;
                }
                m_triangle_fan_sizes.append(size);
                size = 0;
            }
            ++i1, ++i2;
        }
        tmp = z_vertexArray[zs.last()];
        if (tmp.size() >= 3) {
            triangle_srtip_vertexs.append(tmp);
            m_triangle_srtip_sizes.append(tmp.size());
        }
    }
    m_meshVertexs.append(triangle_srtip_vertexs);  // 先存triangle_srtip,再存triangle_fan,要和paint中的顺序一致
    m_meshVertexs.append(triangle_fan_vertexs);
    m_additional_line_1.setVertexOnly(additional_line_vertexs);
    m_additional_line_2.setVertexOnly(triangle_srtip_vertexs);
    this->update();
}

void Object::toLines() {
    Q_ASSERT(m_camera != nullptr);
    Q_ASSERT(f != nullptr);

    for (auto i = z_vertexArray.constBegin(); i != z_vertexArray.constEnd(); ++i) {
        if (z_line.contains(i.key())) {
            z_line[i.key()]->setVertexOnly(i.value());
        } else {
            auto l = new Line(m_lineWidth, m_closed, m_line_r, m_line_g, m_line_b);
            l->setQOpenGLFunctions(f);
            l->setCamera(m_camera);
            l->setProjection(m_projection);
            l->setVertexOnly(i.value());
            z_line.insert(i.key(), l);
        }
    }
    m_additional_line_1.setQOpenGLFunctions(f);
    m_additional_line_2.setQOpenGLFunctions(f);
    m_additional_line_1.setCamera(m_camera);
    m_additional_line_2.setCamera(m_camera);
    m_additional_line_1.setColor(m_line_r, m_line_g, m_line_b);
    m_additional_line_2.setColor(m_line_r, m_line_g, m_line_b);
    m_additional_line_1.setLineWidth(m_lineWidth);
    m_additional_line_2.setLineWidth(m_lineWidth);
    m_additional_line_1.setClosed(m_closed);
    m_additional_line_2.setClosed(m_closed);
    m_additional_line_1.setProjection(m_projection);
    m_additional_line_2.setProjection(m_projection);
}

void Object::toPoints() {
    Q_ASSERT(m_camera != nullptr);
    QVector<Vertex3> points;
    for (auto i = z_vertexArray.constBegin(); i != z_vertexArray.constEnd(); ++i) {
        points.append(i.value());
    }
    m_points.setCamera(m_camera);
    m_points.setVertexOnly(points);
    m_points.setColor(m_point_r, m_point_g, m_point_b);
    m_points.setRadius(m_radius);
}

void Object::updateLines() {
    if (m_drawLines) {
        for (auto b = z_line.constBegin(); b != z_line.constEnd(); ++b) {
            b.value()->setProjection(m_projection);
            b.value()->setColor(m_line_r, m_line_g, m_line_b);
            b.value()->setLineWidth(m_lineWidth);
            b.value()->setClosed(m_closed);
        }
        m_additional_line_1.setProjection(m_projection);
        m_additional_line_2.setProjection(m_projection);
        m_additional_line_1.setColor(m_line_r, m_line_g, m_line_b);
        m_additional_line_2.setColor(m_line_r, m_line_g, m_line_b);
        m_additional_line_1.setLineWidth(m_lineWidth);
        m_additional_line_2.setLineWidth(m_lineWidth);
        m_additional_line_1.setClosed(m_closed);
        m_additional_line_2.setClosed(m_closed);
    }
}