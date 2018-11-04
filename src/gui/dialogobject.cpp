#include "dialogobject.h"
#include "ui_dialogobject.h"

using namespace glm;
using namespace std;

DialogObject::DialogObject(QWidget *p, Scene *s, int i) :
    QDialog(p),
    ui(new Ui::DialogObject)
{
    scene = s;
    id = i;
    parent = p;
    ui->setupUi(this);

    if(id >= 0)
    {
        const auto& object = s->objects.at(static_cast<size_t>(id));

        const QString name = QString::fromStdString(object.name);
        const Transform& transform = object.transform;
        const ObjectType type = object.type;
        const QString material = QString::fromStdString(object.material);

        const size_t subdivisions = object.subdivisions;
        const float width = object.width;
        const float height = object.height;
        const bool caps = object.caps;

        ui->lineEdit_name->setText(name);
        ui->doubleSpinBox_translate_x->setValue(double(transform.translate[0]));
        ui->doubleSpinBox_translate_y->setValue(double(transform.translate[1]));
        ui->doubleSpinBox_translate_z->setValue(double(transform.translate[2]));
        ui->doubleSpinBox_rotate_x->setValue(double(transform.rotation[0]));
        ui->doubleSpinBox_rotate_y->setValue(double(transform.rotation[1]));
        ui->doubleSpinBox_rotate_z->setValue(double(transform.rotation[2]));
        ui->doubleSpinBox_scale_x->setValue(double(transform.scale[0]));
        ui->doubleSpinBox_scale_y->setValue(double(transform.scale[1]));
        ui->doubleSpinBox_scale_z->setValue(double(transform.scale[2]));

        switch(type)
        {
            case ObjectType::PLANE:
                ui->comboBox_object_type->setCurrentIndex(0);
                break;
            case ObjectType::CUBE:
                ui->comboBox_object_type->setCurrentIndex(1);
                break;
            case ObjectType::CYLINDER:
                ui->comboBox_object_type->setCurrentIndex(2);
                ui->spinBox_subdivisions->setEnabled(true);
                ui->doubleSpinBox_height->setEnabled(true);
                ui->doubleSpinBox_width->setEnabled(true);
                ui->checkBox_caps->setEnabled(true);
                break;
        }

        for(size_t x = 0; x < scene->materials.size(); ++x)
        {
            ui->comboBox_material->addItem(QString::fromStdString(scene->materials.at(x).name));
            if(QString::fromStdString(scene->materials.at(x).name) == material)
                ui->comboBox_material->setCurrentIndex(int(x));
        }

        ui->spinBox_subdivisions->setValue(int(subdivisions));
        ui->doubleSpinBox_height->setValue(double(height));
        ui->doubleSpinBox_width->setValue(double(width));
        ui->checkBox_caps->setChecked(caps);
    }
    else
    {
        for(size_t x = 0; x < scene->materials.size(); ++x)
        {
            ui->comboBox_material->addItem(QString::fromStdString(scene->materials.at(x).name));
        }
    }
}

DialogObject::~DialogObject()
{
    delete ui;
}

void DialogObject::on_buttonBox_accepted()
{
    ObjectType ot;

    switch (ui->comboBox_object_type->currentIndex()) {
      default:
      case 0:
        ot = ObjectType::PLANE;
        break;
      case 1:
        ot = ObjectType::CUBE;
        break;
      case 2:
        ot = ObjectType::CYLINDER;
        break;
    }

    const Transform transform(
        vec3(ui->doubleSpinBox_translate_x->value(),
            ui->doubleSpinBox_translate_y->value(),
            ui->doubleSpinBox_translate_z->value()),
        vec3(ui->doubleSpinBox_rotate_x->value(),
            ui->doubleSpinBox_rotate_y->value(),
            ui->doubleSpinBox_rotate_z->value()),
        vec3(ui->doubleSpinBox_scale_x->value(),
            ui->doubleSpinBox_scale_y->value(),
            ui->doubleSpinBox_scale_z->value()));

    SceneObject so(
        ui->lineEdit_name->text().toStdString(),
        transform,
        ot,
        ui->comboBox_material->currentText().toStdString());

    if (ot == ObjectType::CYLINDER)
    {
        so.subdivisions = std::size_t(ui->spinBox_subdivisions->value());
        so.width = float(ui->doubleSpinBox_width->value());
        so.height = float(ui->doubleSpinBox_width->value());
        so.caps = ui->checkBox_caps->isChecked();
    }

    if(id >= 0)
        scene->objects[std::size_t(id)] = so;
    else
        scene->objects.push_back(so);
}

void DialogObject::on_comboBox_object_type_currentIndexChanged(int index)
{
    ObjectType ot;

    switch (ui->comboBox_object_type->currentIndex())
    {
      default:
      case 0:
        ot = ObjectType::PLANE;
        break;
      case 1:
        ot = ObjectType::CUBE;
        break;
      case 2:
        ot = ObjectType::CYLINDER;
        break;
    }

    switch(ot)
    {
      case ObjectType::PLANE:
        ui->comboBox_object_type->setCurrentIndex(0);
        ui->spinBox_subdivisions->setEnabled(false);
        ui->doubleSpinBox_height->setEnabled(false);
        ui->doubleSpinBox_width->setEnabled(false);
        ui->checkBox_caps->setEnabled(false);
        break;
      case ObjectType::CUBE:
        ui->comboBox_object_type->setCurrentIndex(1);
        ui->spinBox_subdivisions->setEnabled(false);
        ui->doubleSpinBox_height->setEnabled(false);
        ui->doubleSpinBox_width->setEnabled(false);
        ui->checkBox_caps->setEnabled(false);
        break;
      case ObjectType::CYLINDER:
        ui->comboBox_object_type->setCurrentIndex(2);
        ui->spinBox_subdivisions->setEnabled(true);
        ui->doubleSpinBox_height->setEnabled(true);
        ui->doubleSpinBox_width->setEnabled(true);
        ui->checkBox_caps->setEnabled(true);
        break;
    }
}

