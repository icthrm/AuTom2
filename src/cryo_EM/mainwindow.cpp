#include "mainwindow.h"

#include <QDialog>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QMovie>
#include <QProcess>
#include <QSettings>
#include <QTextStream>

#include "lib/drawSymbol.h"
#include "lib/light_contrast.h"
#include "ui_mainwindow.h"
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
                                          ui(new Ui::MainWindow) {
    ui->setupUi(this);
    this->binaryImages = new Images();

    QObject::connect(ui->xy_only_graphicsView,  // 让鼠标右键点击的时候，设置中心点
                     SIGNAL(rightClickedX(int)),
                     this,
                     SLOT(setCenterX(int)));
    QObject::connect(ui->xy_only_graphicsView,
                     SIGNAL(rightClickedY(int)),
                     this,
                     SLOT(setCenterY(int)));

    QObject::connect(ui->xy_only_graphicsView,
                     SIGNAL(centerXChanged(int)),
                     this,
                     SLOT(setCenterX(int)));
    QObject::connect(ui->xy_only_graphicsView,
                     SIGNAL(centerYChanged(int)),
                     this,
                     SLOT(setCenterY(int)));

    QObject::connect(ui->xy_graphicsView,
                     SIGNAL(centerXChanged(int)),
                     this,
                     SLOT(setCenterX(int)));
    QObject::connect(ui->xy_graphicsView,
                     SIGNAL(centerYChanged(int)),
                     this,
                     SLOT(setCenterY(int)));

    QObject::connect(ui->xz_graphicsView,
                     SIGNAL(centerXChanged(int)),
                     this,
                     SLOT(setCenterX(int)));
    QObject::connect(ui->xz_graphicsView,
                     SIGNAL(centerYChanged(int)),
                     this,
                     SLOT(setCenterZ(int)));

    QObject::connect(ui->yz_graphicsView,
                     SIGNAL(centerYChanged(int)),
                     this,
                     SLOT(setCenterY(int)));
    QObject::connect(ui->yz_graphicsView,
                     SIGNAL(centerXChanged(int)),
                     this,
                     SLOT(setCenterZ(int)));

    QObject::connect(ui->spinBox_x,
                     SIGNAL(valueChanged(int)),
                     this,
                     SLOT(setCenterX(int)));
    QObject::connect(ui->spinBox_y,
                     SIGNAL(valueChanged(int)),
                     this,
                     SLOT(setCenterY(int)));
    QObject::connect(ui->spinBox_z,
                     SIGNAL(valueChanged(int)),
                     this,
                     SLOT(setCenterZ(int)));

    QObject::connect(ui->horizontalSlider_x,
                     SIGNAL(valueChanged(int)),
                     this,
                     SLOT(setCenterX(int)));
    QObject::connect(ui->horizontalSlider_y,
                     SIGNAL(valueChanged(int)),
                     this,
                     SLOT(setCenterY(int)));
    QObject::connect(ui->horizontalSlider_z,
                     SIGNAL(valueChanged(int)),
                     this,
                     SLOT(setCenterZ(int)));

    QObject::connect(this,
                     SIGNAL(centerXChanged(int)),
                     ui->xy_only_graphicsView,
                     SLOT(setCenterX(int)));
    QObject::connect(this,
                     SIGNAL(centerYChanged(int)),
                     ui->xy_only_graphicsView,
                     SLOT(setCenterY(int)));

    QObject::connect(this,
                     SIGNAL(centerXChanged(int)),
                     ui->xy_graphicsView,
                     SLOT(setCenterX(int)));
    QObject::connect(this,
                     SIGNAL(centerYChanged(int)),
                     ui->xy_graphicsView,
                     SLOT(setCenterY(int)));

    QObject::connect(this,
                     SIGNAL(centerXChanged(int)),
                     ui->xz_graphicsView,
                     SLOT(setCenterX(int)));
    QObject::connect(this,
                     SIGNAL(centerZChanged(int)),
                     ui->xz_graphicsView,
                     SLOT(setCenterY(int)));

    QObject::connect(this,
                     SIGNAL(centerYChanged(int)),
                     ui->yz_graphicsView,
                     SLOT(setCenterY(int)));
    QObject::connect(this,
                     SIGNAL(centerZChanged(int)),
                     ui->yz_graphicsView,
                     SLOT(setCenterX(int)));

    QObject::connect(this,
                     SIGNAL(centerXChanged(int)),
                     ui->spinBox_x,
                     SLOT(setValue(int)));
    QObject::connect(this,
                     SIGNAL(centerYChanged(int)),
                     ui->spinBox_y,
                     SLOT(setValue(int)));
    QObject::connect(this,
                     SIGNAL(centerZChanged(int)),
                     ui->spinBox_z,
                     SLOT(setValue(int)));

    QObject::connect(this,
                     SIGNAL(centerXChanged(int)),
                     ui->horizontalSlider_x,
                     SLOT(setValue(int)));
    QObject::connect(this,
                     SIGNAL(centerYChanged(int)),
                     ui->horizontalSlider_y,
                     SLOT(setValue(int)));
    QObject::connect(this,
                     SIGNAL(centerZChanged(int)),
                     ui->horizontalSlider_z,
                     SLOT(setValue(int)));

    QObject::connect(ui->doubleSpinBox_zoom,
                     SIGNAL(valueChanged(double)),
                     ui->xy_only_graphicsView,
                     SLOT(Zoom(double)));
    QObject::connect(ui->xy_only_graphicsView,
                     SIGNAL(zoomChanged(double)),
                     ui->doubleSpinBox_zoom,
                     SLOT(setValue(double)));

    QObject::connect(ui->doubleSpinBox_zoom,
                     SIGNAL(valueChanged(double)),
                     ui->xy_graphicsView,
                     SLOT(Zoom(double)));
    QObject::connect(ui->xy_graphicsView,
                     SIGNAL(zoomChanged(double)),
                     ui->doubleSpinBox_zoom,
                     SLOT(setValue(double)));

    QObject::connect(ui->doubleSpinBox_zoom,
                     SIGNAL(valueChanged(double)),
                     ui->xz_graphicsView,
                     SLOT(Zoom(double)));
    QObject::connect(ui->xz_graphicsView,
                     SIGNAL(zoomChanged(double)),
                     ui->doubleSpinBox_zoom,
                     SLOT(setValue(double)));

    QObject::connect(ui->doubleSpinBox_zoom,
                     SIGNAL(valueChanged(double)),
                     ui->yz_graphicsView,
                     SLOT(Zoom(double)));
    QObject::connect(ui->yz_graphicsView,
                     SIGNAL(zoomChanged(double)),
                     ui->doubleSpinBox_zoom,
                     SLOT(setValue(double)));

    // 关闭三视图的鼠标事件
    ui->xy_graphicsView->setUseMouseDrag(false);
    ui->xy_graphicsView->setUseMouseRightClick(false);
    ui->xy_graphicsView->setUseMouseDoubleClick(false);
    ui->xy_graphicsView->setUseMouseMiddleClick(false);
    ui->xz_graphicsView->setUseMouseDrag(false);
    ui->xz_graphicsView->setUseMouseRightClick(false);
    ui->xz_graphicsView->setUseMouseDoubleClick(false);
    ui->xz_graphicsView->setUseMouseMiddleClick(false);
    ui->yz_graphicsView->setUseMouseDrag(false);
    ui->yz_graphicsView->setUseMouseRightClick(false);
    ui->yz_graphicsView->setUseMouseDoubleClick(false);
    ui->yz_graphicsView->setUseMouseMiddleClick(false);

    ui->widget_rotate_x_deg->hide();
    ui->widget_rotate_y_deg->hide();
    ui->widget_rotate_z_deg->hide();
    // 要注意，任意方向的那个切面的水平x、y、z滑动条是不会改变图片的位置（因为没有把信号绑定到graphicsView），只是改变切割点的位置
    QObject::connect(ui->spinBox_rotate_x_deg,
                     SIGNAL(valueChanged(int)),
                     this,
                     SLOT(setRotateXDeg(int)));
    QObject::connect(ui->spinBox_rotate_y_deg,
                     SIGNAL(valueChanged(int)),
                     this,
                     SLOT(setRotateYDeg(int)));
    QObject::connect(ui->spinBox_rotate_z_deg,
                     SIGNAL(valueChanged(int)),
                     this,
                     SLOT(setRotateZDeg(int)));
    QObject::connect(ui->horizontalSlider_rotate_x_deg,
                     SIGNAL(valueChanged(int)),
                     this,
                     SLOT(setRotateXDeg(int)));
    QObject::connect(ui->horizontalSlider_rotate_y_deg,
                     SIGNAL(valueChanged(int)),
                     this,
                     SLOT(setRotateYDeg(int)));
    QObject::connect(ui->horizontalSlider_rotate_z_deg,
                     SIGNAL(valueChanged(int)),
                     this,
                     SLOT(setRotateZDeg(int)));
    QObject::connect(this,
                     SIGNAL(rotateXDegChanged(int)),
                     ui->spinBox_rotate_x_deg,
                     SLOT(setValue(int)));
    QObject::connect(this,
                     SIGNAL(rotateYDegChanged(int)),
                     ui->spinBox_rotate_y_deg,
                     SLOT(setValue(int)));
    QObject::connect(this,
                     SIGNAL(rotateZDegChanged(int)),
                     ui->spinBox_rotate_z_deg,
                     SLOT(setValue(int)));
    QObject::connect(this,
                     SIGNAL(rotateXDegChanged(int)),
                     ui->horizontalSlider_rotate_x_deg,
                     SLOT(setValue(int)));
    QObject::connect(this,
                     SIGNAL(rotateYDegChanged(int)),
                     ui->horizontalSlider_rotate_y_deg,
                     SLOT(setValue(int)));
    QObject::connect(this,
                     SIGNAL(rotateZDegChanged(int)),
                     ui->horizontalSlider_rotate_z_deg,
                     SLOT(setValue(int)));
    QObject::connect(ui->doubleSpinBox_zoom,
                     SIGNAL(valueChanged(double)),
                     ui->arbitrarily_graphicsView,
                     SLOT(Zoom(double)));
    QObject::connect(ui->arbitrarily_graphicsView,
                     SIGNAL(zoomChanged(double)),
                     ui->doubleSpinBox_zoom,
                     SLOT(setValue(double)));

    ui->openGLWidget_arbitrarily->hide();
    ui->openGLWidget_arbitrarily->setMinimumSize(200, 200);
    ui->openGLWidget_arbitrarily->setMaxXYZ(100, 100, 100);

    QObject::connect(this,
                     SIGNAL(centerXChanged(int)),
                     ui->openGLWidget_arbitrarily,
                     SLOT(setX(int)));
    QObject::connect(this,
                     SIGNAL(centerZChanged(int)),
                     ui->openGLWidget_arbitrarily,
                     SLOT(setY(int)));
    QObject::connect(this,
                     SIGNAL(centerYChanged(int)),
                     ui->openGLWidget_arbitrarily,
                     SLOT(setZ(int)));
    QObject::connect(this,
                     SIGNAL(rotateXDegChanged(int)),
                     ui->openGLWidget_arbitrarily,
                     SLOT(setRotateX(int)));
    QObject::connect(this,
                     SIGNAL(rotateYDegChanged(int)),
                     ui->openGLWidget_arbitrarily,
                     SLOT(setRotateY(int)));
    QObject::connect(this,
                     SIGNAL(rotateZDegChanged(int)),
                     ui->openGLWidget_arbitrarily,
                     SLOT(setRotateZ(int)));

    // 将三个按钮的checked状态同步到三个radioButton的enabled状态
    ui->radioButton_arrow->setEnabled(false);
    ui->radioButton_rectangle->setEnabled(false);
    ui->radioButton_polyline->setEnabled(false);
    ui->spinBox_object_id->setEnabled(false);
    QObject::connect(ui->pushButton_arrow,  // 新语法，便于调试
                     &QPushButton::toggled,
                     ui->radioButton_arrow,
                     &QRadioButton::setEnabled);
    QObject::connect(ui->pushButton_rectangle,
                     &QPushButton::toggled,
                     ui->radioButton_rectangle,
                     &QRadioButton::setEnabled);
    QObject::connect(ui->pushButton_polyline,
                     &QPushButton::toggled,
                     ui->radioButton_polyline,
                     &QRadioButton::setEnabled);

    QObject::connect(ui->pushButton_arrow,
                     &QPushButton::toggled,
                     ui->radioButton_arrow,
                     &QRadioButton::setChecked);
    QObject::connect(ui->pushButton_rectangle,
                     &QPushButton::toggled,
                     ui->radioButton_rectangle,
                     &QRadioButton::setChecked);
    QObject::connect(ui->pushButton_polyline,
                     &QPushButton::toggled,
                     ui->radioButton_polyline,
                     &QRadioButton::setChecked);

    QObject::connect(ui->pushButton_arrow,
                     &QPushButton::toggled,
                     this,
                     &MainWindow::setEditArrow);
    QObject::connect(ui->pushButton_rectangle,
                     &QPushButton::toggled,
                     this,
                     &MainWindow::setEditRectangle);
    QObject::connect(ui->pushButton_polyline,
                     &QPushButton::toggled,
                     this,
                     &MainWindow::setEditPolyline);

    QObject::connect(ui->radioButton_arrow,
                     &QRadioButton::toggled,
                     this,
                     &MainWindow::setEditArrow);
    QObject::connect(ui->radioButton_rectangle,
                     &QRadioButton::toggled,
                     this,
                     &MainWindow::setEditRectangle);
    QObject::connect(ui->radioButton_polyline,
                     &QRadioButton::toggled,
                     this,
                     &MainWindow::setEditPolyline);

    // 绘制箭头
    QObject::connect(ui->pushButton_arrow,
                     &QPushButton::toggled,
                     this,
                     [&](bool checked) {
                         arrowPointsBuffer.clear();
                         arrowPoints.clear();
                         updated_objects = true;
                         if (checked) {
                             draw_arrow = true;
                             // 记得禁用那些编辑按钮
                             this->disable1();
                             this->enable2();
                             drawXYImage();

                         } else {
                             draw_arrow = false;
                             // 记得启用那些编辑按钮
                             this->enable1();
                             this->disable2();
                             drawXYImage();
                         }
                     });
    // 绘制矩形
    QObject::connect(ui->pushButton_rectangle,
                     &QPushButton::toggled,
                     this,
                     [&](bool checked) {
                         rectanglePointsBuffer.clear();
                         rectanglePoints.clear();
                         updated_objects = true;
                         if (checked) {
                             draw_rectangle = true;
                             // 记得禁用那些编辑按钮
                             this->disable1();
                             this->enable2();
                             drawXYImage();
                         } else {
                             draw_rectangle = false;
                             // 记得启用那些编辑按钮
                             this->enable1();
                             this->disable2();
                             drawXYImage();
                         }
                     });
    // 绘制折线
    QObject::connect(ui->pushButton_polyline,
                     &QPushButton::toggled,
                     this,
                     [&](bool checked) {
                         polylinePoints.clear();
                         updated_objects = true;
                         if (checked) {
                             draw_polyline = true;
                             // 记得禁用那些编辑按钮
                             this->disable1();
                             this->enable2();
                             drawXYImage();

                         } else {
                             draw_polyline = false;
                             // 记得启用那些编辑按钮
                             this->enable1();
                             this->disable2();
                             drawXYImage();
                         }
                     });

    // 用于2d编辑界面的控件
    // 点是否存在
    ui->widget_point->setEnabled(false);  // 应该要每200ms检查当前选中object当前z值是否有点，如果有则enable
    ui->pushButton_delete_point->setEnabled(false);
    // object是否存在
    ui->pushButton_delete_object->setEnabled(false);
    ui->widget_color->setEnabled(false);
    ui->widget_lineWidth->setEnabled(false);
    ui->widget_radius->setEnabled(false);
    ui->widget_object_draw_setting->setEnabled(false);
    ui->pushButton_create_point->setEnabled(false);
    ui->spinBox_object_id->setEnabled(false);
    timer_id_1 = startTimer(std::chrono::milliseconds(200));
    QObject::connect(ui->doubleSpinBox_lineWidth,
                     &QDoubleSpinBox::valueChanged,
                     this,
                     [&](double value) {
                         if (!do_not_run) {
                             objects[now_object_id]->m_lineWidth = value;
                             updated_objects = true;
                         }
                     });
    QObject::connect(ui->doubleSpinBox_radius,
                     &QDoubleSpinBox::valueChanged,
                     this,
                     [&](double value) {
                         if (!do_not_run) {
                             objects[now_object_id]->m_radius = value;
                             updated_objects = true;
                         }
                     });
    QObject::connect(ui->checkBox_closed,
                     &QCheckBox::toggled,
                     this,
                     [&](bool checked) {
                         if (!do_not_run) {
                             objects[now_object_id]->m_closed = checked;
                             updated_objects = true;
                         }
                     });
    QObject::connect(ui->checkBox_drawMesh,
                     &QCheckBox::toggled,
                     this,
                     [&](bool checked) {
                         if (!do_not_run)
                             objects[now_object_id]->m_drawMesh = checked;
                     });
    QObject::connect(ui->checkBox_drawPoints,
                     &QCheckBox::toggled,
                     this,
                     [&](bool checked) {
                         if (!do_not_run) {
                             objects[now_object_id]->m_drawPoints = checked;
                             updated_objects = true;
                         }
                     });
    QObject::connect(ui->checkBox_drawLines,
                     &QCheckBox::toggled,
                     this,
                     [&](bool checked) {
                         if (!do_not_run) {
                             objects[now_object_id]->m_drawLines = checked;
                             updated_objects = true;
                         }
                     });
    // 禁用当前点的z值的编辑
    ui->spinBox_point_z->setReadOnly(true);  // 不能编辑当前点的z值，要么删除它然后调整到新的z值然后再创建点
    ui->spinBox_point_z->setButtonSymbols(QAbstractSpinBox::NoButtons);

    // 创建object
    QObject::connect(ui->pushButton_create_object,
                     &QPushButton::clicked,
                     this,
                     [&]() {
                         objects.emplace_back(new Object());
                         now_object_id = objects.size() - 1;
                         ui->spinBox_object_id->setMaximum(now_object_id);
                         ui->spinBox_object_id_3d->setMaximum(now_object_id);
                         ui->spinBox_object_id->setValue(now_object_id);
                         if (now_object_id == 0) {  // 因为不会触发spinBox_object_id的valueChanged，所以要手动更新
                             updateNowObjectDisplay_2d();
                         }
                         updated_objects = true;
                     });
    // 删除object
    QObject::connect(ui->pushButton_delete_object,
                     &QPushButton::clicked,
                     this,
                     [&]() {
                         if (now_object_id != empty_object_id) {
                             auto it = objects.begin() + now_object_id;
                             delete *it;
                             objects.erase(it);
                             now_object_id = objects.size() - 1;
                             if (now_object_id < 0)
                                 now_object_id = empty_object_id;
                             if (now_object_id != empty_object_id) {
                                 ui->spinBox_object_id->setMaximum(now_object_id);
                                 ui->spinBox_object_id_3d->setMaximum(now_object_id);
                                 ui->spinBox_object_id->setValue(now_object_id);
                             }
                             updated_objects = true;
                         }
                     });
    // 更新object id
    QObject::connect(ui->spinBox_object_id,
                     &QSpinBox::valueChanged,
                     this,
                     [&](int value) {
                         if (!do_not_run) {
                             now_object_id = value;
                             updateNowObjectDisplay_2d();
                         }
                     });
    // 创建点
    QObject::connect(ui->pushButton_create_point,
                     &QPushButton::clicked,
                     this,
                     [&]() {
                         auto &tmp = objects[now_object_id]->z_vertexArray;
                         if (object_exist) {
                             if (!tmp.contains(center_z)) {
                                 tmp[center_z] = QVector<Vertex3>();
                             }
                             tmp[center_z].emplace_back(
                                 center_x,
                                 center_y,
                                 center_z);
                             do_not_run = true;                        // 防止spinBox再次触发valueChanged
                             ui->spinBox_point_x->setValue(center_x);  // 更新编辑框的值
                             ui->spinBox_point_y->setValue(center_y);
                             do_not_run = false;
                             updated_objects = true;
                         }
                     });
    // 删除点
    QObject::connect(ui->pushButton_delete_point,
                     &QPushButton::clicked,
                     this,
                     [&]() {
                         point_exist = object_exist && objects[now_object_id]->z_vertexArray.contains(center_z) && !objects[now_object_id]->z_vertexArray[center_z].empty();
                         if (point_exist) {
                             auto &tmp = objects[now_object_id]->z_vertexArray[center_z];
                             tmp.pop_back();
                             // 更新编辑框的值
                             if (!tmp.isEmpty()) {
                                 auto &last = tmp.back();
                                 ui->spinBox_point_x->setValue(last.x);
                                 ui->spinBox_point_y->setValue(last.y);
                             }
                             updated_objects = true;
                         }
                     });
    // 更新点
    QObject::connect(ui->spinBox_point_x,
                     &QSpinBox::valueChanged,
                     this,
                     [&](int value) {
                         if (!do_not_run && point_exist) {
                             objects[now_object_id]->z_vertexArray[center_z].back().x = value;
                             updated_objects = true;
                         }
                     });
    QObject::connect(ui->spinBox_point_y,
                     &QSpinBox::valueChanged,
                     this,
                     [&](int value) {
                         if (!do_not_run && point_exist) {
                             objects[now_object_id]->z_vertexArray[center_z].back().y = value;
                             updated_objects = true;
                         }
                     });
    // 设置mesh颜色
    QObject::connect(ui->pushButton_mesh_color,
                     &QPushButton::clicked,
                     this,
                     [&]() {
                         QColor color = QColorDialog::getColor();
                         if (color.isValid()) {
                             auto &obj = objects[now_object_id];
                             obj->m_r = color.red();
                             obj->m_g = color.green();
                             obj->m_b = color.blue();
                             ui->pushButton_mesh_color->setStyleSheet(
                                 QString("background-color: rgb(%1,%2,%3);")
                                     .arg(color.red())
                                     .arg(color.green())
                                     .arg(color.blue()));
                         }
                     });
    // 设置line颜色
    QObject::connect(ui->pushButton_line_color,
                     &QPushButton::clicked,
                     this,
                     [&]() {
                         QColor color = QColorDialog::getColor();
                         if (color.isValid()) {
                             auto &obj = objects[now_object_id];
                             obj->m_line_r = color.red();
                             obj->m_line_g = color.green();
                             obj->m_line_b = color.blue();
                             ui->pushButton_line_color->setStyleSheet(
                                 QString("background-color: rgb(%1,%2,%3);")
                                     .arg(color.red())
                                     .arg(color.green())
                                     .arg(color.blue()));
                             updated_objects = true;
                         }
                     });
    // 设置point颜色
    QObject::connect(ui->pushButton_point_color,
                     &QPushButton::clicked,
                     this,
                     [&]() {
                         QColor color = QColorDialog::getColor();
                         if (color.isValid()) {
                             auto &obj = objects[now_object_id];
                             obj->m_point_r = color.red();
                             obj->m_point_g = color.green();
                             obj->m_point_b = color.blue();
                             ui->pushButton_point_color->setStyleSheet(
                                 QString("background-color: rgb(%1,%2,%3);")
                                     .arg(color.red())
                                     .arg(color.green())
                                     .arg(color.blue()));
                             updated_objects = true;
                         }
                     });
    // 鼠标右键点击创建点
    QObject::connect(ui->xy_only_graphicsView,
                     &GraphicsView::rightClickedXY,
                     this,
                     [&](int x, int y) {
                         if (edit_mode) {
                             auto &tmp = objects[now_object_id]->z_vertexArray;
                             if (object_exist) {
                                 if (!tmp.contains(center_z)) {
                                     tmp[center_z] = QVector<Vertex3>();
                                 }
                                 tmp[center_z].emplace_back(
                                     x,
                                     y,
                                     center_z);
                                 do_not_run = true;                 // 防止spinBox再次触发valueChanged
                                 ui->spinBox_point_x->setValue(x);  // 更新编辑框的值
                                 ui->spinBox_point_y->setValue(y);
                                 do_not_run = false;
                                 updated_objects = true;
                             }
                         }
                     });

    // 3d界面
    QObject::connect(ui->spinBox_object_id_3d,
                     &QSpinBox::valueChanged,
                     this,
                     [&](int val) {
                         now_object_id = val;
                         auto &obj = objects[now_object_id];
                         ui->doubleSpinBox_lineWidth_3d->setValue(obj->m_lineWidth);
                         ui->doubleSpinBox_radius_3d->setValue(obj->m_radius);
                         ui->checkBox_closed_3d->setChecked(obj->m_closed);
                         ui->checkBox_draw_mesh_3d->setChecked(obj->m_drawMesh);
                         ui->checkBox_draw_point_3d->setChecked(obj->m_drawPoints);
                         ui->checkBox_draw_line_3d->setChecked(obj->m_drawLines);
                         ui->pushButton_mesh_color_3d->setStyleSheet(
                             QString("background-color: rgb(%1,%2,%3);")
                                 .arg(obj->m_r)
                                 .arg(obj->m_g)
                                 .arg(obj->m_b));
                         ui->pushButton_line_color_3d->setStyleSheet(
                             QString("background-color: rgb(%1,%2,%3);")
                                 .arg(obj->m_line_r)
                                 .arg(obj->m_line_g)
                                 .arg(obj->m_line_b));
                         ui->pushButton_point_color_3d->setStyleSheet(
                             QString("background-color: rgb(%1,%2,%3);")
                                 .arg(obj->m_point_r)
                                 .arg(obj->m_point_g)
                                 .arg(obj->m_point_b));
                     });
    // 设置mesh颜色
    QObject::connect(ui->pushButton_mesh_color_3d,
                     &QPushButton::clicked,
                     this,
                     [&]() {
                         QColor color = QColorDialog::getColor();
                         if (color.isValid()) {
                             auto &obj = objects[now_object_id];
                             obj->setColor(color.red(), color.green(), color.blue());
                             updated_objects = true;
                             ui->pushButton_mesh_color_3d->setStyleSheet(
                                 QString("background-color: rgb(%1,%2,%3);")
                                     .arg(color.red())
                                     .arg(color.green())
                                     .arg(color.blue()));
                         }
                     });
    // 设置line颜色
    QObject::connect(ui->pushButton_line_color_3d,
                     &QPushButton::clicked,
                     this,
                     [&]() {
                         QColor color = QColorDialog::getColor();
                         if (color.isValid()) {
                             auto &obj = objects[now_object_id];
                             obj->setLineColor(color.red(), color.green(), color.blue());
                             updated_objects = true;
                             ui->pushButton_line_color_3d->setStyleSheet(
                                 QString("background-color: rgb(%1,%2,%3);")
                                     .arg(color.red())
                                     .arg(color.green())
                                     .arg(color.blue()));
                         }
                     });
    // 设置point颜色
    QObject::connect(ui->pushButton_point_color_3d,
                     &QPushButton::clicked,
                     this,
                     [&]() {
                         QColor color = QColorDialog::getColor();
                         if (color.isValid()) {
                             auto &obj = objects[now_object_id];
                             obj->setPointColor(color.red(), color.green(), color.blue());
                             updated_objects = true;
                             ui->pushButton_point_color_3d->setStyleSheet(
                                 QString("background-color: rgb(%1,%2,%3);")
                                     .arg(color.red())
                                     .arg(color.green())
                                     .arg(color.blue()));
                         }
                     });
    // 设置line宽度
    QObject::connect(ui->doubleSpinBox_lineWidth_3d,
                     &QDoubleSpinBox::valueChanged,
                     this,
                     [&](double value) {
                         if (!do_not_run) {
                             auto &obj = objects[now_object_id];
                             obj->setLineWidth(value);
                             updated_objects = true;
                         }
                     });
    // 设置point半径
    QObject::connect(ui->doubleSpinBox_radius_3d,
                     &QDoubleSpinBox::valueChanged,
                     this,
                     [&](double value) {
                         if (!do_not_run) {
                             auto &obj = objects[now_object_id];
                             obj->setPointRadius(value);
                             updated_objects = true;
                         }
                     });
    // 设置是否闭合
    QObject::connect(ui->checkBox_closed_3d,
                     &QCheckBox::toggled,
                     this,
                     [&](bool checked) {
                         if (!do_not_run) {
                             auto &obj = objects[now_object_id];
                             ui->openGLWidget_3d->makeCurrent();
                             obj->setLineClosed(checked);
                             obj->setDrawMesh(obj->m_drawMesh);
                             ui->openGLWidget_3d->doneCurrent();
                             updated_objects = true;
                         }
                     });
    // 设置是否绘制mesh
    QObject::connect(ui->checkBox_draw_mesh_3d,
                     &QCheckBox::toggled,
                     this,
                     [&](bool checked) {
                         if (!do_not_run) {
                             auto &obj = objects[now_object_id];
                             ui->openGLWidget_3d->makeCurrent();
                             obj->setDrawMesh(checked);
                             ui->openGLWidget_3d->doneCurrent();
                         }
                     });
    // 设置是否绘制point
    QObject::connect(ui->checkBox_draw_point_3d,
                     &QCheckBox::toggled,
                     this,
                     [&](bool checked) {
                         if (!do_not_run) {
                             auto &obj = objects[now_object_id];
                             ui->openGLWidget_3d->makeCurrent();
                             obj->setDrawPoints(checked);
                             ui->openGLWidget_3d->doneCurrent();
                             updated_objects = true;
                         }
                     });
    // 设置是否绘制line
    QObject::connect(ui->checkBox_draw_line_3d,
                     &QCheckBox::toggled,
                     this,
                     [&](bool checked) {
                         if (!do_not_run) {
                             auto &obj = objects[now_object_id];
                             ui->openGLWidget_3d->makeCurrent();
                             obj->setDrawLines(checked);
                             ui->openGLWidget_3d->doneCurrent();
                             updated_objects = true;
                         }
                     });

    // 同步3d界面的xyz
    QObject::connect(this,
                     SIGNAL(centerXChanged(int)),
                     ui->spinBox_x_3d,
                     SLOT(setValue(int)));
    QObject::connect(this,
                     SIGNAL(centerYChanged(int)),
                     ui->spinBox_y_3d,
                     SLOT(setValue(int)));
    QObject::connect(this,
                     SIGNAL(centerZChanged(int)),
                     ui->spinBox_z_3d,
                     SLOT(setValue(int)));
    QObject::connect(this,
                     SIGNAL(centerXChanged(int)),
                     ui->horizontalSlider_x_3d,
                     SLOT(setValue(int)));
    QObject::connect(this,
                     SIGNAL(centerYChanged(int)),
                     ui->horizontalSlider_y_3d,
                     SLOT(setValue(int)));
    QObject::connect(this,
                     SIGNAL(centerZChanged(int)),
                     ui->horizontalSlider_z_3d,
                     SLOT(setValue(int)));
    QObject::connect(ui->spinBox_x_3d,
                     SIGNAL(valueChanged(int)),
                     this,
                     SLOT(setCenterX(int)));
    QObject::connect(ui->spinBox_y_3d,
                     SIGNAL(valueChanged(int)),
                     this,
                     SLOT(setCenterY(int)));
    QObject::connect(ui->spinBox_z_3d,
                     SIGNAL(valueChanged(int)),
                     this,
                     SLOT(setCenterZ(int)));
    QObject::connect(ui->horizontalSlider_x_3d,
                     SIGNAL(valueChanged(int)),
                     this,
                     SLOT(setCenterX(int)));
    QObject::connect(ui->horizontalSlider_y_3d,
                     SIGNAL(valueChanged(int)),
                     this,
                     SLOT(setCenterY(int)));
    QObject::connect(ui->horizontalSlider_z_3d,
                     SIGNAL(valueChanged(int)),
                     this,
                     SLOT(setCenterZ(int)));
    // 是否显示切片
    QObject::connect(ui->checkBox_show_slice,
                     &QCheckBox::toggled,
                     this,
                     [&](bool checked) {
                         if (!do_not_run) {
                             show_slice = checked;
                             if (checked) {
                                 // 调用那3个函数
                                 display_xy_image_3dview(center_z);
                                 display_xz_image_3dview(center_y);
                                 display_yz_image_3dview(center_x);
                             }
                             ui->widget_x_3d->setEnabled(checked);
                             ui->widget_y_3d->setEnabled(checked);
                             ui->widget_z_3d->setEnabled(checked);
                             ui->openGLWidget_3d->setShowSlice(checked);
                         }
                     });

    // 在3d界面上更新了x值
    QObject::connect(ui->spinBox_x_3d,
                     &QSpinBox::valueChanged,
                     this,
                     [&](int value) {
                         if (!do_not_run && show_slice && on_3d_graphicsView) {
                             display_yz_image_3dview(value);
                         }
                     });
    // 在3d界面上更新了y值
    QObject::connect(ui->spinBox_y_3d,
                     &QSpinBox::valueChanged,
                     this,
                     [&](int value) {
                         if (!do_not_run && show_slice && on_3d_graphicsView) {
                             display_xz_image_3dview(value);
                         }
                     });
    // 在3d界面上更新了z值
    QObject::connect(ui->spinBox_z_3d,
                     &QSpinBox::valueChanged,
                     this,
                     [&](int value) {
                         if (!do_not_run && show_slice && on_3d_graphicsView) {
                             display_xy_image_3dview(value);
                         }
                     });

    timer_id_2 = startTimer(std::chrono::milliseconds(67));

    // 点击中键
    QObject::connect(ui->xy_only_graphicsView,
                     &GraphicsView::middleClicked,
                     this,
                     [&]() {
                         if (edit_mode) {
                             edit_mode = false;
                             ui->checkBox_editControl->setChecked(false);
                         }
                         if (middle_button_clicked) {
                             middle_button_clicked = false;
                         } else {
                             middle_button_clicked = true;
                         }
                     });
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (on_xy_only_graphicsView) {
        auto key = event->key();
        switch (key) {
            case Qt::Key_Right: {
                if (point_exist) {
                    auto &tmp = objects[now_object_id]->z_vertexArray[center_z].back().x;
                    ++tmp;
                    do_not_run = true;
                    ui->spinBox_point_x->setValue(tmp);
                    do_not_run = false;
                    updated_objects = true;
                }
            } break;
            case Qt::Key_Left: {
                if (point_exist) {
                    auto &tmp = objects[now_object_id]->z_vertexArray[center_z].back().x;
                    --tmp;
                    do_not_run = true;
                    ui->spinBox_point_x->setValue(tmp);
                    do_not_run = false;
                    updated_objects = true;
                }
            } break;
            case Qt::Key_Up: {
                if (point_exist) {
                    auto &tmp = objects[now_object_id]->z_vertexArray[center_z].back().y;
                    --tmp;
                    do_not_run = true;
                    ui->spinBox_point_y->setValue(tmp);
                    do_not_run = false;
                    updated_objects = true;
                }
            } break;
            case Qt::Key_Down: {
                if (point_exist) {
                    auto &tmp = objects[now_object_id]->z_vertexArray[center_z].back().y;
                    ++tmp;
                    do_not_run = true;
                    ui->spinBox_point_y->setValue(tmp);
                    do_not_run = false;
                    updated_objects = true;
                }
            } break;

            default:
                break;
        }
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::display_xy_image_3dview(const int &z) {
    // 显示xy切片
    QImage xy_image = binaryImages->getXYImage(center_x, center_y, z);
    ui->openGLWidget_3d->updateXYImage(xy_image, z);
}

void MainWindow::display_xz_image_3dview(const int &y) {
    // 显示xz切片
    QImage xz_image = binaryImages->getXZImage(center_x, y, center_z);
    ui->openGLWidget_3d->updateXZImage(xz_image, y);
}

void MainWindow::display_yz_image_3dview(const int &x) {
    // 显示yz切片
    QImage yz_image = binaryImages->getYZImage(x, center_y, center_z);
    ui->openGLWidget_3d->updateYZImage(yz_image, x);
}

void MainWindow::timerEvent(QTimerEvent *event) {
    int nowid = event->timerId();
    if (timer_id_1 == nowid) {
        object_exist = now_object_id != empty_object_id;
        ui->pushButton_delete_object->setEnabled(object_exist);
        ui->widget_color->setEnabled(object_exist);
        ui->widget_lineWidth->setEnabled(object_exist);
        ui->widget_radius->setEnabled(object_exist);
        ui->widget_object_draw_setting->setEnabled(object_exist);
        ui->pushButton_create_point->setEnabled(object_exist);
        ui->spinBox_object_id->setEnabled(object_exist);
        point_exist = object_exist && objects[now_object_id]->z_vertexArray.contains(center_z) && !objects[now_object_id]->z_vertexArray[center_z].isEmpty();
        ui->widget_point->setEnabled(point_exist);
        ui->pushButton_delete_point->setEnabled(point_exist);
        // 更新当前点的z值的显示
        ui->spinBox_point_z->setValue(center_z);
        // 更新当前点的x、y值的显示
        if (updated_objects) {
            drawXYImage();
        }
    } else if (middle_button_clicked && timer_id_2 == nowid) {
        if (on_xy_only_graphicsView) {
            int t_center_z = center_z + z_play_direction;
            // 到达上边界时反转方向
            if (t_center_z >= max_center_z) {
                z_play_direction = -1;
                t_center_z = center_z - 1;
            }
            // 到达下边界时反转方向
            else if (t_center_z < 1) {
                z_play_direction = 1;
                t_center_z = center_z + 1;
            }
            on_spinBox_z_valueChanged(t_center_z);
        } else {
            middle_button_clicked = false;
        }
    }
    QMainWindow::timerEvent(event);
}
MainWindow::~MainWindow() {
    delete ui;
    if (binaryImages != nullptr)
        delete binaryImages;
}

void MainWindow::on_save_model_file_triggered() {
    ui->pushButton_2d->click();  // 保存之前先切换到2d界面，尝试解决在Ubuntu中系统界面卡死的问题
    QString modelFilePath = QFileDialog::getSaveFileName(this,
                                                         "Select model file to save",
                                                         "/",
                                                         "model file (*.model);; all (*.*);;");
    if (modelFilePath.isEmpty()) {
        QMessageBox::warning(this,
                             "Notice",
                             "You have not selected any files!",
                             QMessageBox::Close,
                             QMessageBox::Close);
        return;
    }
    if (!modelFilePath.endsWith(".model")) {
        modelFilePath += ".model";
    }
    QFile file(modelFilePath);
    // 以只写方式打开文件，如果文件不存在，那么就创建该文件，如果文件存在，那么就清空该文件
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        QMessageBox::warning(this,
                             "Error",
                             "Can not open file: " + modelFilePath,
                             QMessageBox::Close,
                             QMessageBox::Close);
        return;
    }
    QTextStream out(&file);
    out << "1\n";  // 版本号,用于向后兼容,在读取文件时,如果版本号不匹配,那么就不读取该文件
    // Object的数量
    out << objects.size() << "\n";
    for (const auto &obj_ptr : objects) {
        const auto &obj = *obj_ptr;
        // Object的颜色
        out << obj.m_r << " " << obj.m_g << " " << obj.m_b << "\n";  // int
        // Object的线宽
        out << obj.m_lineWidth << "\n";  // float
        // Object的线的颜色
        out << obj.m_line_r << " " << obj.m_line_g << " " << obj.m_line_b << "\n";  // int
        // Object的半径
        out << obj.m_radius << "\n";  // float
        // Object的点的颜色
        out << obj.m_point_r << " " << obj.m_point_g << " " << obj.m_point_b << "\n";  // int
        // Object的是否闭合
        out << obj.m_closed << "\n";  // bool
        // Object的是否绘制mesh
        out << obj.m_drawMesh << "\n";  // bool
        // Object的是否绘制point
        out << obj.m_drawPoints << "\n";  // bool
        // Object的是否绘制line
        out << obj.m_drawLines << "\n";  // bool
        // Object的z_vertexArray的key的数量
        out << obj.z_vertexArray.size() << "\n";
        for (auto it = obj.z_vertexArray.constBegin(); it != obj.z_vertexArray.constEnd(); ++it) {
            // Object的z_vertexArray的key
            out << it.key() << "\n";  // int
            // Object的z_vertexArray的value的数量
            out << it.value().size() << "\n";
            for (const auto &vertex : it.value()) {
                // Object的z_vertexArray的value的x
                out << vertex.x << "\n";  // float
                // Object的z_vertexArray的value的y
                out << vertex.y << "\n";  // float
                // Object的z_vertexArray的value的z
                out << vertex.z << "\n";  // float
            }
        }
    }
    file.close();
    qDebug() << "save model file successfully";
}

void MainWindow::on_open_model_file_triggered() {
    QString modelFilePath = QFileDialog::getOpenFileName(this,
                                                         "Select model file to open",
                                                         "/",
                                                         "model file (*.model);; all (*.*);;");
    if (modelFilePath.isEmpty()) {
        QMessageBox::warning(this,
                             "Notice",
                             "You have not selected any files!",
                             QMessageBox::Close,
                             QMessageBox::Close);
        return;
    }
    QFile file(modelFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this,
                             "Error",
                             "Can not open file: " + modelFilePath,
                             QMessageBox::Close,
                             QMessageBox::Close);
        return;
    }
    QTextStream in(&file);
    int version;
    in >> version;
    if (version != 1) {
        QMessageBox::warning(this,
                             "Error",
                             "The version of the file is not supported!",
                             QMessageBox::Close,
                             QMessageBox::Close);
        return;
    }
    for (auto &obj_ptr : objects) {
        delete obj_ptr;
    }
    objects.clear();

    int object_count;
    in >> object_count;
    for (int i = 0; i < object_count; ++i) {
        int r, g, b;
        float lineWidth;
        int line_r, line_g, line_b;
        float radius;
        int point_r, point_g, point_b;
        int closed, drawMesh, drawPoints, drawLines;
        in >> r >> g >> b;
        in >> lineWidth;
        in >> line_r >> line_g >> line_b;
        in >> radius;
        in >> point_r >> point_g >> point_b;
        in >> closed;
        in >> drawMesh;
        in >> drawPoints;
        in >> drawLines;
        int z_vertexArray_size;
        in >> z_vertexArray_size;
        auto obj_ptr = new Object(lineWidth, radius, closed, r, g, b);
        auto &obj = *obj_ptr;
        obj.m_line_r = line_r;
        obj.m_line_g = line_g;
        obj.m_line_b = line_b;
        obj.m_point_r = point_r;
        obj.m_point_g = point_g;
        obj.m_point_b = point_b;
        obj.m_drawMesh = drawMesh;
        obj.m_drawPoints = drawPoints;
        obj.m_drawLines = drawLines;
        int key_z;
        float x, y, z;
        for (int j = 0; j < z_vertexArray_size; ++j) {
            in >> key_z;
            int vertex_size;
            in >> vertex_size;
            QVector<Vertex3> vertexArray;
            for (int k = 0; k < vertex_size; ++k) {
                in >> x >> y >> z;
                vertexArray.emplace_back(x, y, z);
            }
            obj.z_vertexArray[key_z] = vertexArray;
        }
        objects.emplace_back(obj_ptr);
    }
    file.close();
    updated_objects = true;
    // 打开文件后，当前的object_id就是最后一个object,如果没有object，那么就是empty_object_id
    if (object_count == 0) {
        now_object_id = empty_object_id;
        do_not_run = true;
        ui->spinBox_object_id->setMaximum(0);
        ui->spinBox_object_id_3d->setMaximum(0);
        do_not_run = false;
    } else {
        now_object_id = object_count - 1;
        ui->spinBox_object_id->setMaximum(now_object_id);
        ui->spinBox_object_id->setValue(now_object_id);
        ui->spinBox_object_id_3d->setMaximum(now_object_id);
        updateNowObjectDisplay_2d();
    }
    ui->pushButton_2d->click();
    ui->pushButton_xy_only->click();
    qDebug() << "open model file successfully";
    qDebug() << "objects.size()=" << objects.size();
}

