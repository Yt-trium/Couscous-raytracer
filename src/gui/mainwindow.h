#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// couscous headers.
#include "gui/frameviewer.h"
#include "renderer/render.h"

// Qt headers.
#include <QFileDialog>
#include <QTime>
#include <QImage>
#include <QMainWindow>
#include <QMessageBox>

// Forward declarations.
namespace Ui { class MainWindow; }

// Main window
class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

  signals:
    void signal_rendering_finished(const QImage&) const;

  private:
    Ui::MainWindow* ui;
    QImage          m_image;
    FrameViewer     m_frame_viewer;
    Render          *m_render;

  private slots:
    void slot_do_render();
    void slot_save_as_image();
    void slot_zoom_in();
    void slot_zoom_out();
    void slot_run_unit_test();
    void slot_render_new_tile();
};

#endif // MAINWINDOW_H

