#include "scene.h"

using namespace std;
using namespace glm;

void SceneObject::add_object(MeshGroup &world)
{
    mat4 transform;
    transform = translate(mat4(1.0f), m_translate);
    transform = rotate(transform, radians(m_rotate_d), m_rotate);
    transform = scale(transform, m_scale);

    shared_ptr<Material> mat_dft(new Lambertian(vec3(0.73f, 0.73f, 0.73f)));

    switch(m_type)
    {
    case PLANE:
        create_plane(world, mat_dft, transform);
        break;
    case CUBE:
        create_cube(world, mat_dft, transform);
        break;
    }
}
