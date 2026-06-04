#pragma once

#include <QMatrix4x4>
#include <QObject>
#include <QVector3D>
#include <QVector4D>
class Camera : public QObject {
    Q_OBJECT

public:
    Camera(QObject *parent = nullptr);
    ~Camera();

public:
    void look(float yaw, float pitch, float roll);
    void move(float x, float y, float z);

public:
    QVector3D pos() { return m_pos; }
    float yaw() { return m_yaw; }
    float pitch() { return m_pitch; }
    float roll() { return m_roll; }

public:
    void resetLook();
    void resetRotate();
    void resetMove();
    void resetAll();
    void resetInit() {
		m_pos = m_init_pos;
		m_front = m_init_front;
		m_up = m_init_up;
    }

public:
    void update();
    QMatrix4x4 view() { return m_view; }

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;

protected:
    QMatrix4x4 m_view;
    QVector3D m_pos{0, 0, 0} ,m_init_pos=m_pos;
    QVector3D m_front{0, 0, -1} ,m_init_front=m_front;
    QVector3D m_up{0, 1, 0} ,m_init_up=m_up;
    float m_yaw = 0;
    float m_pitch = 0;
    float m_roll = 0;
    QVector3D m_move;
    float m_lookSpeed = 0.1;
    float m_moveSpeed = 0.1;
	bool m_first = true;
};
