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

// Standard library includes
#include <random>

// Forward declaration.
class VoxelGridAccelerator;
class PhotonTree;


#define PHOTON_FETCH_SIZE 5

class Render : public QObject
{
    Q_OBJECT


  private:

    glm::vec3 randomPointInTriangle(const std::shared_ptr<Triangle>& triangle);

    glm::vec3 random_in_unit_sphere() const;

    glm::vec3 getRandomPointOnLights(const MeshGroup& lights);


  public:

    glm::vec3 get_ray_color_phong(
        const Ray&                                  r,
        const VoxelGridAccelerator&                 grid,
        const MeshGroup&                            lights,
        PhotonTree::Fetcher<PHOTON_FETCH_SIZE>&     pfetcher);

    void get_render_image(
        const size_t                    width,
        const size_t                    height,
        const size_t                    spp,
        const size_t                    ray_max_depth,
        const Camera&                   camera,
        const VoxelGridAccelerator&     grid,
        const PhotonTree&               ptree,
        const MeshGroup&                lights,
        const bool                      parallel,
        const bool                      get_normal_color,
        const bool                      get_albedo_color,
        const bool                      display_photon_map,
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

  private:
    std::mt19937                              engine;
    std::uniform_real_distribution<>          distributor;
};

#endif // RENDER_H
