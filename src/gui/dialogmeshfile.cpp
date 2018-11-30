#include "dialogmeshfile.h"
#include "ui_dialogmeshfile.h"

using namespace glm;
using namespace std;

DialogMeshFile::DialogMeshFile(QWidget* parent, Scene& scene, SceneMeshFile& scene_file)
  : QDialog(parent)
  , m_ui(new Ui::DialogMeshFile)
  , m_scene_file(scene_file)
{
    m_ui->setupUi(this);

    const QString name = QString::fromStdString(scene_file.name);
    const QString path = QString::fromStdString(scene_file.path);
    const Transform& transform = scene_file.transform;
    const QString material = QString::fromStdString(scene_file.material);

    m_ui->lineEdit_name->setText(name);
    m_ui->lineEdit_path->setText(path);
    m_ui->doubleSpinBox_translate_x->setValue(double(transform.translate[0]));
    m_ui->doubleSpinBox_translate_y->setValue(double(transform.translate[1]));
    m_ui->doubleSpinBox_translate_z->setValue(double(transform.translate[2]));
    m_ui->doubleSpinBox_rotate_x->setValue(double(transform.rotation[0]));
    m_ui->doubleSpinBox_rotate_y->setValue(double(transform.rotation[1]));
    m_ui->doubleSpinBox_rotate_z->setValue(double(transform.rotation[2]));
    m_ui->doubleSpinBox_scale_x->setValue(double(transform.scale[0]));
    m_ui->doubleSpinBox_scale_y->setValue(double(transform.scale[1]));
    m_ui->doubleSpinBox_scale_z->setValue(double(transform.scale[2]));

    for(size_t x = 0; x < scene.materials.size(); ++x)
    {
        const auto& name = scene.materials.at(x).name;
        m_ui->comboBox_material->addItem(QString::fromStdString(name));

        if(QString::fromStdString(name) == material)
            m_ui->comboBox_material->setCurrentIndex(static_cast<int>(x));
    }
}

DialogMeshFile::~DialogMeshFile()
{
    delete m_ui;
}

void DialogMeshFile::on_buttonBox_accepted()
{
    const Transform transform(
        vec3(m_ui->doubleSpinBox_translate_x->value(),
            m_ui->doubleSpinBox_translate_y->value(),
            m_ui->doubleSpinBox_translate_z->value()),
        vec3(m_ui->doubleSpinBox_rotate_x->value(),
            m_ui->doubleSpinBox_rotate_y->value(),
            m_ui->doubleSpinBox_rotate_z->value()),
        vec3(m_ui->doubleSpinBox_scale_x->value(),
            m_ui->doubleSpinBox_scale_y->value(),
            m_ui->doubleSpinBox_scale_z->value()));

    m_scene_file.name = m_ui->lineEdit_name->text().toStdString();
    m_scene_file.path = m_ui->lineEdit_path->text().toStdString();
    m_scene_file.transform = transform;
    m_scene_file.material = m_ui->comboBox_material->currentText().toStdString();
}

