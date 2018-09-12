#ifndef RENDER_H
#define RENDER_H

// QT includes.
#include <QImage>
#include <QColor>

// couscous includes.
#include "renderer/camera.h"
#include "renderer/ray.h"
#include "renderer/visualobject.h"

// Math includes.
#include <glm/vec3.hpp>

class Render
{
  public:
    Render();

    glm::vec3 get_ray_color(
        const Ray&          r,
        VisualObjectList    world) const;

    float ray_hit_sphere(
        const glm::vec3&    center,
        float               radius,
        const Ray&          r);

    QImage get_render_image(
        size_t &width,
        size_t &height,
        size_t &samples,
        Camera &camera,
        VisualObjectList &world) const;

};

#endif // RENDER_H
