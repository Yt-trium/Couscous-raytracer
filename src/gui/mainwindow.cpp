
// Interface.
#include "mainwindow.h"
#include "ui_mainwindow.h"

// couscous includes.
#include "common/logger.h"
#include "gui/scene.h"
#include "io/scenefilereader.h"
#include "renderer/camera.h"
#include "renderer/material.h"
#include "renderer/photonMapping.h"
#include "renderer/gridaccelerator.h"
#include "renderer/ray.h"
#include "renderer/visualobject.h"
#include "test/test.h"

// Qt includes.
#include <QAction>
#include <QActionGroup>

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

    connect(ui->dockWidget_render, SIGNAL(visibilityChanged(bool)), SLOT(slot_dock_widget_changed()));
    connect(ui->dockWidget_scene, SIGNAL(visibilityChanged(bool)), SLOT(slot_dock_widget_changed()));
    connect(ui->actionRender_Options, SIGNAL(triggered(bool)), SLOT(slot_action_dock_changed()));
    connect(ui->actionScene_Options, SIGNAL(triggered(bool)), SLOT(slot_action_dock_changed()));
    connect(ui->treeWidget_scene,
            SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
            SLOT(slot_treeWidget_itemDoubleClicked(QTreeWidgetItem*, int)));

    connect(
        &m_render,
        SIGNAL(on_tile_end(size_t, size_t, size_t, size_t, QImage)),
        &m_frame_viewer,
        SLOT(update_tile(size_t, size_t, size_t, size_t, QImage)));

    connect(ui->treeWidget_scene, SIGNAL(customContextMenuRequested(const QPoint&)),
            SLOT(slot_treeWidget_customContextMenuRequested(const QPoint&)));

    connect(ui->actionNew_material, SIGNAL(triggered(bool)), SLOT(slot_create_material()));
    connect(ui->actionNew_object, SIGNAL(triggered(bool)), SLOT(slot_create_object()));
    connect(ui->actionDelete, SIGNAL(triggered(bool)), SLOT(slot_delete()));

    // Makes sure we can't select multiple log level at once.
    auto log_level_action_group = new QActionGroup(this);
    log_level_action_group->addAction(ui->actionLogLevelDebug);
    log_level_action_group->addAction(ui->actionLogLevelInfo);
    log_level_action_group->addAction(ui->actionLogLevelError);
    log_level_action_group->addAction(ui->actionLogLevelNone);

    // Map log level events.
    connect(log_level_action_group, SIGNAL(triggered(QAction*)), SLOT(slot_log_level_changed(QAction*)));

    // Create a default scene.
    scene = Scene::cornell_box();

    // Hook up the gui status bar to the logger.
    Logger::set_gui_bar(ui->statusBar);

    update_scene_widget();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::update_scene_widget()
{
    ui->treeWidget_scene->clear();

    QTreeWidgetItem *widgetItemScene = new QTreeWidgetItem();
    widgetItemScene->setText(0, tr("Scene"));
    widgetItemScene->setIcon(0, QIcon(":/sceneOptions/baseline_landscape_black_18dp.png"));

    QTreeWidgetItem *widgetItemMaterials = new QTreeWidgetItem();
    widgetItemMaterials->setText(0, tr("Materials"));
    widgetItemMaterials->setIcon(0, QIcon(":/sceneOptions/baseline_gradient_black_18dp.png"));

    QTreeWidgetItem *widgetItemObjects = new QTreeWidgetItem();
    widgetItemObjects->setText(0, tr("Objects"));
    widgetItemObjects->setIcon(0, QIcon(":/sceneOptions/baseline_business_black_18dp.png"));

    ui->treeWidget_scene->addTopLevelItem(widgetItemScene);
    widgetItemScene->addChild(widgetItemMaterials);
    widgetItemScene->addChild(widgetItemObjects);

    for(std::size_t i = 0 ; i < scene.materials.size() ; ++i)
    {
        QTreeWidgetItem *widgetItem = new QTreeWidgetItem();
        widgetItem->setText(0, QString::fromStdString(scene.materials.at(i).name));
        widgetItem->setIcon(0, QIcon(":/sceneOptions/baseline_texture_black_18dp.png"));
        widgetItemMaterials->addChild(widgetItem);
    }

    for(std::size_t i = 0 ; i < scene.objects.size() ; ++i)
    {
        QTreeWidgetItem *widgetItem = new QTreeWidgetItem();
        widgetItem->setText(0, QString::fromStdString(scene.objects.at(i).name));

        switch(scene.objects.at(i).type)
        {
          case ObjectType::PLANE:
            widgetItem->setIcon(0, QIcon(":/sceneOptions/square-outline.png"));
            break;
          case ObjectType::CUBE:
            widgetItem->setIcon(0, QIcon(":/sceneOptions/cube-outline.png"));
            break;
          case ObjectType::CYLINDER:
            widgetItem->setIcon(0, QIcon(":/sceneOptions/cylinder-outline.png"));
            break;
        }

        widgetItemObjects->addChild(widgetItem);
    }

    for(std::size_t i = 0 ; i < scene.object_files.size() ; ++i)
    {
        QTreeWidgetItem *widgetItem = new QTreeWidgetItem();
        widgetItem->setText(0, QString::fromStdString(scene.object_files.at(i).name));

        switch(scene.object_files.at(i).type)
        {
          case ObjectFileType::OBJ:
            widgetItem->setIcon(0, QIcon(":/sceneOptions/file_format_obj.png"));
            break;
          case ObjectFileType::OFF:
            widgetItem->setIcon(0, QIcon(":/sceneOptions/file_format_off.png"));
            break;
        }

        widgetItemObjects->addChild(widgetItem);
    }

    ui->treeWidget_scene->expandAll();
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

    // Create cornell box.
    MeshGroup world;

    // Get visual.
    Logger::log_info("creating the scene...");
    scene.create_scene(world);

    // Get lights from the scene.
    const MeshGroup lights = fetch_lights(world);
    Logger::log_debug(to_string(lights.size()) + " light triangles");
    Logger::log_debug(to_string(world.size() - lights.size()) + " triangles");

    // Create the grid accelerator.
    VoxelGridAccelerator accelerator(world);

    // Create photon map.
    PhotonMap pmap;
    pmap.compute_map(1000, 8, accelerator, world, lights);

    QTime render_timer;
    Logger::log_info("rendering...");
    render_timer.start();
    m_statusBarProgress.setVisible(true);

    m_frame_viewer.on_render_begin(width, height);

    m_render.get_render_image(
        width,
        height,
        samples,
        ray_max_depth,
        camera,
        accelerator,
        pmap,
        parallel,
        ui->actionDisplay_Normals->isChecked(),
        ui->actionDisplayPhotonMap->isChecked(),
        m_image,
        m_statusBarProgress);

    m_statusBarProgress.setVisible(false);

    const auto& elapsed = render_timer.elapsed();

    QString message =
        QString("rendering finished in ")
        + ((elapsed > 1000.0f)
            ? (QString::number(render_timer.elapsed() / 1000.0f) + "s.")
            : (QString::number(render_timer.elapsed()%1000) + "ms."));

    Logger::log_info(message.toStdString().c_str());

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

void MainWindow::slot_dock_widget_changed()
{
    ui->actionRender_Options->setChecked(ui->dockWidget_render->isVisible());
    ui->actionScene_Options->setChecked(ui->dockWidget_scene->isVisible());
}

void MainWindow::slot_action_dock_changed()
{
    ui->dockWidget_render->setVisible(ui->actionRender_Options->isChecked());
    ui->dockWidget_scene->setVisible(ui->actionScene_Options->isChecked());
}

void MainWindow::slot_update_scene_tree_widget()
{
    QTreeWidgetItem *itm = new QTreeWidgetItem();
    ui->treeWidget_scene->insertTopLevelItem(0, itm);
}

void MainWindow::slot_treeWidget_customContextMenuRequested(const QPoint &p)
{
    QMenu menu(this);

    menu.addAction(ui->actionNew_material);
    menu.addAction(ui->actionNew_object);
    menu.addAction(ui->actionDelete);

    menu.exec(ui->treeWidget_scene->mapToGlobal(p));
}

void MainWindow::slot_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    QString itmname = item->text(column);
    for(std::size_t i = 0 ; i < scene.materials.size() ; ++i)
    {
        if(QString::fromStdString(scene.materials.at(i).name) == itmname)
        {
            DialogMaterial *dlg = new DialogMaterial(this, &scene, int(i));
            dlg->exec();
            update_scene_widget();
            return;
        }
    }
    for(std::size_t i = 0 ; i < scene.objects.size() ; ++i)
    {
        if(QString::fromStdString(scene.objects.at(i).name) == itmname)
        {
            DialogObject *dlg = new DialogObject(this, &scene, int(i));
            dlg->exec();
            update_scene_widget();
            return;
        }
    }

}

