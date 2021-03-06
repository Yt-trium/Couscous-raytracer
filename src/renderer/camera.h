#ifndef RENDERER_CAMERA_H
#define RENDERER_CAMERA_H

// couscous includes.
#include "ray.h"

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
    Camera(
        const glm::vec3&    position = glm::vec3(0.0f),
        const glm::vec3&    up = glm::vec3(0.0f, 1.0f, 0.0f),
        const float&        yaw = -90.0f,
        const float&        pitch = 0.0f,
        const float&        fov = 85.0f,
        const size_t&       width = 800,
        const size_t&       height = 600);

    const glm::vec3& get_position() const;

    glm::mat4 get_view_matrix() const;

    glm::mat4 get_projection_matrix(
        const size_t        width,
        const size_t        height) const;

    Ray get_ray(float u, float v) const;

  private:
    glm::vec3   m_position;
    glm::vec3   m_up;
    glm::vec3   m_world_up;
    glm::vec3   m_right;
    glm::vec3   m_front;
    glm::vec3   m_lower_left;
    glm::vec3   m_horizontal;
    glm::vec3   m_vertical;
    float       m_yaw;
    float       m_pitch;
    float       m_fov;
    size_t      m_width;
    size_t      m_height;

    // Compute direction, up and right vectors.
    void update_vectors();
};

#endif // RENDERER_CAMERA_H
