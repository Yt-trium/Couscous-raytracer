
// Inteface.
#include "dialogmaterial.h"
#include "ui_dialogmaterial.h"

using namespace glm;

DialogMaterial::DialogMaterial(QWidget* parent, Scene& scene, const size_t id)
  : QDialog(parent)
  , m_scene(scene)
  , m_id(id)
  , m_ui(new Ui::DialogMaterial)
{
    m_ui->setupUi(this);

    if(m_id != size_t(~0))
    {
        const auto& object = m_scene.materials.at(m_id);
        const QString name = QString::fromStdString(object.name);
        const vec3& color = object.color;
        const vec3& emission = object.emission;

        m_ui->lineEdit_name->setText(name);
        m_ui->doubleSpinBox_color_r->setValue(double(color[0]));
        m_ui->doubleSpinBox_color_g->setValue(double(color[1]));
        m_ui->doubleSpinBox_color_b->setValue(double(color[2]));
        m_ui->doubleSpinBox_emission_r->setValue(double(emission[0]));
        m_ui->doubleSpinBox_emission_g->setValue(double(emission[1]));
        m_ui->doubleSpinBox_emission_b->setValue(double(emission[2]));
        m_ui->doubleSpinBox_kd->setValue(double(object.kd));
        m_ui->doubleSpinBox_ks->setValue(double(object.ks));
        m_ui->doubleSpinBox_specularExponent->setValue(double(object.specularExponent));
    }
}

DialogMaterial::~DialogMaterial()
{
    delete m_ui;
}

void DialogMaterial::on_buttonBox_accepted()
{
    const vec3 color(
        m_ui->doubleSpinBox_color_r->value(),
        m_ui->doubleSpinBox_color_g->value(),
        m_ui->doubleSpinBox_color_b->value());

    const vec3 emission(
        m_ui->doubleSpinBox_emission_r->value(),
        m_ui->doubleSpinBox_emission_g->value(),
        m_ui->doubleSpinBox_emission_b->value());

    const float kd = m_ui->doubleSpinBox_kd->value();
    const float ks = m_ui->doubleSpinBox_ks->value();
    const float specExponent = m_ui->doubleSpinBox_specularExponent->value();

    SceneMaterial sm(m_ui->lineEdit_name->text().toStdString(), color, emission, kd, ks, specExponent);

    if(m_id != size_t(~0))
        m_scene.materials[m_id] = sm;
    else
        m_scene.materials.push_back(sm);
}