void MainWindow::on_open_mrc_file_triggered() {
    qDebug() << "pressed open mrc file";
    QString mrcFilePath = QFileDialog::getOpenFileName(this,
                                                       "Select mrc file to open",
                                                       "/",
                                                       "mrc file (*.mrc *.map *.binaryData);; all (*.*);; ");
    qDebug() << "QFileDialog mrcFilePath=" << mrcFilePath;
    open_mrc_file(mrcFilePath);
}

void MainWindow::open_mrc_file(QString mrcFilePath) {
    
    if (mrcFilePath.isEmpty()) {
        QMessageBox::warning(this,
                             "Notice",
                             "You have not selected any files!",
                             QMessageBox::Close,
                             QMessageBox::Close);
        return;
    }

    QFileInfo mrcFileInfo = QFileInfo(mrcFilePath);
    mrcFilePath = mrcFileInfo.absoluteFilePath();
    qDebug() << "fileInfo mrcFilePath=" << mrcFilePath;
    if (mrcFilePath.endsWith(".binaryData", Qt::CaseSensitive)) {
        qDebug() << "read binary file";
        bool loadSuccess = this->binaryImages->load(mrcFilePath.toStdString());  // 注意这里的路径不能有中文
        if (!loadSuccess) {
            QMessageBox::critical(this,
                                "Load Error",
                                "Failed to load binary file: " + mrcFilePath + "\nPlease check the console for details.",
                                QMessageBox::Close);
            return;
        }
        qDebug() << "show binary file";
        //  应该发送一个信号来让xy_only_graphicsView、以及其他的graphicsView显示出来，例如centralXChange、centralYChange、centralZChange
        uint32_t *shape = this->binaryImages->get_shape();
        if (shape[0] == 0 || shape[1] == 0 || shape[2] == 0) {
            QMessageBox::critical(this,
                                "Load Error",
                                "Invalid data shape, file may be corrupted.",
                                QMessageBox::Close);
            return;
        }
        setMaxCenterX(shape[2]-1), setMaxCenterY(shape[1]-1), setMaxCenterZ(shape[0]-1);  // 更新编辑框的最大值

        // 阻塞信号，避免在数据加载过程中触发槽函数
        {
            QSignalBlocker blocker1(ui->spinBox_x);
            QSignalBlocker blocker2(ui->spinBox_y);
            QSignalBlocker blocker3(ui->spinBox_z);
            QSignalBlocker blocker4(ui->horizontalSlider_x);
            QSignalBlocker blocker5(ui->horizontalSlider_y);
            QSignalBlocker blocker6(ui->horizontalSlider_z);

            setCenterX(shape[2] >> 1), setCenterY(shape[1] >> 1), setCenterZ(shape[0] >> 1);
        }

        xy_only_image = this->binaryImages->getXYImage(
            center_x,
            center_y,
            center_z);  // 先凑合着，后续要在mainwindow的槽函数对各个graphicsView进行更新
        drawXYImage();
        ui->openGLWidget_arbitrarily->setMaxXYZ((int)shape[2], (int)shape[0], (int)shape[1]);  // 因为OpenGL的坐标系不太一样
        ui->pushButton_2d->click();
        ui->pushButton_xy_only->click();
        ui->checkBox_show_slice->setEnabled(true);
        return;
    }
    QDir dir(mrcFileInfo.canonicalPath());
    QString binaryFilePath = dir.absoluteFilePath(mrcFileInfo.completeBaseName() + ".binaryData");
    qDebug() << "binaryFilePath=" << binaryFilePath;

    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Loading...");
    dialog->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);  // 隐藏关闭按钮
    QMovie *movie = new QMovie(":/loading/lib/loading.gif");
    QLabel *label = new QLabel;
    QHBoxLayout *l = new QHBoxLayout(dialog);
    l->addWidget(label);
    label->setMovie(movie);
    movie->start();

    QStringList arguments;
    arguments << "-c"
              << "import sys\n"
                 "import ncempy.io as nio  # 使用该命令来安装 pip install ncempy\n"
                 "import numpy as np\n"
                 "def load_to_ndarray(file_path: str) -> np.ndarray:\n"
                 "    src = nio.mrc.mrcReader(file_path)\n"
                 "    data = src['data'].copy()\n"
                 "    src.clear()\n"
                 "    return data\n"
                 "def scale_image_to_grayscale(data: np.ndarray) -> np.ndarray:\n"
                 "    mi = data.min()\n"
                 "    ma = data.max()\n"
                 "    return ((data - mi) / (ma - mi)) * 255\n"
                 "def save_to_binary(file_path: str, data: np.ndarray):\n"
                 "    fileobj = open(file_path, mode='wb')\n"
                 "    np.asarray(data.shape, dtype=np.dtype('uint32')).tofile(fileobj)\n"
                 "    data.astype('uint8').tofile(fileobj)\n"
                 "    fileobj.close()\n"
                 "if __name__ == '__main__':\n"
                 "    assert len(sys.argv) == 3,'参数必须是这个格式：mrc2binary.py mrc文件的路径 "
                 "输出文件的路径'\n"
                 "    input_file_path = sys.argv[1]\n"
                 "    output_file_path = sys.argv[2]\n"
                 "    print(f'Try to load the file from {input_file_path}')\n"
                 "    data = load_to_ndarray(input_file_path)\n"
                 "    print(f'Loaded successfully')\n"
                 "    data = scale_image_to_grayscale(data)\n"
                 "    print(f'Try to save the file to {output_file_path}')\n"
                 "    save_to_binary(output_file_path, data)\n"
                 "    print(f'Saved successfully , data.shape={data.shape}')\n"
                 "    print('Done')\n"
              << mrcFilePath << binaryFilePath;

    QProcess *process = new QProcess();
    process->setProcessChannelMode(QProcess::MergedChannels);
    // 解决process没有启动的情况
    bool show_loading = true;
    QObject::connect(process,
                     QOverload<QProcess::ProcessError>::of(&QProcess::errorOccurred),
                     this,
                     [&](QProcess::ProcessError error) {
                         qDebug() << "errorOccurred:" << error;
                         show_loading = false;
                         if (dialog->isVisible())
                             dialog->close();
                         QMessageBox::warning(nullptr,
                                              "Error",
                                              "Can not start process: " + process->program() + " \n" + process->errorString() + "\n请在本应用的设置中设置正确的python可执行文件的路径",
                                              QMessageBox::Close,
                                              QMessageBox::Close);
                         process->deleteLater();
                         dialog->deleteLater();
                     });

    QObject::connect(
        process,
        QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        this,  // 解决报错：Pass a context object as 3rd connect parameter [clazy-connect-3arg-lambda]
        [&](int exitCode, QProcess::ExitStatus) {
            QString a = QString::fromLocal8Bit(process->readAllStandardOutput());
            qDebug() << a;
            bool ok = true;
            if (!a.contains("Done")) {
                ok = false;
                if (dialog->isVisible())
                    dialog->close();
                QMessageBox::warning(nullptr, "Error", a, QMessageBox::Close, QMessageBox::Close);
            }
            qDebug() << "process exited with code " << exitCode;
            if (ok) {
                // 在这里读入那个二进制文件，然后显示出来
                qDebug() << "read binary file";
                bool loadSuccess = this->binaryImages->load(binaryFilePath.toStdString());  // 注意这里的路径不能有中文
                if (!loadSuccess) {
                    qDebug() << "Failed to load binary file";
                    if (dialog->isVisible())
                        dialog->close();
                    QMessageBox::critical(nullptr,
                                        "Load Error",
                                        "Failed to load binary file: " + binaryFilePath + "\nPlease check the console for details.",
                                        QMessageBox::Close);
                    process->deleteLater();
                    dialog->deleteLater();
                    return;
                }
                qDebug() << "show binary file";
                //  应该发送一个信号来让xy_only_graphicsView、以及其他的graphicsView显示出来，例如centralXChange、centralYChange、centralZChange
                qDebug() << "About to call get_shape()";
                uint32_t *shape = this->binaryImages->get_shape();
                qDebug() << "get_shape() returned, shape[0]=" << shape[0] << "shape[1]=" << shape[1] << "shape[2]=" << shape[2];
                if (shape[0] == 0 || shape[1] == 0 || shape[2] == 0) {
                    qDebug() << "Invalid data shape";
                    if (dialog->isVisible())
                        dialog->close();
                    QMessageBox::critical(nullptr,
                                        "Load Error",
                                        "Invalid data shape, file may be corrupted.",
                                        QMessageBox::Close);
                    process->deleteLater();
                    dialog->deleteLater();
                    return;
                }
                qDebug() << "Shape is valid, about to setMaxCenter...";
                setMaxCenterX(shape[2]-1), setMaxCenterY(shape[1]-1), setMaxCenterZ(shape[0]-1);  // 更新编辑框的最大值
                qDebug() << "setMaxCenter done, about to setCenter...";

                // 阻塞信号，避免在数据加载过程中触发槽函数
                {
                    QSignalBlocker blocker1(ui->spinBox_x);
                    QSignalBlocker blocker2(ui->spinBox_y);
                    QSignalBlocker blocker3(ui->spinBox_z);
                    QSignalBlocker blocker4(ui->horizontalSlider_x);
                    QSignalBlocker blocker5(ui->horizontalSlider_y);
                    QSignalBlocker blocker6(ui->horizontalSlider_z);

                    setCenterX(shape[2] >> 1), setCenterY(shape[1] >> 1), setCenterZ(shape[0] >> 1);
                }

                qDebug() << "setCenter done, about to call getXYImage()";
                xy_only_image = this->binaryImages->getXYImage(
                    center_x,
                    center_y,
                    center_z);  // 先凑合着，后续要在mainwindow的槽函数对各个graphicsView进行更新
                qDebug() << "getXYImage done, about to call drawXYImage()";
                drawXYImage();
                qDebug() << "drawXYImage done, about to set OpenGL widget...";
                ui->openGLWidget_arbitrarily->setMaxXYZ((int)shape[2], (int)shape[0], (int)shape[1]);  // 因为OpenGL的坐标系不太一样
                qDebug() << "OpenGL widget set, about to click buttons...";
                ui->pushButton_2d->click();
                qDebug() << "pushButton_2d clicked";
                ui->pushButton_xy_only->click();
                qDebug() << "pushButton_xy_only clicked";
                ui->checkBox_show_slice->setEnabled(true);
                qDebug() << "All done successfully!";
                // 还要重置亮度和对比度
            }
            if (dialog->isVisible())
                dialog->close();
            process->deleteLater();
            dialog->deleteLater();
        });

    QSettings settings;
    process->start(settings.value("python_path").toString(), arguments);
    if (show_loading)
        dialog->exec();

    qDebug() << "end of on_open_mrc_file_triggered";
}

