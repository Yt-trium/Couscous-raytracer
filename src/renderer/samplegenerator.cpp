// Interface.
#include "renderer/samplegenerator.h"

// Standard headers.
#include <cmath>

using namespace glm;

SampleGenerator::SampleGenerator(const size_t dimension_size)
  : m_dim_size(dimension_size)
  , m_stride(1.0f / static_cast<float>(m_dim_size))
  , m_count(0)
{
}

vec2 SampleGenerator::next()
{
    if (m_dim_size == 1)
    {
        return vec2(0.5f);
    }

    static const float inv_rand_max = 1.0f / static_cast<float>(RAND_MAX);
    const size_t pos_x = m_count % m_dim_size;
    const size_t pos_y = ((m_count - pos_x) / m_dim_size) % m_dim_size;
    ++m_count;

    // Generate a random number that lies in a subpixel element.
    const float random_x =
        static_cast<float>(rand()) * inv_rand_max * m_stride;
    const float random_y =
        static_cast<float>(rand()) * inv_rand_max * m_stride;

    return vec2(
        pos_x * m_stride + random_x,
        pos_y * m_stride + random_y);
}

