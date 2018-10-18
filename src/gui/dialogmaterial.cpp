#include "dialogmaterial.h"
#include "ui_dialogmaterial.h"

using namespace glm;

DialogMaterial::DialogMaterial(QWidget *p, Scene *s, std::size_t i) :
    QDialog(p),
    ui(new Ui::DialogMaterial)
{
    scene = s;
    id = i;
    parent = p;
    ui->setupUi(this);

    QString name = QString::fromStdString(s->materials.at(i).m_name);
    vec3 color(s->materials.at(i).m_color);
    vec3 emission(s->materials.at(i).m_emission);

    ui->lineEdit_name->setText(name);
    ui->doubleSpinBox_color_r->setValue(double(color[0]));
    ui->doubleSpinBox_color_g->setValue(double(color[1]));
    ui->doubleSpinBox_color_b->setValue(double(color[2]));
    ui->doubleSpinBox_emission_r->setValue(double(emission[0]));
    ui->doubleSpinBox_emission_g->setValue(double(emission[1]));
    ui->doubleSpinBox_emission_b->setValue(double(emission[2]));
}
DialogMaterial::~DialogMaterial()
{
    delete ui;
}

void DialogMaterial::on_buttonBox_accepted()
{
    vec3 color(ui->doubleSpinBox_color_r->value(),
               ui->doubleSpinBox_color_g->value(),
               ui->doubleSpinBox_color_b->value());

    vec3 emission(ui->doubleSpinBox_emission_r->value(),
                  ui->doubleSpinBox_emission_g->value(),
                  ui->doubleSpinBox_emission_b->value());

    SceneMaterial sm(ui->lineEdit_name->text().toStdString(), color, emission);
    scene->materials[id] = sm;
}
