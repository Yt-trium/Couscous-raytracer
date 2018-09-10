#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "usualshapes.h"
#include "visualobjectlist.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_render_pushButton_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