void MainWindow::on_spinBox_z_valueChanged(int arg1) {  // 只要捕捉了这个信号，就会导致拖动滑动条期间也会不断的调用这个函数，如果文件很大会导致界面卡顿，除非给滑动条的tracking设置为false
    setCenterZ(arg1);
    if (on_xy_only_graphicsView || on_xy_xz_yz) {
        xy_only_image = this->binaryImages->getXYImage(
            center_x,
            center_y,
            center_z);  // 先凑合着，后续要在mainwindow的槽函数对各个graphicsView进行更新
        updated_objects = true;
        drawXYImage();
    } else if (on_arbitrarily) {
        arbitrarily_image = this->binaryImages->clip_slow(
            center_x,
            center_y,
            center_z,
            rotate_x_deg,
            rotate_y_deg,
            rotate_z_deg);
        drawArbitrarilyImage();
    }
}

QImage MainWindow::preProcessImage(const QImage &image) {
    return lightContrastImage(image, light, contrast);
}

void MainWindow::drawXYImage() {
    QImage tmp = preProcessImage(xy_only_image);
    ui->xy_graphicsView->SetImage(tmp);
    tmp = tmp.convertToFormat(QImage::Format_RGB32);
    if (updated_objects) {  // 有可能当前的图片是空的但也被触发了update，因为在切换按钮那里设置了updated_objects = true，不过没关系，因为QPainter会自动处理这种情况
        drawModel(tmp, objects, center_z);
        updated_objects = false;
    }
    if (draw_arrow && !arrowPoints.isEmpty()) {
        tmp = drawArrow(tmp, arrowPoints[0].first, arrowPoints[0].second, arrowPoints[1].first, arrowPoints[1].second, 12, 245, 116);
    }
    if (draw_rectangle && !rectanglePoints.isEmpty()) {
        tmp = drawRectangle(tmp, rectanglePoints[0].first, rectanglePoints[0].second, rectanglePoints[1].first, rectanglePoints[1].second, 208, 252, 179);
    }
    if (draw_polyline && !polylinePoints.isEmpty()) {
        tmp = drawPolyline(tmp, polylinePoints, 200, 173, 85);
    }
    ui->xy_only_graphicsView->SetImage(tmp);
}

