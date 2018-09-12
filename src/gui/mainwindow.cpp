
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
{
    ui->setupUi(this);

    QGraphicsScene *scene = new QGraphicsScene(ui->graphicsView);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->scale(4,4);

    // Connect widgets events.
    connect(ui->do_render, SIGNAL(released()), SLOT(slot_do_render()));
    connect(ui->actionSave_As_Image, SIGNAL(triggered()), SLOT(slot_save_as_image()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slot_do_render()
{
    size_t width = 200;
    size_t height = 100;
    size_t samples = 100;

    Render render;
    Camera camera(vec3(0.0f), vec3(0.0f, 1.0f, 0.0f),
                  -90.0f, 0.0f, 85.0f, width, height);
    QPixmap pixmap;

    VisualObjectList world;
    world.object_list.push_back(new Sphere(vec3(0,0,-1), 0.5));
    world.object_list.push_back(new Sphere(vec3(0,-100.5,-1), 100));

    pixmap.convertFromImage(render.get_render_image(width, height, samples, camera, world));
    ui->graphicsView->scene()->clear();
    ui->graphicsView->scene()->addPixmap(pixmap);
}

void MainWindow::slot_save_as_image()
{

    size_t width = 200;
    size_t height = 100;
    size_t samples = 100;

    Render render;
    Camera camera(vec3(0.0f), vec3(0.0f, 1.0f, 0.0f),
                  -90.0f, 0.0f, 85.0f, width, height);
    QPixmap pixmap;

    VisualObjectList world;
    world.object_list.push_back(new Sphere(vec3(0,0,-1), 0.5));
    world.object_list.push_back(new Sphere(vec3(0,-100.5,-1), 100));

    QImage image = render.get_render_image(width, height, samples, camera, world);

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

    // Future code for more advanced options (maybe ask for quality).
    // QFileDialog pathDialog(this);
    // pathDialog.setFileMode(QFileDialog::AnyFile);
    // pathDialog.setViewMode(QFileDialog::Detail);
    // pathDialog.setNameFilter(tr("Images (*.png *.jpg *.bmp)"));
}

