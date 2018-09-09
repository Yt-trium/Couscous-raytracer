#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <glm/vec3.hpp>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QGraphicsScene *scene = new QGraphicsScene(ui->graphicsView);
    ui->graphicsView->setScene(scene);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_render_pushButton_clicked()
{
    int size_x = 200;
    int size_y = 100;

    QImage image(size_x, size_y, QImage::Format_RGB32);
    QPixmap pixmap;

    int x, y;
    for(x=0;x<size_x;x++)
    {
        for(y=0;y<size_y;y++)
        {
            glm::vec3 color(double(y) / double(size_y), double(x) / double(size_x), 0.2);

            int ir = int(255.99*color[0]);
            int ig = int(255.99*color[1]);
            int ib = int(255.99*color[2]);

            image.setPixelColor(x, y, QColor(ir, ig, ib));
        }
    }

    pixmap.convertFromImage(image);
    ui->graphicsView->scene()->clear();
    ui->graphicsView->scene()->addPixmap(pixmap);
}
