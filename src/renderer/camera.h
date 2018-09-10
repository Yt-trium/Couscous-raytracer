#ifndef RENDERER_CAMERA_H
#define RENDERER_CAMERA_H

// glm headers.
#include <glm/glm.hpp>

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

    glm::vec3 m_pos;
    glm::vec3 m_target;
};

#endif // RENDERER_CAMERA_H
