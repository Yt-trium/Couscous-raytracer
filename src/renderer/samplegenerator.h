#ifndef RENDERER_SAMPLEGENERATOR_H
#define RENDERER_SAMPLEGENERATOR_H

// glm headers.
#include <glm/glm.hpp>

// Generate jittered subpixel positions.
class SampleGenerator
{
  public:
    SampleGenerator(const size_t dimension_size);

    // Returns the next sample position.
    glm::vec3 next();

  private:
    const size_t    m_dim_size; // number of subpixels in both dimensions
    const float     m_stride; // subpixels size
    size_t          m_count; // number of generated samples so far
};

#endif // RENDERER_SAMPLEGENERATOR_H

