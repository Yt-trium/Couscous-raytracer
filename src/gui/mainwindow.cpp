
// Interface.
#include "mainwindow.h"
#include "ui_mainwindow.h"

// couscous includes.
#include "common/logger.h"
#include "gui/scene.h"
#include "gui/dialogmeshfile.h"
#include "renderer/camera.h"
#include "renderer/material.h"
#include "renderer/photonMapping.h"
#include "renderer/gridaccelerator.h"
#include "renderer/ray.h"
#include "renderer/rng.h"
#include "renderer/visualobject.h"
#include "test/test.h"

// Qt includes.
#include <QAction>
#include <QActionGroup>

// glm includes.
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Standard includes.
#include <cstddef>
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
    connect(ui->actionNew_mesh_file, SIGNAL(triggered(bool)), SLOT(slot_create_mesh_file()));
    connect(ui->actionDelete, SIGNAL(triggered(bool)), SLOT(slot_delete()));

    // Makes sure we can't select multiple log level at once.
    auto log_level_action_group = new QActionGroup(this);
    log_level_action_group->addAction(ui->actionLogLevelDebug);
    log_level_action_group->addAction(ui->actionLogLevelInfo);
    log_level_action_group->addAction(ui->actionLogLevelWarning);
    log_level_action_group->addAction(ui->actionLogLevelError);
    log_level_action_group->addAction(ui->actionLogLevelNone);

    // Makes sure we can't select multiple debug view at once.
    auto debug_view_action_group = new QActionGroup(this);
    debug_view_action_group->addAction(ui->actionDisplayNormals);
    debug_view_action_group->addAction(ui->actionDisplayAlbedo);
    debug_view_action_group->addAction(ui->actionDisplayPhotonMap);
    debug_view_action_group->addAction(ui->actionDisplayDirectDiffuse);
    debug_view_action_group->addAction(ui->actionDisplayDirectSpecular);
    debug_view_action_group->addAction(ui->actionDisplayDirectPhong);
    debug_view_action_group->addAction(ui->actionDisplayNone);

    // Map log level events.
    connect(log_level_action_group, SIGNAL(triggered(QAction*)), SLOT(slot_log_level_changed(QAction*)));

    // Presets selection events.
    auto presets_group = new QActionGroup(this);
    presets_group->addAction(ui->actionPresetsCornellBox);
    presets_group->addAction(ui->actionPresetsCornellBoxSuzanne);
    presets_group->addAction(ui->actionPresetsOrangeBlueCornellBox);
    presets_group->addAction(ui->actionPresetsSimpleCube);
    presets_group->addAction(ui->actionPresetsSphere);
    presets_group->addAction(ui->actionPresetsEmpty);

    connect(presets_group, SIGNAL(triggered(QAction*)), SLOT(slot_presets_changed(QAction*)));

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

// Update the scene tree view.
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

    QTreeWidgetItem *widgetItemCamera = new QTreeWidgetItem();
    widgetItemCamera->setText(0, tr("Camera Presets"));
    widgetItemCamera->setIcon(0, QIcon(":/sceneOptions/baseline_photo_camera_black_18dp.png"));

    ui->treeWidget_scene->addTopLevelItem(widgetItemScene);
    widgetItemScene->addChild(widgetItemMaterials);
    widgetItemScene->addChild(widgetItemObjects);

    ui->treeWidget_scene->addTopLevelItem(widgetItemCamera);

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
        widgetItem->setIcon(0, QIcon(":/sceneOptions/file_format_off.png"));
        widgetItemObjects->addChild(widgetItem);
    }

    for(std::size_t i = 0 ; i < scene.cameras.size() ; ++i)
    {
        QTreeWidgetItem *widgetItem = new QTreeWidgetItem();
        widgetItem->setText(0, QString::fromStdString(scene.cameras.at(i).name));
        widgetItem->setIcon(0, QIcon(":/sceneOptions/baseline_videocam_black_18dp.png"));
        widgetItemCamera->addChild(widgetItem);
    }

    ui->treeWidget_scene->expandAll();
}

