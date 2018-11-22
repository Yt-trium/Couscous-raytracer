#ifndef RENDERER_RNG_G
#define RENDERER_RNG_G

// Standard includes.
#include <random>

// Random number generator.
// Based on on std's random_device, mt19937
// and uniform_real_distribution.
// Can generate numbers in [0.0, 1.0]
class RNG
{
  public:
    RNG();

    // Returns the next random number.
    float next();

  private:
    std::mt19937                            m_engine;
    std::uniform_real_distribution<float>   m_distributor;
};

#endif // RENDERER_RNG_G
