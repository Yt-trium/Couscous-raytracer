#ifndef GUI_DIALOGMESHFILE_H
#define GUI_DIALOGMESHFILE_H

// couscous includes.
#include "gui/scene.h"

// Qt includes.
#include <QDialog>

// glm includes.
#include <glm/glm.hpp>

// Forward declarations.
namespace Ui { class DialogMeshFile;}

class DialogMeshFile : public QDialog
{
    Q_OBJECT

  public:
    DialogMeshFile(QWidget* parent, Scene& scene, SceneMeshFile& scene_file);
    ~DialogMeshFile();

  private slots:
    void on_buttonBox_accepted();

  private:
    Ui::DialogMeshFile* m_ui;
    SceneMeshFile&      m_scene_file;
};

#endif // GUI_DIALOGMESHFILE_H
