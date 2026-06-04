#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QColorDialog>
#include <QMainWindow>
#include <QPair>
#include <QPointF>

#include "lib/Object.h"
#include "lib/graphicsView.h"
#include "lib/load.hpp"
#include "lib/showModel.h"
#include "ui_mainwindow.h"
#define empty_object_id -1

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void setCenterX(int x) {
        if (x != center_x) {
            center_x = x;
            emit centerXChanged(x);
        }
    }
    void setCenterY(int y) {
        if (y != center_y) {
            center_y = y;
            emit centerYChanged(y);
        }
    }
    void setCenterZ(int z) {
        if (z != center_z) {
            center_z = z;
            emit centerZChanged(z);
        }
    }
    void setMaxCenterX(int max_x) {
        ui->horizontalSlider_x->setMaximum(max_x);
        ui->spinBox_x->setMaximum(max_x);
        ui->spinBox_point_x->setMaximum(max_x);
        ui->horizontalSlider_x_3d->setMaximum(max_x);
        ui->spinBox_x_3d->setMaximum(max_x);
    }
    void setMaxCenterY(int max_y) {
        ui->spinBox_y->setMaximum(max_y);
        ui->horizontalSlider_y->setMaximum(max_y);
        ui->spinBox_point_y->setMaximum(max_y);
        ui->horizontalSlider_y_3d->setMaximum(max_y);
        ui->spinBox_y_3d->setMaximum(max_y);
    }
    void setMaxCenterZ(int max_z) {
        max_center_z = max_z;
        ui->spinBox_z->setMaximum(max_z);
        ui->horizontalSlider_z->setMaximum(max_z);
        ui->spinBox_point_z->setMaximum(max_z);
        ui->horizontalSlider_z_3d->setMaximum(max_z);
        ui->spinBox_z_3d->setMaximum(max_z);
    }
    void setRotateXDeg(int x) {
        if (x != rotate_x_deg) {
            rotate_x_deg = x;
            emit rotateXDegChanged(x);
        }
    }
    void setRotateYDeg(int y) {
        if (y != rotate_y_deg) {
            rotate_y_deg = y;
            emit rotateYDegChanged(y);
        }
    }
    void setRotateZDeg(int z) {
        if (z != rotate_z_deg) {
            rotate_z_deg = z;
            emit rotateZDegChanged(z);
        }
    }

    void rightClickReceiver(int x, int y);
    void setEditArrow(bool edit) {
        edit_arrow = edit;
        if (edit) {
            edit_rectangle = false;
            edit_polyline = false;
            arrowPointsBuffer.clear();
        }
    }
    void setEditRectangle(bool edit) {
        edit_rectangle = edit;
        if (edit) {
            edit_arrow = false;
            edit_polyline = false;
            rectanglePointsBuffer.clear();
        }
    }
    void setEditPolyline(bool edit) {
        edit_polyline = edit;
        if (edit) {
            edit_arrow = false;
            edit_rectangle = false;
        }
    }

    void open_mrc_file(QString mrcFilePath);


private slots:
    void on_open_mrc_file_triggered();

    void on_spinBox_z_valueChanged(int arg1);

    void on_horizontalSlider_light_valueChanged(int value);

    void on_checkBox_editControl_stateChanged(int arg1);
    void on_horizontalSlider_contrast_valueChanged(int value);

    void on_pushButton_xy_only_clicked();

    void on_pushButton_xy_xz_yz_clicked();

    void on_pushButton_arbitrarily_clicked();

    void on_spinBox_x_valueChanged(int arg1);
    void on_spinBox_y_valueChanged(int arg1);

    void on_spinBox_rotate_x_deg_valueChanged(int arg1);
    void on_spinBox_rotate_y_deg_valueChanged(int arg1);
    void on_spinBox_rotate_z_deg_valueChanged(int arg1);

    void on_pushButton_2d_clicked();
    void on_pushButton_3d_clicked();

    void on_save_model_file_triggered();
    void on_open_model_file_triggered();

    void on_set_python_path_triggered();

