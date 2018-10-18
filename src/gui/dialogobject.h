#ifndef DIALOGOBJECT_H
#define DIALOGOBJECT_H

// couscous includes.
#include "gui/scene.h"

// Qt includes.
#include <QDialog>

// glm includes.
#include <glm/glm.hpp>

namespace Ui {
class DialogObject;
}

class DialogObject : public QDialog
{
    Q_OBJECT

public:
    explicit DialogObject(QWidget *p, Scene *s, int i);
    ~DialogObject();

private slots:
    void on_buttonBox_accepted();

    void on_comboBox_object_type_currentIndexChanged(int index);

private:
    QWidget *parent;
    Scene *scene;
    int id;
    Ui::DialogObject *ui;
};

#endif // DIALOGOBJECT_H
