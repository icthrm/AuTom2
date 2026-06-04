#include "Line.h"
/*
X坐标轴从左至右，Y坐标轴从下至上，Z坐标轴从里至外。
OpenGL屏幕中心的坐标值是X和Y轴上的0.0点。
中心左面的坐标值是负值，右面是正值。移向屏幕顶端是正值，移向屏幕底端是负值。
移入屏幕深处是负值，移出屏幕则是正值。
*/
Line::Line(const float& lineWidth /*线的宽度*/, const bool& closed, const int& r, const int& g, const int& b) {
    m_lineWidth = lineWidth;
    m_closed = closed;
    setColor(r, g, b);
}

void Line::init() {
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

    // 几何着色器
    const char* geometryShaderSource =  // 几何着色器
        u8R"(
#version 330 core

uniform float Thickness;
uniform vec2 Viewport;
uniform float MiterLimit;

layout(lines_adjacency) in;
layout(triangle_strip, max_vertices = 7) out;


vec2 toScreenSpace(vec4 vertex)
{
    return vec2( vertex.xy / vertex.w ) * Viewport;
}

float toZValue(vec4 vertex)
{
    return (vertex.z/vertex.w);
}

void drawSegment(vec2 points[4], float zValues[4])
{
    vec2 p0 = points[0];
    vec2 p1 = points[1];
    vec2 p2 = points[2];
    vec2 p3 = points[3];

    /* perform naive culling */
    vec2 area = Viewport * 4;
    if( p1.x < -area.x || p1.x > area.x ) return;
    if( p1.y < -area.y || p1.y > area.y ) return;
    if( p2.x < -area.x || p2.x > area.x ) return;
    if( p2.y < -area.y || p2.y > area.y ) return;

    /* determine the direction of each of the 3 segments (previous, current, next) */
    vec2 v0 = normalize( p1 - p0 );
    vec2 v1 = normalize( p2 - p1 );
    vec2 v2 = normalize( p3 - p2 );

    /* determine the normal of each of the 3 segments (previous, current, next) */
    vec2 n0 = vec2( -v0.y, v0.x );
    vec2 n1 = vec2( -v1.y, v1.x );
    vec2 n2 = vec2( -v2.y, v2.x );

    /* determine miter lines by averaging the normals of the 2 segments */
    vec2 miter_a = normalize( n0 + n1 );	// miter at start of current segment
    vec2 miter_b = normalize( n1 + n2 ); // miter at end of current segment

    /* determine the length of the miter by projecting it onto normal and then inverse it */
    float an1 = dot(miter_a, n1);
    float bn1 = dot(miter_b, n2);
    if (an1==0) an1 = 1;
    if (bn1==0) bn1 = 1;
    float length_a = Thickness / an1;
    float length_b = Thickness / bn1;

    /* prevent excessively long miters at sharp corners */
    if( dot( v0, v1 ) < -MiterLimit ) {
        miter_a = n1;
        length_a = Thickness;

        /* close the gap */
        if( dot( v0, n1 ) > 0 ) {
            gl_Position = vec4( ( p1 + Thickness * n0 ) / Viewport, zValues[1], 1.0 );
            EmitVertex();

            gl_Position = vec4( ( p1 + Thickness * n1 ) / Viewport, zValues[1], 1.0 );
            EmitVertex();

            gl_Position = vec4( p1 / Viewport, zValues[1], 1.0 );
            EmitVertex();

            EndPrimitive();
        }
        else {
            gl_Position = vec4( ( p1 - Thickness * n1 ) / Viewport, zValues[1], 1.0 );
            EmitVertex();

            gl_Position = vec4( ( p1 - Thickness * n0 ) / Viewport, zValues[1], 1.0 );
            EmitVertex();

            gl_Position = vec4( p1 / Viewport, zValues[1], 1.0 );
            EmitVertex();

            EndPrimitive();
        }
    }
    if( dot( v1, v2 ) < -MiterLimit ) {
        miter_b = n1;
        length_b = Thickness;
    }
    // generate the triangle strip
    gl_Position = vec4( ( p1 + length_a * miter_a ) / Viewport, zValues[1], 1.0 );
    EmitVertex();

    gl_Position = vec4( ( p1 - length_a * miter_a ) / Viewport, zValues[1], 1.0 );
    EmitVertex();

    gl_Position = vec4( ( p2 + length_b * miter_b ) / Viewport, zValues[2], 1.0 );
    EmitVertex();

    gl_Position = vec4( ( p2 - length_b * miter_b ) / Viewport, zValues[2], 1.0 );
    EmitVertex();

    EndPrimitive();
}

void main(void)
{
    // 4 points
    vec4 Points[4];
    Points[0] = gl_in[0].gl_Position;
    Points[1] = gl_in[1].gl_Position;
    Points[2] = gl_in[2].gl_Position;
    Points[3] = gl_in[3].gl_Position;

    // screen coords
    vec2 points[4];
    points[0] = toScreenSpace(Points[0]);
    points[1] = toScreenSpace(Points[1]);
    points[2] = toScreenSpace(Points[2]);
    points[3] = toScreenSpace(Points[3]);

    // deepness values
    float zValues[4];
    zValues[0] = toZValue(Points[0]);
    zValues[1] = toZValue(Points[1]);
    zValues[2] = toZValue(Points[2]);
    zValues[3] = toZValue(Points[3]);

    drawSegment(points, zValues);
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
    _program->addShaderFromSourceCode(QOpenGLShader::Geometry, geometryShaderSource);
    _program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    setShaderProgram(_program);

    if (!m_vao.isCreated())
        m_vao.create();
    if (!m_vbo.isCreated())
        m_vbo.create();
    if (!m_program->isLinked())
        m_program->link();

    m_vao.bind();
    m_vbo.bind();
    m_vbo.allocate(m_points.data(), sizeof(float) * m_points.size() * 3);

    m_program->bind();
    // 绑定顶点坐标信息, 从0 * sizeof(float)字节开始读取3个float, 因为一个顶点有3个float数据, 所以下一个数据需要偏移3 * sizeof(float)个字节
    m_program->setAttributeBuffer("aPos", GL_FLOAT, 0 * sizeof(float), 3, VertexFloatCount * sizeof(float));
    m_program->enableAttributeArray("aPos");
    m_vbo.release();

    m_program->release();
    m_vao.release();
}

void Line::update() {
    if (!inited) this->init();
    Q_ASSERT(f != nullptr);
    m_vao.bind();
    m_vbo.bind();
    m_vbo.allocate(m_points.data(), sizeof(float) * m_points.size() * 3);

    m_program->bind();
    // 绑定顶点坐标信息, 从0 * sizeof(float)字节开始读取3个float, 因为一个顶点有3个float数据, 所以下一个数据需要偏移3 * sizeof(float)个字节
    m_program->setAttributeBuffer("aPos", GL_FLOAT, 0 * sizeof(float), 3, VertexFloatCount * sizeof(float));
    m_program->enableAttributeArray("aPos");
    m_vbo.release();

    m_program->release();
    m_vao.release();
}

void Line::paint() {
    Q_ASSERT(m_camera != nullptr);
    if (!inited) this->init();
    Q_ASSERT(f != nullptr);

    m_vao.bind();
    m_program->bind();
    // 绑定变换矩阵
    m_program->setUniformValue("projection", m_projection);
    m_program->setUniformValue("view", m_camera->view());
    m_program->setUniformValue("model", model());
    m_program->setUniformValue("color", m_r, m_g, m_b, 1.0f);
    m_program->setUniformValue("Thickness", m_lineWidth);

    m_program->setUniformValue("MiterLimit", 0.1f);

    f->glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, m_vertexCount);
    m_program->release();
    m_vao.release();
}