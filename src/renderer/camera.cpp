// Interface.
#include "renderer/camera.h"

// couscous includes.
#include "ray.h"

// Math includes.
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

Camera::Camera(
    const vec3&     position,
    const vec3&     up,
    const float&    yaw,
    const float&    pitch,
    const float&    fov)
  : m_position(position)
  , m_world_up(up)
  , m_front(0.0f, 0.0f, -1.0f)
  , m_yaw(yaw)
  , m_pitch(pitch)
  , m_fov(fov)
{
    update_vectors();
}

const vec3& Camera::get_position() const
{
    return m_position;
}

mat4 Camera::get_view_matrix() const
{
    return lookAt(m_position, m_position + m_front, m_up);
}

mat4 Camera::get_projection_matrix(
    const size_t    width,
    const size_t    height) const
{
    return perspective(
        m_fov,
        static_cast<float>(width) / static_cast<float>(height),
        0.001f, 1000.0f);
}

void Camera::get_plane_vectors(
    vec3&           lower_left,
    vec3&           horizontal,
    vec3&           vertical,
    const size_t    width,
    const size_t    height) const
{
    // Calculate the matrix going from image space to world space.
    const mat4 inverse_view_proj = inverse(
        get_projection_matrix(width, height)
        * get_view_matrix());

    // Calculate world position of image corners.
    vec4 plane_lower_left = inverse_view_proj * vec4(-1, -1, 0, 1);
    vec4 plane_lower_right = inverse_view_proj * vec4(1, -1, 0, 1);
    vec4 plane_upper_left = inverse_view_proj * vec4(-1, 1, 0, 1);

    // Get the lower left pixel world position.
    lower_left = vec3(plane_lower_left / plane_lower_left.w);

    // Get the horizontal world vector of the image.
    horizontal =
        vec3(plane_lower_right / plane_lower_right.w)
        - lower_left;

    // Get the vertical world vector of the image.
    vertical =
        vec3(plane_upper_left / plane_upper_left.w)
        - lower_left;
}

void Camera::update_vectors()
{
    // Calculate the front of the camera.
    vec3 front;
    front.x = cos(radians(m_yaw)) * cos(radians(m_pitch));
    front.y = sin(radians(m_pitch));
    front.z = sin(radians(m_yaw)) * cos(radians(m_pitch));
    m_front = normalize(front);

    // Then calculate right and up vectors.
    m_right = normalize(cross(m_front, m_world_up));
    m_up = normalize(cross(m_right, m_front));
}