void MainWindow::drawXZImage() {
    ui->xz_graphicsView->SetImage(preProcessImage(xz_only_image));
}

void MainWindow::drawYZImage() {
    ui->yz_graphicsView->SetImage(preProcessImage(yz_only_image));
}

void MainWindow::drawArbitrarilyImage(const bool &use_last_center) {
    ui->arbitrarily_graphicsView->SetImage(preProcessImage(arbitrarily_image), use_last_center);
}

void MainWindow::on_horizontalSlider_light_valueChanged(int value) {
    light = value;
    // 还要调用某个方法更新界面的图像（还没有写这个函数）
    drawXYImage();
    drawXZImage();
    drawYZImage();
    drawArbitrarilyImage();
}

void MainWindow::on_horizontalSlider_contrast_valueChanged(int value) {
    contrast = value;
    drawXYImage();
    drawXZImage();
    drawYZImage();
    drawArbitrarilyImage();
}

void MainWindow::on_pushButton_xy_only_clicked() {
    on_xy_only_graphicsView = true;
    on_xy_xz_yz = false;
    on_arbitrarily = false;
    ui->pushButton_xy_only->setChecked(true);
    ui->pushButton_xy_xz_yz->setChecked(false);
    ui->pushButton_arbitrarily->setChecked(false);
    ui->view_stackedWidget->setCurrentIndex(0);
    updated_objects = true;  // 因为之前已经绘制了（会设置为false），如果不设置为true，就不会更新
    on_spinBox_x_valueChanged(ui->spinBox_x->value());
    ui->widget_rotate_x_deg->hide();
    ui->widget_rotate_y_deg->hide();
    ui->widget_rotate_z_deg->hide();
    ui->openGLWidget_arbitrarily->hide();
    ui->widget_editControl->show();
    ui->widget_arrow_rectangle_polyline->show();
    setFocusPolicy(Qt::StrongFocus);  // 捕获键盘事件
}

