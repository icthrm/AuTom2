#include "Rectangle.h"
/*
X坐标轴从左至右，Y坐标轴从下至上，Z坐标轴从里至外。
OpenGL屏幕中心的坐标值是X和Y轴上的0.0点。
中心左面的坐标值是负值，右面是正值。移向屏幕顶端是正值，移向屏幕底端是负值。
移入屏幕深处是负值，移出屏幕则是正值。
*/
Rectangle::Rectangle(const QImage& texture) {
    auto _texture = new QOpenGLTexture(QImage(texture.mirrored()));
    _texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    _texture->setMagnificationFilter(QOpenGLTexture::Linear);
    setTexture(_texture);
}

void Rectangle::init() {
    Q_ASSERT(f != nullptr);
    const char* vertexShaderSource =  // 顶点着色器
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec2 aTexCoords;\n"
        "out vec2 TexCoords;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "void main(){\n"
        "TexCoords = aTexCoords;\n"
        "gl_Position = projection * view * model * vec4(aPos, 1.0);}\n";
    const char* fragmentShaderSource =  // 片段着色器
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec2 TexCoords;\n"
        "uniform sampler2D texture1;\n"
        "void main(){\n"
        "FragColor = texture(texture1, TexCoords);}\n";
    setVertices({
        //  顶点    纹理   法线
        {{-1, 1, 0}, {0, 1}},   // 左上
        {{-1, -1, 0}, {0, 0}},  // 左下
        {{1, -1, 0}, {1, 0}},   // 右下
        {{1, 1, 0}, {1, 1}}     // 右上
    });

    auto _program = new QOpenGLShaderProgram();
    _program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    _program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    setShaderProgram(_program);
    this->update();
}

void Rectangle::setTexture(const QImage& texture) {
    auto _texture = new QOpenGLTexture(QImage(texture.mirrored()));
    _texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    _texture->setMagnificationFilter(QOpenGLTexture::Linear);
    setTexture(_texture);
}

void Rectangle::setTexture(QOpenGLTexture* texture, int index) {
    if (!m_textures.isEmpty()) {
        for (auto it = m_textures.constBegin(); it != m_textures.constEnd(); ++it) {
            delete it.value();
        }
        m_textures.clear();
    }
    m_textures.insert(0, texture);
}
void Rectangle::update() {

    if (!m_vao.isCreated())
        m_vao.create();
    if (!m_vbo.isCreated())
        m_vbo.create();
    if (!m_program->isLinked())
        m_program->link();

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

    m_vao.bind();
    m_vbo.bind();
    m_vbo.allocate(m_vertexBuffer, sizeof(float) * m_vertices.count() * VertexFloatCount);

    m_program->bind();
    // 绑定顶点坐标信息, 从0 * sizeof(float)字节开始读取3个float, 因为一个顶点有8个float数据, 所以下一个数据需要偏移8 * sizeof(float)个字节
    m_program->setAttributeBuffer("aPos", GL_FLOAT, 0 * sizeof(float), 3, VertexFloatCount * sizeof(float));
    m_program->enableAttributeArray("aPos");
    // 绑定纹理坐标信息, 从3 * sizeof(float)字节开始读取2个float, 因为一个顶点有8个float数据, 所以下一个数据需要偏移8 * sizeof(float)个字节
    m_program->setAttributeBuffer("aTexCoords", GL_FLOAT, 3 * sizeof(float), 2, VertexFloatCount * sizeof(float));
    m_program->enableAttributeArray("aTexCoords");

    m_program->release();

    m_vbo.release();
    m_vao.release();
}

void Rectangle::paint() {
    for (auto b = m_textures.keyBegin(); b != m_textures.keyEnd(); ++b) {  // 避免创建list的消耗
        m_textures[*b]->bind(*b);
    }
    m_vao.bind();
    m_program->bind();
    // 绑定变换矩阵
    m_program->setUniformValue("projection", m_projection);
    m_program->setUniformValue("view", m_camera->view());
    m_program->setUniformValue("model", model());
    // 绘制
    f->glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    m_program->release();
    m_vao.release();
    for (auto texture : qAsConst(m_textures)) {
        texture->release();
    }
}