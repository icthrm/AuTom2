#include "graphicsView.h"

#include <QGraphicsPixmapItem>

GraphicsView::GraphicsView(QWidget *parent)
    : QGraphicsView(parent), m_isTranslate(false), m_scene(new QGraphicsScene()), m_imageItem(new QGraphicsPixmapItem()) {
    m_scene->addItem(m_imageItem);
    setScene(m_scene);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHint(QPainter::Antialiasing);

    setSceneRect(INT_MIN / 2, INT_MIN / 2, INT_MAX, INT_MAX);
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);

    centerOn(center_x, center_y);
    setFocusPolicy(Qt::NoFocus);
}

GraphicsView::~GraphicsView() {
    m_scene->deleteLater();
    delete m_imageItem;
}

void GraphicsView::SetImage(const QImage &image, bool use_last_center) {
    m_imageItem->setPixmap(QPixmap::fromImage(image));
    if (!use_last_center)
        center_x = image.width() / 2, center_y = image.height() / 2;
    // 设置scene中心到图像中点
    centerOn(center_x, center_y);
    emit centerXChanged(center_x);
    emit centerYChanged(center_y);

    show();
}

void GraphicsView::wheelEvent(QWheelEvent *event) {
    // 滚轮的滚动量
    QPoint scrollAmount = event->angleDelta();
    // 正值表示滚轮远离使用者放大负值表示朝向使用者缩小
    scrollAmount.y() > 0 ? ZoomIn() : ZoomOut();
}

void GraphicsView::mouseMoveEvent(QMouseEvent *event) {
    if (m_isTranslate && m_useMouseDrag) {
        // 获取
        QPointF mouseDelta = event->pos() - m_lastMousePos;
        sum_delta_x -= mouseDelta.x(), sum_delta_y -= mouseDelta.y();
        // 如果移动的比较慢，每次更新的值都很小，基本不超过1，而如果再除以globalScaleFactor就变成0了
        if (qAbs(sum_delta_x) > globalScaleFactor) {
            center_x += sum_delta_x / globalScaleFactor + 0.5;
            sum_delta_x = 0;
            centerOn(center_x, center_y);
        }
        if (qAbs(sum_delta_y) > globalScaleFactor) {
            center_y += sum_delta_y / globalScaleFactor + 0.5;
            sum_delta_y = 0;
            centerOn(center_x, center_y);
        }
        m_lastMousePos = event->pos();
    }
    QGraphicsView::mouseMoveEvent(event);
}

void GraphicsView::mousePressEvent(QMouseEvent *event) {
    auto button = event->button();
    if (m_useMouseDrag && button == Qt::LeftButton) {
        m_isTranslate = true;
        m_lastMousePos = event->pos();
    } else if (m_useMouseRightClick && button == Qt::RightButton) {  // 右键点击时，发送当前鼠标位置的坐标
        QPointF point = mapToScene(event->pos());
        // 只有点击图片时才发送
        if (scene()->itemAt(point, transform()) != NULL) {
            emit rightClickedX(point.x());
            emit rightClickedY(point.y());
            emit rightClickedXY(point.x(), point.y());
        }
    } else if (m_useMouseMiddleClick && button == Qt::MiddleButton) {
        QPointF point = mapToScene(event->pos());
        if (scene()->itemAt(point, transform()) != NULL) {
            emit middleClicked();
        }
    }
    QGraphicsView::mousePressEvent(event);
}


void GraphicsView::mouseReleaseEvent(QMouseEvent *event) {
    if (m_useMouseDrag) {
        if (event->button() == Qt::LeftButton)
            m_isTranslate = false;
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void GraphicsView::mouseDoubleClickEvent(QMouseEvent *event) {
    if (m_useMouseDoubleClick) {
        center_x = m_imageItem->pixmap().width() / 2, center_y = m_imageItem->pixmap().height() / 2;
        centerOn(center_x, center_y);
        emit centerXChanged(center_x);
        emit centerYChanged(center_y);
    }
    QGraphicsView::mouseDoubleClickEvent(event);
}

void GraphicsView::ZoomIn() {
    Zoom(1.1 * globalScaleFactor);
}

void GraphicsView::ZoomOut() {
    Zoom(0.9 * globalScaleFactor);
}

void GraphicsView::Zoom(double scaleFactor) {
    double tmp = scaleFactor / globalScaleFactor;
    // 防止过小或过大
    qreal factor = transform().scale(tmp, tmp).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.07 || factor > 100)
        return;
    scale(tmp, tmp);
    globalScaleFactor = scaleFactor;
    if (qAbs(tmp - 1.0) > 1e-6)  // 如果tmp不是1.0，说明进行了缩放
        emit zoomChanged(scaleFactor);
}
