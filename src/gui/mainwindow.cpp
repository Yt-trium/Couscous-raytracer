
// Interface.
#include "mainwindow.h"
#include "ui_mainwindow.h"

// couscous includes.
#include "renderer/camera.h"
#include "renderer/render.h"
#include "renderer/ray.h"
#include "renderer/visualobject.h"

// glm includes.
#include <glm/glm.hpp>

using namespace glm;

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , m_frame_viewer(200, 100)
{
    ui->setupUi(this);

    // Add the image viewer.
    ui->viewer_container_layout->addWidget(&m_frame_viewer);

    // Connect widgets events.
    connect(ui->pushButton_render, SIGNAL(released()), SLOT(slot_do_render()));
    connect(ui->actionSave_As_Image, SIGNAL(triggered()), SLOT(slot_save_as_image()));
    connect(ui->pushButton_zoom_in, SIGNAL(released()), SLOT(slot_zoom_in()));
    connect(ui->pushButton_zoom_out, SIGNAL(released()), SLOT(slot_zoom_out()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slot_do_render()
{
    size_t width = size_t(ui->spinBox_width->value());
    size_t height = size_t(ui->spinBox_height->value());
    size_t samples = size_t(ui->spinBox_spp->value());
    QImage image(int(width), int(height), QImage::Format_RGB888);

    m_frame_viewer.on_render_begin();

    Render render;
    Camera camera(vec3(0.0f), vec3(0.0f, 1.0f, 0.0f),
                  -90.0f, 0.0f, 85.0f, width, height);

    VisualObjectList world;
    world.object_list.push_back(new Sphere(vec3(0,0,-1), 0.5));
    world.object_list.push_back(new Sphere(vec3(0,-100.5,-1), 100));

    world.object_list.push_back(new Triangle(
        vec3(-0.7f, 0.0f, -1.0f),
        vec3(-0.7f, 0.4f, -1.0f),
        vec3(-1.3f, 0.2f, -1.0f)));
    world.object_list.push_back(new Sphere(vec3(-0.7f, 0.0f, -1.0f), 0.05f));
    world.object_list.push_back(new Sphere(vec3(-0.7f, 0.4f, -1.0f), 0.05f));
    world.object_list.push_back(new Sphere(vec3(-1.3f, 0.2f, -1.0f), 0.05f));

    render.get_render_image(width, height, samples, camera, world, image);

    m_frame_viewer.on_render_end(image);
}

void MainWindow::slot_save_as_image()
{
    size_t width = size_t(ui->spinBox_width->value());
    size_t height = size_t(ui->spinBox_height->value());
    size_t samples = size_t(ui->spinBox_spp->value());
    QImage image(int(width), int(height), QImage::Format_RGB32);

    Render render;
    Camera camera(vec3(0.0f), vec3(0.0f, 1.0f, 0.0f),
                  -90.0f, 0.0f, 85.0f, width, height);
    VisualObjectList world;
    world.object_list.push_back(new Sphere(vec3(0,0,-1), 0.5));
    world.object_list.push_back(new Sphere(vec3(0,-100.5,-1), 100));

    render.get_render_image(width, height, samples, camera, world, image);

    // Create file filters.
    QString selected_filter =  "PNG (*.png)";

    QStringList filter_list;
    filter_list << selected_filter;
    filter_list << "JPG (*.jpg)";
    filter_list << "JPEG (*.jpeg)";
    filter_list << "PBM (*.pbm)";
    filter_list << "All Files (*.*)";

    QString path = QFileDialog::getSaveFileName(
        this,
        tr("Save Image"),
        QDir::currentPath(),
        filter_list.join(";;"),
        &selected_filter);

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

    image.save(path);
}

void MainWindow::slot_zoom_in()
{
    m_frame_viewer.zoom_viewport(2.0f);
}

void MainWindow::slot_zoom_out()
{
    m_frame_viewer.zoom_viewport(-2.0f);
}

