#ifndef RENDER_H
#define RENDER_H

// QT includes.
#include <QImage>
#include <QColor>
#include <QMessageBox>
#include <QProgressDialog>

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
        const Ray&              r,
        const VisualObjectList& world,
        const int               depth = 0) const;

    float ray_hit_sphere(
        const glm::vec3&        center,
        float                   radius,
        const Ray&              r);

    void get_render_image(
        const size_t            width,
        const size_t            height,
        const size_t            spp,
        const Camera&           camera,
        const VisualObjectList& world,
        QImage&                 image) const;

    void get_render_image_thread(
        const size_t            width,
        const size_t            height,
        const size_t            spp,
        const Camera&           camera,
        const VisualObjectList& world,
        QImage&                 image) const;

private:
    glm::vec3 random_in_unit_sphere() const;
};

#endif // RENDER_H
