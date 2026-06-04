#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <qevent.h>

#include <QBoxLayout>
#include <QGraphicsView>

class GraphicsView : public QGraphicsView {
    Q_OBJECT

public:
    GraphicsView(QWidget *parent = 0);
    ~GraphicsView();

    void setUseMouseDrag(bool useMouseDrag) { m_useMouseDrag = useMouseDrag; }
    void setUseMouseRightClick(bool useMouseRightClick) { m_useMouseRightClick = useMouseRightClick; }
    void setUseMouseDoubleClick(bool useMouseDoubleClick) { m_useMouseDoubleClick = useMouseDoubleClick; }
    void setUseMouseMiddleClick(bool useMouseMiddleClick) { m_useMouseMiddleClick = useMouseMiddleClick; }

protected:
    virtual void wheelEvent(QWheelEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;

public slots:
    void ZoomIn();
    void ZoomOut();
    void Zoom(double scaleFactor);
    void SetImage(const QImage &image, bool use_last_center = true);
    void setCenterX(int x) {
        if (x != center_x) {
            center_x = x;
            centerOn(center_x, center_y);
            emit centerXChanged(x);
        }
    }
    void setCenterY(int y) {
        if (y != center_y) {
            center_y = y;
            centerOn(center_x, center_y);
            emit centerYChanged(y);
        }
    }

private:
    int center_x = 0, center_y = 0;
    double globalScaleFactor = 1.0;
    double sum_delta_x = 0, sum_delta_y = 0;
    bool m_useMouseDrag=true, m_useMouseRightClick=true, m_useMouseDoubleClick=true;
    bool m_useMouseMiddleClick=true;

    bool m_isTranslate;
    QPoint m_lastMousePos;

    QGraphicsScene *m_scene;
    QGraphicsPixmapItem *m_imageItem;

signals:
    void centerXChanged(int x);
    void centerYChanged(int y);
    void zoomChanged(double zoom);
    void rightClickedX(int x);
    void rightClickedY(int y);
    void rightClickedXY(int x, int y);
    void middleClicked();
};

#endif  // GRAPHICSVIEW_H
