
#ifndef GUI_DIALOGMATERIAL_H
#define GUI_DIALOGMATERIAL_H

// couscous includes.
#include "gui/scene.h"

// Qt includes.
#include <QDialog>

// glm includes.
#include <glm/glm.hpp>

// Standard includes.
#include <cstddef>

// Forward declarations.
namespace Ui { class DialogMaterial; }

// Graphic interface for editing a material.
class DialogMaterial
  : public QDialog
{
    Q_OBJECT

  public:
    DialogMaterial(QWidget* parent, Scene& scene, const size_t id);
    ~DialogMaterial();

  private slots:
    void on_buttonBox_accepted();

  private:
    Scene&                  m_scene;
    size_t                  m_id;
    Ui::DialogMaterial*     m_ui;
};

#endif // GUI_DIALOGMATERIAL_H
