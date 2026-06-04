#include "Points.h"
/*
X坐标轴从左至右，Y坐标轴从下至上，Z坐标轴从里至外。
OpenGL屏幕中心的坐标值是X和Y轴上的0.0点。
中心左面的坐标值是负值，右面是正值。移向屏幕顶端是正值，移向屏幕底端是负值。
移入屏幕深处是负值，移出屏幕则是正值。
*/
Points::Points(const float& radius, const int& r, const int& g, const int& b) {
    QImage texture(1, 1, QImage::Format_BGR888);
    texture.setPixel(0, 0, qRgb(r, g, b));
    auto _texture = new QOpenGLTexture(texture);
    _texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    _texture->setMagnificationFilter(QOpenGLTexture::Linear);
    setTexture(_texture);
    m_radius = radius;
}
void Points::initSphere() {
    QVector4D _top{0, 1, 0, 1};
    int _step = 10;  // 默认步长为1(最精细的球体)
    /*使用一个向量，指向北极，也就是（0，radius，0），然后，首先让它沿着x轴旋转，再沿着y轴旋转。
    x轴旋转10°是什么意思呢？默认我们的向量指向北极，转10°就意味着靠近赤道了10°。
    显然，此时，我们的向量指向的地方就是就是北纬80°，东经0°！*/
    QVector<QVector<QVector3D>> _vertexMatrix;

    for (int _yaw = 0; _yaw <= 180; _yaw += _step) {
        _vertexMatrix << QVector<QVector3D>();
        m_col = 0;
        for (int _pitch = 0; _pitch < 360; _pitch += _step) {
            QMatrix4x4 _mat;
            _mat.setToIdentity();
            _mat.rotate(_yaw, 1, 0, 0);
            _mat.rotate(-_pitch, 0, 1, 0);
            QVector3D _p = QVector3D(_top * _mat);
            _vertexMatrix[m_row] << _p;
            ++m_col;
        }
        ++m_row;
    }
    m_vertices.clear();
    for (int y = 0; y < m_row - 1; ++y) {
        for (int x = 0; x < m_col; ++x) {
            auto _p0 = _vertexMatrix[y][x];
            auto _p1 = _vertexMatrix[y + 1][x];
            int _nextX = x + 1;
            if (_nextX == m_col) {
                _nextX = 0;
            }
            auto _p2 = _vertexMatrix[y + 1][_nextX];
            auto _p3 = _vertexMatrix[y][_nextX];
            m_vertices << Vertex{{_p0.x(), _p0.y(), _p0.z()}, {(float)x / m_col, (float)y / m_row}}
                       << Vertex{{_p1.x(), _p1.y(), _p1.z()}, {(float)x / m_col, (float)(y + 1) / m_row}}
                       << Vertex{{_p2.x(), _p2.y(), _p2.z()}, {(float)(x + 1) / m_col, (float)(y + 1) / m_row}}
                       << Vertex{{_p3.x(), _p3.y(), _p3.z()}, {(float)(x + 1) / m_col, (float)y / m_row}};
        }
    }
}
void Points::init() {
    if (inited) return;
    inited = true;
    initSphere();
    Q_ASSERT(f != nullptr);

    const char* vertexShaderSource =  // 顶点着色器
        u8R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aOffset;
uniform float radius = 1;
out vec2 TexCoords;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main(){
TexCoords = aTexCoords;
gl_Position = projection * view * model * vec4(aPos*radius + aOffset, 1.0);}
)";
    const char* fragmentShaderSource =  // 片段着色器
        u8R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D texture1;
