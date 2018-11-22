
// Interface.
#include "test.h"

// catch2 includes.
#define CATCH_CONFIG_RUNNER
#include "test/catch.hpp"

// couscous includes.
#include "renderer/aabb.h"
#include "renderer/ray.h"

// glm includes.
#include <glm/glm.hpp>

using namespace glm;
using namespace std;

int run_tests()
{
    return Catch::Session().run();
}

TEST_CASE( "AABB Ray intersection", "[aabb]" )
{
    AABB bbox(vec3(-1.0f), vec3(1.0f));
    Ray ray(vec3(0.0f, 0.0f, 200.0f), vec3(0.0f, 0.0f, -1.0f));

    // Ray intersecting the bbox.
    bool result = bbox.intersect(ray);
    REQUIRE(result);

    // Ray not intersecting, too far on the right.
    ray.origin.x = 300.0f;
    result = bbox.intersect(ray);
    REQUIRE(!result);
}

TEST_CASE( "AABB Triangle creation", "[aabb]" )
{
    /*
                    v0
                   / \
                  /   \
                 v1..  \
                     ``v2
    */

    const vec3 v0(1.0f, 2.0f, 0.0f);
    const vec3 v1(-1.0f, 0.3f, 1.0f);
    const vec3 v2(3.0f, -1.0f, -1.0f);

    AABB bbox(v0);
    bbox.add_point(v1);
    bbox.add_point(v2);

    const vec3 expected_min(-1.0f, -1.0f, -1.0f);
    const vec3 expected_max(3.0f, 2.0f, 1.0f);

    REQUIRE(bbox.min == expected_min);
    REQUIRE(bbox.max == expected_max);
}