void MainWindow::on_pushButton_xy_xz_yz_clicked() {
    on_xy_only_graphicsView = false;
    on_xy_xz_yz = true;
    on_arbitrarily = false;
    ui->pushButton_xy_xz_yz->setChecked(true);
    ui->pushButton_xy_only->setChecked(false);
    ui->pushButton_arbitrarily->setChecked(false);
    ui->view_stackedWidget->setCurrentIndex(1);
    on_spinBox_x_valueChanged(ui->spinBox_x->value());
    on_spinBox_y_valueChanged(ui->spinBox_y->value());
    on_spinBox_z_valueChanged(ui->spinBox_z->value());
    ui->widget_rotate_x_deg->hide();
    ui->widget_rotate_y_deg->hide();
    ui->widget_rotate_z_deg->hide();
    ui->openGLWidget_arbitrarily->hide();
    ui->widget_editControl->hide();
    ui->widget_arrow_rectangle_polyline->hide();
    setFocusPolicy(Qt::NoFocus);  // 不捕获键盘事件
}

void MainWindow::on_pushButton_arbitrarily_clicked() {
    on_xy_only_graphicsView = false;
    on_xy_xz_yz = false;
    on_arbitrarily = true;
    ui->pushButton_arbitrarily->setChecked(true);
    ui->pushButton_xy_xz_yz->setChecked(false);
    ui->pushButton_xy_only->setChecked(false);
    ui->view_stackedWidget->setCurrentIndex(2);
    arbitrarily_image = this->binaryImages->clip_slow(center_x,
                                                      center_y,
                                                      center_z,
                                                      rotate_x_deg,
                                                      rotate_y_deg,
                                                      rotate_z_deg);
    drawArbitrarilyImage(false);
    ui->widget_rotate_x_deg->show();
    ui->widget_rotate_y_deg->show();
    ui->widget_rotate_z_deg->show();
    ui->widget_editControl->hide();
    ui->openGLWidget_arbitrarily->show();
    ui->widget_arrow_rectangle_polyline->hide();
    setFocusPolicy(Qt::NoFocus);  // 不捕获键盘事件
}