void main(){
FragColor = texture(texture1, TexCoords);}
)";

    auto _program = new QOpenGLShaderProgram();
    _program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    _program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    setShaderProgram(_program);

    if (!m_vao.isCreated())
        m_vao.create();
    if (!m_vbo.isCreated())
        m_vbo.create();
    if (!m_instance_vbo.isCreated())
        m_instance_vbo.create();
    if (!m_program->isLinked())
        m_program->link();

    if (m_vertexCount < m_vertices.count()) {
        if (m_vertexBuffer)
            delete[] m_vertexBuffer;
        m_vertexBuffer = new float[m_vertices.count() * VertexFloatCount];
        m_vertexCount = m_vertices.count();
        int _offset = 0;
        for (auto& vertex : m_vertices) {
            m_vertexBuffer[_offset] = vertex.pos.x();
            _offset++;
            m_vertexBuffer[_offset] = vertex.pos.y();
            _offset++;
            m_vertexBuffer[_offset] = vertex.pos.z();
            _offset++;
            m_vertexBuffer[_offset] = vertex.texture.x();
            _offset++;
            m_vertexBuffer[_offset] = vertex.texture.y();
            _offset++;
        }
    }
    m_vao.bind();
    m_vbo.bind();
    m_vbo.allocate(m_vertexBuffer, sizeof(float) * m_vertexCount * VertexFloatCount);
    m_program->bind();
    // 绑定顶点坐标信息, 从0 * sizeof(float)字节开始读取3个float, 因为一个顶点有8个float数据, 所以下一个数据需要偏移8 * sizeof(float)个字节
    m_program->setAttributeBuffer("aPos", GL_FLOAT, 0 * sizeof(float), 3, VertexFloatCount * sizeof(float));
    m_program->enableAttributeArray("aPos");
    // 绑定纹理坐标信息, 从3 * sizeof(float)字节开始读取2个float, 因为一个顶点有8个float数据, 所以下一个数据需要偏移8 * sizeof(float)个字节
    m_program->setAttributeBuffer("aTexCoords", GL_FLOAT, 3 * sizeof(float), 2, VertexFloatCount * sizeof(float));
    m_program->enableAttributeArray("aTexCoords");

    m_instance_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_instance_vbo.bind();
    m_instance_vbo.allocate(m_points.data(), 3 * num_instances * sizeof(float));  // 8个实例，每个实例有3个float提供给x,y,z坐标作为偏移量
    m_program->setAttributeBuffer("aOffset", GL_FLOAT, 0 * sizeof(float), 3, 3 * sizeof(float));
    m_program->enableAttributeArray("aOffset");
    f->glVertexAttribDivisor(2, 1);  // 指定 index=2 的属性为实例化数组，1 表示每绘制一个实例，更新一次数组中的元素
}

void Points::update() {
    if (!inited) this->init();
    m_vao.bind();

    m_program->bind();

    m_instance_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_instance_vbo.bind();
    m_instance_vbo.allocate(m_points.data(), 3 * num_instances * sizeof(float));  // 8个实例，每个实例有3个float提供给x,y,z坐标作为偏移量
    m_program->setAttributeBuffer("aOffset", GL_FLOAT, 0 * sizeof(float), 3, 3 * sizeof(float));
    m_program->enableAttributeArray("aOffset");
    m_instance_vbo.release();
    f->glVertexAttribDivisor(2, 1);  // 指定 index=2 的属性为实例化数组，1 表示每绘制一个实例，更新一次数组中的元素

    m_program->release();
    m_vao.release();
}

void Points::paint() {
    Q_ASSERT(m_camera != nullptr);
    if (!inited) this->init();
    Q_ASSERT(f != nullptr);

    for (auto i = m_textures.constBegin(); i != m_textures.constEnd(); ++i) {
        i.value()->bind(i.key());
    }
    m_vao.bind();
    m_program->bind();
    // 绑定变换矩阵
    m_program->setUniformValue("projection", m_projection);
    m_program->setUniformValue("view", m_camera->view());
    m_program->setUniformValue("model", model());
    m_program->setUniformValue("radius", m_radius);
    // 绘制
    f->glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4 * m_col * (m_row - 1), num_instances);
    m_program->release();
    m_vao.release();
    for (auto i = m_textures.constBegin(); i != m_textures.constEnd(); ++i) {
        i.value()->release();
    }
}