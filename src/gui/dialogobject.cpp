#include "dialogobject.h"
#include "ui_dialogobject.h"

using namespace glm;

DialogObject::DialogObject(QWidget *p, Scene *s, int i) :
    QDialog(p),
    ui(new Ui::DialogObject)
{
    scene = s;
    id = i;
    parent = p;
    ui->setupUi(this);

    QString name = QString::fromStdString(s->objects.at(std::size_t(id)).m_name);
    vec3 translate(s->objects.at(std::size_t(id)).m_translate);
    vec3 rotate(s->objects.at(std::size_t(id)).m_rotate);
    float rotate_d = s->objects.at(std::size_t(id)).m_rotate_d;
    vec3 scale(s->objects.at(std::size_t(id)).m_scale);
    object_type type = s->objects.at(std::size_t(id)).m_type;
    QString material = QString::fromStdString(s->objects.at(std::size_t(id)).m_material);
    std::size_t subdivisions = s->objects.at(std::size_t(id)).m_subdivisions;
    float height = s->objects.at(std::size_t(id)).m_height;
    float width = s->objects.at(std::size_t(id)).m_width;
    bool caps = s->objects.at(std::size_t(id)).m_caps;

    ui->lineEdit_name->setText(name);
    ui->doubleSpinBox_translate_x->setValue(double(translate[0]));
    ui->doubleSpinBox_translate_y->setValue(double(translate[1]));
    ui->doubleSpinBox_translate_z->setValue(double(translate[2]));
    ui->doubleSpinBox_rotate_x->setValue(double(rotate[0]));
    ui->doubleSpinBox_rotate_y->setValue(double(rotate[1]));
    ui->doubleSpinBox_rotate_z->setValue(double(rotate[2]));
    ui->doubleSpinBox_rotate_angle->setValue(double(rotate_d));
    ui->doubleSpinBox_scale_x->setValue(double(scale[0]));
    ui->doubleSpinBox_scale_y->setValue(double(scale[1]));
    ui->doubleSpinBox_scale_z->setValue(double(scale[2]));

    switch(type)
    {
    case PLANE:
        ui->comboBox_object_type->setCurrentIndex(0);
        break;
    case CUBE:
        ui->comboBox_object_type->setCurrentIndex(1);
        break;
    case CYLINDER:
        ui->comboBox_object_type->setCurrentIndex(2);
        ui->spinBox_subdivisions->setEnabled(true);
        ui->doubleSpinBox_height->setEnabled(true);
        ui->doubleSpinBox_width->setEnabled(true);
        ui->checkBox_caps->setEnabled(true);
        break;
    }

    for(std::size_t x = 0 ; x < scene->materials.size() ; ++x)
    {
        ui->comboBox_material->addItem(QString::fromStdString(scene->materials.at(x).m_name));
        if(QString::fromStdString(scene->materials.at(x).m_name) == material)
            ui->comboBox_material->setCurrentIndex(int(x));
    }

    ui->spinBox_subdivisions->setValue(int(subdivisions));
    ui->doubleSpinBox_height->setValue(double(height));
    ui->doubleSpinBox_width->setValue(double(width));
    ui->checkBox_caps->setChecked(caps);
}

DialogObject::~DialogObject()
{
    delete ui;
}

void DialogObject::on_buttonBox_accepted()
{
    object_type ot;

    switch (ui->comboBox_object_type->currentIndex()) {
    case 0:
        ot = PLANE;
        break;
    case 1:
        ot = CUBE;
        break;
    case 2:
        ot = CYLINDER;
        break;
    default:
        ot = PLANE;
        break;
    }

    SceneObject so(ui->lineEdit_name->text().toStdString(),
                   vec3(ui->doubleSpinBox_translate_x->value(),
                        ui->doubleSpinBox_translate_y->value(),
                        ui->doubleSpinBox_translate_z->value()),
                   vec3(ui->doubleSpinBox_rotate_x->value(),
                        ui->doubleSpinBox_rotate_y->value(),
                        ui->doubleSpinBox_rotate_z->value()),
                   float(ui->doubleSpinBox_rotate_angle->value()),
                   vec3(ui->doubleSpinBox_scale_x->value(),
                        ui->doubleSpinBox_scale_y->value(),
                        ui->doubleSpinBox_scale_z->value()),
                   ot,
                   ui->comboBox_material->currentText().toStdString(),
                   std::size_t(ui->spinBox_subdivisions->value()),
                   float(ui->doubleSpinBox_height->value()),
                   float(ui->doubleSpinBox_width->value()),
                   ui->checkBox_caps->isChecked()
                   );

    if(id >= 0)
        scene->objects[std::size_t(id)] = so;
    else
        scene->objects.push_back(so);
}
