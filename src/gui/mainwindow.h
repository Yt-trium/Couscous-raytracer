#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// coucous includes.
#include "usualshapes.h"
#include "visualobjectlist.h"

// Qt includes.
#include <QMainWindow>

// Forward declarations.
namespace Ui { class MainWindow; }

// Main window
class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

  private:
    Ui::MainWindow* ui;

  private slots:
    void slot_do_render();
    void slot_save_as_image();

};

#endif // MAINWINDOW_H