void MainWindow::slot_create_material()
{
    DialogMaterial *dlg = new DialogMaterial(this, &scene, -1);
    dlg->exec();
    update_scene_widget();
}

void MainWindow::slot_create_object()
{
    DialogObject *dlg = new DialogObject(this, &scene, -1);
    dlg->exec();
    update_scene_widget();
}

void MainWindow::slot_delete()
{
    QList<QTreeWidgetItem *> selection = ui->treeWidget_scene->selectedItems();
    QList<std::size_t> selection_materials;
    QList<std::size_t> selection_objects;

    for(int x = 0 ; x < selection.size() ; ++x)
    {
        QString name = selection.at(x)->text(0);

        for(std::size_t i = 0 ; i < scene.materials.size() ; ++i)
        {
            if(QString::fromStdString(scene.materials.at(i).name) == name)
            {
                selection_materials.push_front(i);
            }
        }
        for(std::size_t i = 0 ; i < scene.objects.size() ; ++i)
        {
            if(QString::fromStdString(scene.objects.at(i).name) == name)
            {
                selection_objects.push_front(i);
            }
        }
    }

    for(int x = 0 ; x < selection_materials.size() ; ++x)
    {
        scene.materials.erase(scene.materials.begin() + selection_materials.at(x));
    }

    for(int x = 0 ; x < selection_objects.size() ; ++x)
    {
        scene.objects.erase(scene.objects.begin() + selection_objects.at(x));
    }

    update_scene_widget();
}

void MainWindow::slot_log_level_changed(QAction* action)
{
    if (action == ui->actionLogLevelDebug)
    {
        Logger::set_level(LogLevel::Debug);
    }
    else if (action == ui->actionLogLevelInfo)
    {
        Logger::set_level(LogLevel::Info);
    }
    else if (action == ui->actionLogLevelError)
    {
        Logger::set_level(LogLevel::Error);
    }
    else if (action == ui->actionLogLevelNone)
    {
        Logger::set_level(LogLevel::None);
    }
}