// Do the render.
void MainWindow::slot_do_render()
{
    ui->pushButton_render->setEnabled(false);

    const size_t width     = size_t(ui->spinBox_width->value());
    const size_t height    = size_t(ui->spinBox_height->value());
    const size_t samples   = size_t(ui->spinBox_spp->value());
    const size_t direct_light_rays_count = size_t(ui->spinBox_dlrc->value());
    const float  pos_x     = float(ui->doubleSpinBox_position_x->value());
    const float  pos_y     = float(ui->doubleSpinBox_position_y->value());
    const float  pos_z     = float(ui->doubleSpinBox_position_z->value());
    const float  yaw       = float(ui->doubleSpinBox_yaw->value());
    const float  pitch     = float(ui->doubleSpinBox_pitch->value());
    const float  fov       = float(ui->doubleSpinBox_fov->value());
    const bool   parallel  = ui->checkBox_parallel_rendering->isChecked();

    m_image = QImage(int(width), int(height), QImage::Format_RGB888);

    // Create the camera.
    Camera camera(vec3(pos_x, pos_y, pos_z), vec3(0.0f, 1.0f, 0.0f),
        yaw, pitch, fov, width, height);

    // Create the scene.
    Logger::log_info("creating the scene...");
    MeshGroup world;
    scene.create_scene(world);

    if (world.empty())
    {
        Logger::log_warning("nothing to render.");
        ui->pushButton_render->setEnabled(true);
        return;
    }

    m_frame_viewer.on_render_begin(width, height);

    m_render.get_render_image(
        width,
        height,
        samples,
        camera,
        world,
        direct_light_rays_count,
        parallel,
        ui->actionDisplayNormals->isChecked(),
        ui->actionDisplayAlbedo->isChecked(),
        ui->actionDisplayPhotonMap->isChecked(),
        ui->actionDisplayDirectDiffuse->isChecked(),
        ui->actionDisplayDirectSpecular->isChecked(),
        ui->actionDisplayDirectPhong->isChecked(),
        m_image,
        m_statusBarProgress);

    ui->pushButton_render->setEnabled(true);
}

// Save the last rendered image.
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

// Zoom in the viewport.
void MainWindow::slot_zoom_in()
{
    m_frame_viewer.zoom_viewport(0.1f);
}

// Zoom out the viewport.
void MainWindow::slot_zoom_out()
{
    m_frame_viewer.zoom_viewport(-0.1f);
}

// Run unit tests and quit the application.
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
    menu.addAction(ui->actionNew_mesh_file);
    menu.addAction(ui->actionDelete);

    menu.exec(ui->treeWidget_scene->mapToGlobal(p));
}

// Edit a scene element.
void MainWindow::slot_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    const QString itmname = item->text(column);

    for(size_t i = 0; i < scene.materials.size(); ++i)
    {
        if(QString::fromStdString(scene.materials.at(i).name) == itmname)
        {
            DialogMaterial* dlg = new DialogMaterial(this, scene, i);
            dlg->exec();
            update_scene_widget();

            return;
        }
    }

    for(size_t i = 0; i < scene.objects.size(); ++i)
    {
        if(QString::fromStdString(scene.objects.at(i).name) == itmname)
        {
            DialogObject* dlg = new DialogObject(this, &scene, int(i));
            dlg->exec();
            update_scene_widget();

            return;
        }
    }

    for(size_t i = 0; i < scene.object_files.size(); ++i)
    {
        if(QString::fromStdString(scene.object_files.at(i).name) == itmname)
        {
            DialogMeshFile* dlg = new DialogMeshFile(this, scene, scene.object_files.at(i));
            dlg->exec();
            update_scene_widget();

            return;
        }
    }

    for(size_t i = 0; i < scene.cameras.size(); ++i)
    {
        if(QString::fromStdString(scene.cameras.at(i).name) == itmname)
        {
            const SceneCamera& cam = scene.cameras.at(i);

            ui->doubleSpinBox_position_x->setValue(double(cam.position.x));
            ui->doubleSpinBox_position_y->setValue(double(cam.position.y));
            ui->doubleSpinBox_position_z->setValue(double(cam.position.z));

            ui->doubleSpinBox_yaw->setValue(double(cam.yaw));
            ui->doubleSpinBox_pitch->setValue(double(cam.pitch));
            ui->doubleSpinBox_fov->setValue(double(cam.fov));

            ui->spinBox_height->setValue(int(cam.height));
            ui->spinBox_width->setValue(int(cam.width));

            return;
        }
    }
}