private:
    Ui::MainWindow *ui;
    Images *binaryImages = nullptr;
    int center_x = 0, center_y = 0, center_z = 0;
    int light = 0 /*brightness*/, contrast = 100 /*contrast*/;
    QImage xy_only_image, xz_only_image, yz_only_image, arbitrarily_image;
    bool on_xy_only_graphicsView = true, on_xy_xz_yz = false, on_arbitrarily = false;
    int rotate_x_deg = 0, rotate_y_deg = 0, rotate_z_deg = 0;

    bool draw_arrow = false, draw_rectangle = false, draw_polyline = false;
    bool edit_arrow = false, edit_rectangle = false, edit_polyline = false;
    QVector<QPair<int, int>> arrowPoints, arrowPointsBuffer, rectanglePoints, rectanglePointsBuffer;
    QVector<QPointF> polylinePoints;
    int now_object_id = empty_object_id;
    QVector<Object *> objects;  // Must use pointers because Object class has non-copyable members (copy constructor and assignment operator are deleted)
    int timer_id_1 = 0;
    bool edit_mode = false;
    bool point_exist = false, object_exist = false;
    bool updated_objects = false;
    bool do_not_run = false;
    bool on_3d_graphicsView = false;
    bool show_slice = false;

    bool middle_button_clicked = false;
    int timer_id_2 = 0;
    int max_center_z = 0;
    int z_play_direction = 1;  // 1表示z值增加，-1表示z值减少

protected:
    virtual void timerEvent(QTimerEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;

private:
    void drawXYImage();
    void drawXZImage();
    void drawYZImage();
    void drawArbitrarilyImage(const bool &use_last_center = true);
    QImage preProcessImage(const QImage &image);
    void disable1();
    int cnt1 = 1;  // Initial value is 1 because it's already connected once in mainwindow constructor
    void enable1();
    void disable2();
    int cnt2 = 0;
    void enable2();
    void updateNowObjectDisplay_2d() {
        if (now_object_id != empty_object_id) {
            do_not_run = true;
            ui->spinBox_object_id->setValue(now_object_id);
            auto &obj = objects[now_object_id];
            ui->doubleSpinBox_lineWidth->setValue(obj->m_lineWidth);
            ui->doubleSpinBox_radius->setValue(obj->m_radius);
            ui->checkBox_closed->setChecked(obj->m_closed);
            ui->checkBox_drawMesh->setChecked(obj->m_drawMesh);
            ui->checkBox_drawPoints->setChecked(obj->m_drawPoints);
            ui->checkBox_drawLines->setChecked(obj->m_drawLines);
            ui->pushButton_mesh_color->setStyleSheet(
                QString("background-color: rgb(%1,%2,%3);")
                    .arg(obj->m_r)
                    .arg(obj->m_g)
                    .arg(obj->m_b));
            ui->pushButton_line_color->setStyleSheet(
                QString("background-color: rgb(%1,%2,%3);")
                    .arg(obj->m_line_r)
                    .arg(obj->m_line_g)
                    .arg(obj->m_line_b));
            ui->pushButton_point_color->setStyleSheet(
                QString("background-color: rgb(%1,%2,%3);")
                    .arg(obj->m_point_r)
                    .arg(obj->m_point_g)
                    .arg(obj->m_point_b));
            do_not_run = false;
        }
    }
    void display_xy_image_3dview(const int &z);
    void display_xz_image_3dview(const int &y);
    void display_yz_image_3dview(const int &x);

signals:
    void centerXChanged(int x);
    void centerYChanged(int y);
    void centerZChanged(int z);
    void rotateXDegChanged(int x);
    void rotateYDegChanged(int y);
    void rotateZDegChanged(int z);
};

#endif  // MAINWINDOW_H