void MainWindow::on_spinBox_x_valueChanged(int arg1) {
    setCenterX(arg1);
    if (on_xy_xz_yz) {
        yz_only_image = this->binaryImages->getYZImage(
            center_x,
            center_y,
            center_z);
        drawYZImage();
    } else if (on_arbitrarily) {
        arbitrarily_image = this->binaryImages->clip_slow(
            center_x,
            center_y,
            center_z,
            rotate_x_deg,
            rotate_y_deg,
            rotate_z_deg);
        drawArbitrarilyImage();
    }
}

void MainWindow::on_spinBox_y_valueChanged(int arg1) {
    setCenterY(arg1);
    if (on_xy_xz_yz) {
        xz_only_image = this->binaryImages->getXZImage(
            center_x,
            center_y,
            center_z);
        drawXZImage();
    } else if (on_arbitrarily) {
        arbitrarily_image = this->binaryImages->clip_slow(
            center_x,
            center_y,
            center_z,
            rotate_x_deg,
            rotate_y_deg,
            rotate_z_deg);
        drawArbitrarilyImage();
    }
}

void MainWindow::on_spinBox_rotate_x_deg_valueChanged(int arg1) {
    setRotateXDeg(arg1);
    if (on_arbitrarily) {
        arbitrarily_image = this->binaryImages->clip_slow(
            center_x,
            center_y,
            center_z,
            rotate_x_deg,
            rotate_y_deg,
            rotate_z_deg);
        drawArbitrarilyImage();
    }
}