// Create a material.
void MainWindow::slot_create_material()
{
    DialogMaterial *dlg = new DialogMaterial(this, scene, size_t(~0));
    dlg->exec();
    update_scene_widget();
}

// Create a primitive.
void MainWindow::slot_create_object()
{
    DialogObject *dlg = new DialogObject(this, &scene, -1);
    dlg->exec();
    update_scene_widget();
}

// Create a mesh file.
void MainWindow::slot_create_mesh_file()
{
    SceneMeshFile file("", "", Transform(), "");
    DialogMeshFile *dlg = new DialogMeshFile(this, scene, file);
    dlg->exec();

    // Don't add it if it's empty.
    if (file.name != "" || file.path != "")
    {
        scene.object_files.push_back(file);
    }

    update_scene_widget();
}

// Delete an item from the scene.
void MainWindow::slot_delete()
{
    QList<QTreeWidgetItem *> selection = ui->treeWidget_scene->selectedItems();
    QList<std::size_t> selection_materials;
    QList<std::size_t> selection_objects;
    QList<std::size_t> selection_object_files;

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

        for(std::size_t i = 0 ; i < scene.object_files.size() ; ++i)
        {
            if(QString::fromStdString(scene.object_files.at(i).name) == name)
            {
                selection_object_files.push_front(i);
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

    for(int x = 0 ; x < selection_object_files.size() ; ++x)
    {
        scene.object_files.erase(scene.object_files.begin() + selection_object_files.at(x));
    }

    update_scene_widget();
}

// Change the logging level.
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
    else if (action == ui->actionLogLevelWarning)
    {
        Logger::set_level(LogLevel::Warning);
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

// Load a new scene.
void MainWindow::slot_presets_changed(QAction *action)
{
    if(action == ui->actionPresetsCornellBox)
    {
        scene = Scene::cornell_box();
    }
    else if(action == ui->actionPresetsCornellBoxSuzanne)
    {
        scene = Scene::cornell_box_suzanne();
    }
    else if(action == ui->actionPresetsOrangeBlueCornellBox)
    {
        scene = Scene::cornell_box_orange_and_blue();
    }
    else if(action == ui->actionPresetsSimpleCube)
    {
        scene = Scene::simple_cube();
    }
    else if(action == ui->actionPresetsSphere)
    {
        scene = Scene::sphere();
    }
    else if(action == ui->actionPresetsEmpty)
    {
        scene = Scene();
    }

    if(scene.cameras.size() > 0)
    {
        SceneCamera cam = scene.cameras.at(0);
        ui->doubleSpinBox_position_x->setValue(double(cam.position.x));
        ui->doubleSpinBox_position_y->setValue(double(cam.position.y));
        ui->doubleSpinBox_position_z->setValue(double(cam.position.z));

        ui->doubleSpinBox_yaw->setValue(double(cam.yaw));
        ui->doubleSpinBox_pitch->setValue(double(cam.pitch));
        ui->doubleSpinBox_fov->setValue(double(cam.fov));

        ui->spinBox_height->setValue(int(cam.height));
        ui->spinBox_width->setValue(int(cam.width));
    }

    Logger::log_info("loaded a new preset.");

    update_scene_widget();
}

// Quit the application.
void MainWindow::on_actionQuit_triggered()
{
    QCoreApplication::quit();
}

