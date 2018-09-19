
// Interface.
#include "mainwindow.h"
#include "ui_mainwindow.h"

// couscous includes.
#include "renderer/camera.h"
#include "renderer/material.h"
#include "renderer/ray.h"
#include "renderer/visualobject.h"
#include "test/test.h"

// glm includes.
#include <glm/glm.hpp>

// Standard includes.
#include <memory>

using namespace glm;
using namespace std;

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , m_image(512, 512, QImage::Format_RGB888)
  , m_frame_viewer(512, 512)
{
    ui->setupUi(this);

    // Add the image viewer.
    ui->viewer_container_layout->addWidget(&m_frame_viewer);
    m_frame_viewer.on_render_end(m_image);
    m_render = new Render();

    // Connect widgets events.
    connect(ui->pushButton_render, SIGNAL(released()), SLOT(slot_do_render()));
    connect(ui->actionSave_As_Image, SIGNAL(triggered()), SLOT(slot_save_as_image()));
    connect(ui->pushButton_zoom_in, SIGNAL(released()), SLOT(slot_zoom_in()));
    connect(ui->pushButton_zoom_out, SIGNAL(released()), SLOT(slot_zoom_out()));
    connect(ui->actionRun_Unit_Test, SIGNAL(triggered()), SLOT(slot_run_unit_test()));
    connect(m_render, SIGNAL(render_new_tile()), SLOT(slot_render_new_tile()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slot_do_render()
{
    const size_t width = size_t(ui->spinBox_width->value());
    const size_t height = size_t(ui->spinBox_height->value());
    const size_t samples = size_t(ui->spinBox_spp->value());
    const size_t ray_max_depth = size_t(ui->spinBox_ray_max_depth->value());
    m_image = QImage(int(width), int(height), QImage::Format_RGB888);

    const float pos_x = float(ui->doubleSpinBox_position_x->value());
    const float pos_y = float(ui->doubleSpinBox_position_y->value());
    const float pos_z = float(ui->doubleSpinBox_position_z->value());
    const float yaw   = float(ui->doubleSpinBox_yaw->value());
    const float pitch = float(ui->doubleSpinBox_pitch->value());
    const float fov   = float(ui->doubleSpinBox_fov->value());

    const bool parallel = ui->checkBox_parallel_rendering->isChecked();
    const bool preview = ui->checkBox_real_time_preview->isChecked();

    m_frame_viewer.on_render_begin(width, height);

    Camera camera(vec3(pos_x, pos_y, pos_z), vec3(0.0f, 1.0f, 0.0f),
        yaw, pitch, fov, width, height);

    // Create materials.
    shared_ptr<Material> mat_light(new Light(vec3(15.0f)));
    shared_ptr<Material> mat_dull_light(new Light(vec3(1.0f)));
    shared_ptr<Material> mat_red(new Lambertian(vec3(0.65f, 0.05f, 0.05f)));
    shared_ptr<Material> mat_green(new Lambertian(vec3(0.12f, 0.45f, 0.15f)));
    shared_ptr<Material> mat_white(new Lambertian(vec3(0.73f, 0.73f, 0.73f)));

    // Create objects.
    VisualObjectList world;
    world.add(new Quad(
        vec3(213.0f, 554.0f, 227.0f),
        vec3(343.0f, 554.0f, 227.0f),
        vec3(343.0f, 554.0f, 332.0f),
        vec3(213.0f, 554.0f, 332.0f),
        mat_light));
    world.add(new Quad(
        vec3(555.0f, 0.0f, 0.0f),
        vec3(555.0f, 0.0f, 555.0f),
        vec3(555.0f, 555.0f, 555.0f),
        vec3(555.0f, 555.0f, 0.0f),
        mat_green));
    world.add(new Quad(
        vec3(0.0f, 0.0f, 0.0f),
        vec3(0.0f, 555.0f, 0.0f),
        vec3(0.0f, 555.0f, 555.0f),
        vec3(0.0f, 0.0f, 555.0f),
        mat_red));
    world.add(new Quad(
        vec3(555.0f, 0.0f, 0.0f),
        vec3(0.0f, 0.0f, 0.0f),
        vec3(0.0f, 555.0f, 0.0f),
        vec3(555.0f, 555.0f, 0.0f),
        mat_white));
    world.add(new Quad(
        vec3(555.0f, 0.0f, 555.0f),
        vec3(0.0f, 0.0f, 555.0f),
        vec3(0.0f, 0.0f, 0.0f),
        vec3(555.0f, 0.0f, 0.0f),
        mat_dull_light));
    world.add(new Quad(
        vec3(555.0f, 555.0f, 555.0f),
        vec3(0.0f, 555.0f, 555.0f),
        vec3(0.0f, 555.0f, 0.0f),
        vec3(555.0f, 555.0f, 0.0f),
        mat_white));

    world.add(new Box(
        vec3(400.0f, 0.0f, 100.0f),
        70.0f,
        150.0f,
        70.0f,
        mat_white));

    world.add(new Box(
        vec3(100.0f, 0.0f, 400.0f),
        70.0f,
        70.0f,
        70.0f,
        mat_white));

    QTime render_timer;
    render_timer.start();

    m_render->get_render_image(width, height, samples, ray_max_depth, camera, world, parallel, preview, m_image);

    QString message = "Rendering time : " + QString::number(render_timer.elapsed()) + " ms";

    m_frame_viewer.on_render_end(m_image);

    ui->statusBar->showMessage(message);
}

void MainWindow::slot_save_as_image()
{
    // Create file filters.
    QString selected_filter =  "PNG (*.png)";

    QStringList filter_list;
    filter_list << selected_filter;
    filter_list << "JPG (*.jpg)";
    filter_list << "JPEG (*.jpeg)";
    filter_list << "PBM (*.pbm)";
    filter_list << "All Files (*.*)";
    /*
    QString path = QFileDialog::getSaveFileName(
        this,
        tr("Save Image"),
        QDir::currentPath(),
        filter_list.join(";;"),
        &selected_filter);*/
    QString path = "toto";
    if (path.isEmpty())
        return;

    QFileInfo file_info(path);

    // Set the default extension.
    // If the file name has no extension and the selected filter has a single extension
    // (i.e. it contains one *.ext substring) then add that extension to the file name.
    if (file_info.suffix().isEmpty() && selected_filter.count('*') == 1)
    {
        const int begin = selected_filter.indexOf("*") + 1;
        const int end = selected_filter.indexOf(")", begin);
        assert(begin > 0 && end > begin);
        path += selected_filter.mid(begin, end - begin);
    }

    m_image.save(path);
}

void MainWindow::slot_zoom_in()
{
    m_frame_viewer.zoom_viewport(0.1f);
}

void MainWindow::slot_zoom_out()
{
    m_frame_viewer.zoom_viewport(-0.1f);
}

void MainWindow::slot_run_unit_test()
{
    int result = test::run();

    QMessageBox::information(this, "Unit Test", "Unit Test return code : " + QString::number(result));
    QCoreApplication::quit();
}

void MainWindow::slot_render_new_tile()
{
    m_frame_viewer.on_render_end(m_image);
}

