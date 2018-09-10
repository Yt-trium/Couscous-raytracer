#ifndef RENDERER_CAMERA_H
#define RENDERER_CAMERA_H

// glm headers.
#include <glm/glm.hpp>

class Ray;

// 3D movable camera.
//
// Retreive pixel coordinate from the camera:
//  Rx, Ry : resolution
//  Dx, Dy : view plane size
//
//  Pixel(x, y) = m_pos
//      + m_pos.x * (-dx/2 + (i + 0.5) * (dx / rx))
//      + m_pos.y * (-dy/2 + (j + 0.5) * (dy / ry))
//
// Line going through given pixel : m_pos + t * (Pi - m_pos)
//
class Camera
{
  public:
    Camera();

    // Compute the ray going through pixel `i`, `j`.
    Ray compute_ray(
        const size_t i,
        const size_t j,
        const size_t width,
        const size_t height
        ) const;

  private:
    glm::vec3 m_pos;
    glm::vec3 m_target;
    glm::vec3 m_direction;
    glm::vec3 m_up;
    glm::vec3 m_right;

    // Compute direction, up and right vectors.
    void update();
};

#endif // RENDERER_CAMERA_H
