#ifndef DIALOGMATERIAL_H
#define DIALOGMATERIAL_H

// couscous includes.
#include "gui/scene.h"

// Qt includes.
#include <QDialog>

// glm includes.
#include <glm/glm.hpp>

namespace Ui {
class DialogMaterial;
}

class DialogMaterial : public QDialog
{
    Q_OBJECT

public:
    explicit DialogMaterial(QWidget *p, Scene *s, std::size_t i);
    ~DialogMaterial();

private slots:
    void on_buttonBox_accepted();

private:
    QWidget *parent;
    Scene *scene;
    std::size_t id;
    Ui::DialogMaterial *ui;
};

#endif // DIALOGMATERIAL_H
