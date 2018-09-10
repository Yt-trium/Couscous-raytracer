
// Interface.
#include "mainwindow.h"
#include "ui_mainwindow.h"

// couscous includes.
#include "render.h"
#include "ray.h"

// math includes.
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
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slot_do_render()
{
    int size_x = 200;
    int size_y = 100;

    QImage image(size_x, size_y, QImage::Format_RGB32);
    QPixmap pixmap;

    int x, y;

    Render render;

    vec3 lower_left_corner(-2.0, -1.0, -1.0);
    vec3 horizontal(4.0, 0.0, 0.0);
    vec3 vertical(0.0, 2.0, 0.0);
    vec3 origin(0.0, 0.0, 0.0);

    VisualObjectList world;
    world.objectList.push_back(new UsualShapes::Sphere(vec3(0,0,-1), 0.5));
    world.objectList.push_back(new UsualShapes::Sphere(vec3(0,-100.5,-1), 100));

    for(x=0;x<size_x;x++)
    {
        for(y=0;y<size_y;y++)
        {
            float u = float(x) / float(size_x);
            float v = float(y) / float(size_y);

            Ray r(origin, lower_left_corner + u*horizontal + v*vertical);
            vec3 color = render.getRayColor(r, world);

            int ir = int(255.0f * color[0]);
            int ig = int(255.0f * color[1]);
            int ib = int(255.0f * color[2]);
            image.setPixel(size_x-1-x, size_y-1-y, QColor(ir, ig, ib).rgb());
        }
    }

    pixmap.convertFromImage(image);
    ui->graphicsView->scene()->clear();
    ui->graphicsView->scene()->addPixmap(pixmap);
}

