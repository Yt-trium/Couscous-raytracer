
// Interface.
#include "rng.h"

using namespace std;

RNG::RNG()
  : m_engine(random_device()())
  , m_distributor(0.0f, 1.0f)
{
}

float RNG::next()
{
    return m_distributor(m_engine);
}

