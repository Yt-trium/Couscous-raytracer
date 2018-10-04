#ifndef RENDER_H
#define RENDER_H

// QT includes.
#include <QObject>
#include <QImage>
#include <QColor>
#include <QMessageBox>
#include <QProgressBar>

// couscous includes.
#include "renderer/camera.h"
#include "renderer/ray.h"
#include "renderer/visualobject.h"
#include "renderer/samplegenerator.h"

// Math includes.
#include <glm/vec3.hpp>

// Forward declaration.
class VoxelGridAccelerator;

class Render : public QObject
{
    Q_OBJECT

  public:
    glm::vec3 get_ray_color(
        const Ray&                      r,
        const size_t                    ray_max_depth,
        const VoxelGridAccelerator&     grid,
        const int                       depth = 0) const;

    float ray_hit_sphere(
        const glm::vec3&                center,
        float                           radius,
        const Ray&                      r);

    void get_render_image(
        const size_t                    width,
        const size_t                    height,
        const size_t                    spp,
        const size_t                    ray_max_depth,
        const Camera&                   camera,
        const VoxelGridAccelerator&     grid,
        const bool                      parallel,
        const bool                      preview,
        QImage&                         image,
        QProgressBar&                   progressBar);

  private:
    glm::vec3 random_in_unit_sphere() const;

  signals:
    void render_new_tile();
};

#endif // RENDER_H