void MainWindow::on_spinBox_rotate_y_deg_valueChanged(int arg1) {
    setRotateYDeg(arg1);
    if (on_arbitrarily) {
        arbitrarily_image = this->binaryImages->clip_slow(
            center_x,
            center_y,
            center_z,
            rotate_x_deg,
            rotate_y_deg,
            rotate_z_deg);
        drawArbitrarilyImage();
    }
}

void MainWindow::on_spinBox_rotate_z_deg_valueChanged(int arg1) {
    setRotateZDeg(arg1);
    if (on_arbitrarily) {
        arbitrarily_image = this->binaryImages->clip_slow(
            center_x,
            center_y,
            center_z,
            rotate_x_deg,
            rotate_y_deg,
            rotate_z_deg);
        drawArbitrarilyImage();
    }
}

void MainWindow::on_pushButton_2d_clicked() {
    do_not_run = true;
    updateNowObjectDisplay_2d();
    do_not_run = false;
    on_3d_graphicsView = false;
    ui->pushButton_xy_only->click();
    ui->controlView_stackedWidget->setCurrentIndex(0);
    ui->pushButton_2d->setChecked(true);
    ui->pushButton_3d->setChecked(false);
}

void MainWindow::on_pushButton_3d_clicked() {
    setFocusPolicy(Qt::NoFocus);  // 不捕获键盘事件
    if (now_object_id == empty_object_id) {
        ui->widget_object_id_3d->setEnabled(false);
        ui->widget_color_3d->setEnabled(false);
        ui->widget_lineWidth_3d->setEnabled(false);
        ui->widget_radius_3d->setEnabled(false);
        ui->widget_object_draw_setting_3d->setEnabled(false);
    } else {
        ui->widget_object_id_3d->setEnabled(true);
        ui->widget_color_3d->setEnabled(true);
        ui->widget_lineWidth_3d->setEnabled(true);
        ui->widget_radius_3d->setEnabled(true);
        ui->widget_object_draw_setting_3d->setEnabled(true);
        auto &obj = objects[now_object_id];
        do_not_run = true;
        ui->spinBox_object_id_3d->setValue(now_object_id);
        ui->doubleSpinBox_lineWidth_3d->setValue(obj->m_lineWidth);
        ui->doubleSpinBox_radius_3d->setValue(obj->m_radius);
        ui->checkBox_closed_3d->setChecked(obj->m_closed);
        ui->checkBox_draw_mesh_3d->setChecked(obj->m_drawMesh);
        ui->checkBox_draw_point_3d->setChecked(obj->m_drawPoints);
        ui->checkBox_draw_line_3d->setChecked(obj->m_drawLines);
        do_not_run = false;
        ui->pushButton_mesh_color_3d->setStyleSheet(
            QString("background-color: rgb(%1,%2,%3);")
                .arg(obj->m_r)
                .arg(obj->m_g)
                .arg(obj->m_b));
        ui->pushButton_line_color_3d->setStyleSheet(
            QString("background-color: rgb(%1,%2,%3);")
                .arg(obj->m_line_r)
                .arg(obj->m_line_g)
                .arg(obj->m_line_b));
        ui->pushButton_point_color_3d->setStyleSheet(
            QString("background-color: rgb(%1,%2,%3);")
                .arg(obj->m_point_r)
                .arg(obj->m_point_g)
                .arg(obj->m_point_b));
    }
    on_3d_graphicsView = true;
    on_xy_only_graphicsView = false, on_xy_xz_yz = false, on_arbitrarily = false;
    ui->controlView_stackedWidget->setCurrentIndex(1);
    ui->pushButton_3d->setChecked(true);
    ui->pushButton_2d->setChecked(false);
    ui->openGLWidget_3d->setObjects(objects);  // 必须要在显示了之后才能调用，否则可能还没有创建QOpenGLContext
}

