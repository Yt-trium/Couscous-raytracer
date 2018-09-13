#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// couscous headers.
#include "gui/frameviewer.h"

// Qt headers.
#include <QFileDialog>
#include <QImage>
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

  signals:
    void signal_rendering_finished(const QImage&) const;

  private:
    Ui::MainWindow* ui;
    QImage          m_image;
    FrameViewer     m_frame_viewer;

  private slots:
    void slot_do_render();
    void slot_save_as_image();
    void slot_zoom_in();
    void slot_zoom_out();
};

#endif // MAINWINDOW_H

