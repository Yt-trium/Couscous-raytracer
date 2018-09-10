// Interface.
#include "renderer/camera.h"

#include "ray.h"

using namespace glm;

Camera::Camera()
  : m_pos(0.0f, 0.0f, 0.0f)
  , m_target(0.0f, 0.0f, 5.0f)
{
    update();
}

Camera::Camera(vec3 pos, vec3 target, vec3 direction, vec3 up, vec3 right)
    : m_pos(pos)
    , m_target(target)
    , m_direction(direction)
    , m_up(up)
    , m_right(right)
{

}

Ray Camera::compute_ray(
    const size_t i,
    const size_t j,
    const size_t width,
    const size_t height) const
{
    float normalized_i = (i / width) - 0.5f;
    float normalized_j = (j / height) - 0.5f;

    vec3 image_point = normalized_i * m_right
        + normalized_j * m_up
        + m_pos
        + m_direction;

    vec3 ray_direction = image_point - m_pos;

    return Ray(m_pos, ray_direction);
}

void Camera::update()
{
    m_direction = normalize(m_target - m_pos);
    m_up = vec3(0.0f, 1.0f, 0.0f);
    m_right = cross(m_direction, m_up);
}