void MainWindow::disable1() {
    if (cnt1 != 1) {
        --cnt1;
        return;
    }
    --cnt1;
    QObject::disconnect(ui->xy_only_graphicsView,  // 让鼠标右键点击的时候，设置中心点
                        SIGNAL(rightClickedX(int)),
                        this,
                        SLOT(setCenterX(int)));
    QObject::disconnect(ui->xy_only_graphicsView,
                        SIGNAL(rightClickedY(int)),
                        this,
                        SLOT(setCenterY(int)));
}

void MainWindow::enable1() {
    if (cnt1 != 0) {
        ++cnt1;
        return;
    }
    ++cnt1;
    QObject::connect(ui->xy_only_graphicsView,  // 让鼠标右键点击的时候，设置中心点
                     SIGNAL(rightClickedX(int)),
                     this,
                     SLOT(setCenterX(int)));
    QObject::connect(ui->xy_only_graphicsView,
                     SIGNAL(rightClickedY(int)),
                     this,
                     SLOT(setCenterY(int)));
}
void MainWindow::disable2() {
    if (cnt2 != 1) {
        --cnt2;
        return;
    }
    --cnt2;
    QObject::disconnect(ui->xy_only_graphicsView,
                        SIGNAL(rightClickedXY(int, int)),
                        this,
                        SLOT(rightClickReceiver(int, int)));
}
void MainWindow::enable2() {
    if (cnt2 != 0) {
        ++cnt2;
        return;
    }
    ++cnt2;
    QObject::connect(ui->xy_only_graphicsView,
                     SIGNAL(rightClickedXY(int, int)),
                     this,
                     SLOT(rightClickReceiver(int, int)));
}

void MainWindow::rightClickReceiver(int x, int y) {
    if (edit_arrow) {
        if (arrowPointsBuffer.size() > 1) {
            arrowPointsBuffer.clear();
        }
        arrowPointsBuffer.emplaceBack(x, y);
        if (arrowPointsBuffer.size() == 2) {
            arrowPoints = arrowPointsBuffer;
            arrowPointsBuffer.clear();
            updated_objects = true;
            drawXYImage();
        }
    }
    if (edit_rectangle) {
        if (rectanglePointsBuffer.size() > 1) {
            rectanglePointsBuffer.clear();
        }
        rectanglePointsBuffer.emplaceBack(x, y);
        if (rectanglePointsBuffer.size() == 2) {
            rectanglePoints = rectanglePointsBuffer;
            rectanglePointsBuffer.clear();
            updated_objects = true;
            drawXYImage();
        }
    }
    if (edit_polyline) {
        polylinePoints.emplaceBack(x, y);
        updated_objects = true;
        drawXYImage();
    }
}

void MainWindow::on_checkBox_editControl_stateChanged(int arg1) {
    switch (arg1) {
        case Qt::Unchecked: {
            ui->editControl_widget->setEnabled(false);
            edit_mode = false;
            enable1();
            ui->pushButton_arrow->setChecked(false);
            ui->pushButton_rectangle->setChecked(false);
            ui->pushButton_polyline->setChecked(false);
            ui->widget_arrow_rectangle_polyline->setEnabled(true);
            break;
        }
        case Qt::Checked: {
            ui->editControl_widget->setEnabled(true);
            edit_mode = true;
            disable1();  // 禁用鼠标右键设置中心点
            ui->pushButton_arrow->setChecked(false);
            ui->pushButton_rectangle->setChecked(false);
            ui->pushButton_polyline->setChecked(false);
            ui->pushButton_arrow->setChecked(false);
            ui->pushButton_rectangle->setChecked(false);
            ui->pushButton_polyline->setChecked(false);
            ui->widget_arrow_rectangle_polyline->setEnabled(false);
            break;
        }
        default:
            break;
    }
}

void MainWindow::on_set_python_path_triggered() {
// 选择python可执行文件的路径
#ifdef Q_OS_WIN
    QString pythonPath = QFileDialog::getOpenFileName(this,
                                                      "Select python executable file",
                                                      "/",
                                                      "python executable file (python.exe);; all (*.*);; ");
#elif defined Q_OS_LINUX
    QString pythonPath = QFileDialog::getOpenFileName(this,
                                                      "Select python executable file",
                                                      "/",
                                                      "python executable file (python);; all (*);; ");
#endif
    if (pythonPath.isEmpty()) {
        QMessageBox::warning(this,
                             "Notice",
                             "You have not selected any files!",
                             QMessageBox::Close,
                             QMessageBox::Close);
        return;
    }
    QFileInfo pythonFileInfo = QFileInfo(pythonPath);
    pythonPath = pythonFileInfo.absoluteFilePath();
    qDebug() << "pythonPath=" << pythonPath;
    QSettings settings;
    settings.setValue("python_path", pythonPath);
}
