
// Interface.
#include "mainwindow.h"
#include "ui_mainwindow.h"

// couscous includes.
#include "renderer/camera.h"
#include "renderer/material.h"
#include "renderer/gridaccelerator.h"
#include "renderer/ray.h"
#include "renderer/visualobject.h"
#include "test/test.h"

// glm includes.
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Standard includes.
#include <memory>

using namespace glm;
using namespace std;

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , m_image(512, 512, QImage::Format_RGB888)
  , m_frame_viewer(512, 512)
  , m_statusBarProgress(this)
{
    ui->setupUi(this);

    ui->statusBar->addPermanentWidget(&m_statusBarProgress);
    m_statusBarProgress.setVisible(false);

    // Add the image viewer.
    ui->viewer_container_layout->addWidget(&m_frame_viewer);

    // Connect widgets events.
    connect(ui->pushButton_render, SIGNAL(released()), SLOT(slot_do_render()));
    connect(ui->actionSave_As_Image, SIGNAL(triggered()), SLOT(slot_save_as_image()));
    connect(ui->pushButton_zoom_in, SIGNAL(released()), SLOT(slot_zoom_in()));
    connect(ui->pushButton_zoom_out, SIGNAL(released()), SLOT(slot_zoom_out()));
    connect(ui->actionRun_Unit_Test, SIGNAL(triggered()), SLOT(slot_run_unit_test()));
    connect(
        &m_render,
        SIGNAL(on_tile_end(size_t, size_t, size_t, size_t, QImage)),
        &m_frame_viewer,
        SLOT(update_tile(size_t, size_t, size_t, size_t, QImage)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slot_do_render()
{
    ui->pushButton_render->setEnabled(false);

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
    // Create the scene.
    Camera camera(vec3(pos_x, pos_y, pos_z), vec3(0.0f, 1.0f, 0.0f),
        yaw, pitch, fov, width, height);

    // Create materials.
    shared_ptr<Material> mat_light(new Light(vec3(15.0f)));
    shared_ptr<Material> mat_red(new Lambertian(vec3(0.65f, 0.05f, 0.05f)));
    shared_ptr<Material> mat_green(new Lambertian(vec3(0.12f, 0.45f, 0.15f)));
    shared_ptr<Material> mat_white(new Lambertian(vec3(0.73f, 0.73f, 0.73f)));

    // Create cornell box.
    MeshGroup world;

    // Floor.
    mat4 transform = scale(mat4(1.0f), vec3(200.0f));
    create_plane(world, mat_white, transform);
    // Backgorund.
    transform = translate(mat4(1.0f), vec3(0.0f, 100.0f, -100.0f));
    transform = rotate(transform, radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
    transform = scale(transform, vec3(200.0f));
    create_plane(world, mat_white, transform);
    // Ceilling.
    transform = translate(mat4(1.0f), vec3(0.0f, 200.0f, 0.0f));
    transform = scale(transform, vec3(200.0f));
    transform = rotate(transform, radians(180.0f), vec3(1.0f, 0.0f, 0.0f));
    create_plane(world, mat_white, transform);
    // Light.
    transform = translate(mat4(1.0f), vec3(0.0f, 199.0f, 0.0f));
    transform = scale(transform, vec3(30.0f));
    transform = rotate(transform, radians(180.0f), vec3(1.0f, 0.0f, 0.0f));
    create_plane(world, mat_light, transform);
    // Left.
    transform = translate(mat4(1.0f), vec3(100.0f, 100.0f, 0.0f));
    transform = scale(transform, vec3(200.0f));
    transform = rotate(transform, radians(90.0f), vec3(0.0f, 0.0f, 1.0f));
    create_plane(world, mat_green, transform);
    // Right.
    transform = translate(mat4(1.0f), vec3(-100.0f, 100.0f, 0.0f));
    transform = scale(transform, vec3(200.0f));
    transform = rotate(transform, radians(-90.0f), vec3(0.0f, 0.0f, 1.0f));
    create_plane(world, mat_red, transform);
    // Right Box.
    transform = translate(mat4(1.0f), vec3(-50.0f, 30.0f, 30.0f));
    transform = scale(transform, vec3(60.0f));
    transform = rotate(transform, radians(20.0f), vec3(0.0f, 1.0f, 0.0f));
    create_cube(world, mat_white, transform);
    // Left Box.
    transform = translate(mat4(1.0f), vec3(50.0f, 50.0f, -30.0f));
    transform = scale(transform, vec3(60.0f, 100.0f, 60.0f));
    transform = rotate(transform, radians(-20.0f), vec3(0.0f, 1.0f, 0.0f));
    create_cube(world, mat_white, transform);

    VoxelGridAccelerator accelerator(world);

    QTime render_timer;
    render_timer.start();
    ui->statusBar->showMessage("Rendering progress : ");
    m_statusBarProgress.setVisible(true);

    m_frame_viewer.on_render_begin(width, height);

    m_render.get_render_image(
        width,
        height,
        samples,
        ray_max_depth,
        camera,
        accelerator,
        parallel,
        m_image,
        m_statusBarProgress);

    m_statusBarProgress.setVisible(false);
    QString message = "Rendering time : " + QString::number(render_timer.elapsed()) + " ms";

    ui->statusBar->showMessage(message);
    ui->pushButton_render->setEnabled(true);
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
    int result = run_tests();

    QMessageBox::information(this, "Unit Test", "Unit Test return code : " + QString::number(result));
    QCoreApplication::quit();
}

