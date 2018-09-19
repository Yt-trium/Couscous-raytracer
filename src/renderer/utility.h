#ifndef RENDERER_UTILITY_H
#define RENDERER_UTILITY_H

// Standard includes.
#include <cstdlib>

// Generate a random number in [0, 1].
template <typename T>
inline T random();


//
// Implementation.
//

template <typename T>
inline T random()
{
    static const T inv_rand_max = T(1) / static_cast<T>(RAND_MAX);
    return std::rand() * inv_rand_max;
}

#endif // RENDERER_UTILITY_H
