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
#include "renderer/photonMapping.h"

// Math includes.
#include <glm/vec3.hpp>

// Forward declaration.
class PhotonMap;
class PhotonTree;
class RNG;
class VoxelGridAccelerator;

class Render : public QObject
{
    Q_OBJECT
  public:
    void get_render_image(
        const size_t                    width,
        const size_t                    height,
        const size_t                    spp,
        const Camera&                   camera,
        const MeshGroup&                world,
        const size_t                    direct_light_rays_count,
        const size_t                    indirect_light_rays_count,
        const size_t                    photons_count,
        const bool                      parallel,
        const bool                      get_normal_color,
        const bool                      get_albedo_color,
        const bool                      display_photon_map,
        const bool                      direct_diffuse,
        const bool                      direct_specular,
        const bool                      direct_phong,
        const bool                      indirect_light,
        QImage&                         image,
        QProgressBar&                   progressBar);

  signals:

    // Emitted when a tile begins.
    void on_tile_begin(
        const size_t                    x0,
        const size_t                    y0,
        const size_t                    x1,
        const size_t                    y1);

    // Emitted when a tile ends.
    void on_tile_end(
        const size_t                    x0,
        const size_t                    y0,
        const size_t                    x1,
        const size_t                    y1,
        const QImage&                   frame);
};

#endif // RENDER_H
