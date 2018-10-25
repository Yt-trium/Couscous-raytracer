#include "scene.h"

using namespace std;
using namespace glm;

#include <iostream>

void Scene::create_scene(MeshGroup &world)
{
    SceneMaterial *material;
    SceneObject *object;
    SceneOBJ *obj;
    std::size_t i, j;

    std::vector<std::shared_ptr<Material>> mats;

    for(i = 0 ; i < materials.size() ; ++i)
    {
        material = &materials.at(i);

        if(material->m_emission == vec3(0.0f))
        {
            shared_ptr<Material> mat(new Lambertian(material->m_color));
            mats.push_back(mat);
        }
        else
        {
            shared_ptr<Material> mat(new Light(material->m_emission));
            mats.push_back(mat);
        }
    }

    for(i = 0 ; i < objects.size() ; ++i)
    {
        object = &objects.at(i);

        mat4 transform;
        transform = translate(mat4(1.0f), object->m_translate);
        if(object->m_rotate_d != 0.0f)
            transform = rotate(transform, radians(object->m_rotate_d), object->m_rotate);
        transform = scale(transform, object->m_scale);

        shared_ptr<Material> mat_dft(new Lambertian(vec3(0.73f, 0.73f, 0.73f)));
        for(j = 0 ; j < materials.size() ; ++j)
        {
            if(object->m_material == materials.at(j).m_name)
            {
                mat_dft = mats.at(j);
                break;
            }
        }
        switch(object->m_type)
        {
        case PLANE:
            create_plane(world, mat_dft, transform);
            break;
        case CUBE:
            create_cube(world, mat_dft, transform);
            break;
        case CYLINDER:
            create_cylinder(world, mat_dft, object->m_subdivisions,
                            object->m_height, object->m_width,
                            object->m_caps, transform);
            break;
        }
    }

    for(i = 0 ; i < objs.size() ; ++i)
    {
        obj = &objs.at(i);

        if(obj->normals.size() == 0 ||
           obj->triangles.size() == 0 ||
           obj->vertices.size() == 0)
            continue;

        std::cout << obj->triangles.size() << " " <<
                     obj->vertices.size() << " " <<
                     obj->normals.size() << " " <<
                     obj->triangles.data() << " " <<
                     obj->vertices.data() << " " <<
                     obj->normals.data() << std::endl;

        obj = &objs.at(i);
        shared_ptr<Material> mat_dft(new Lambertian(vec3(0.73f, 0.73f, 0.73f)));
        create_triangle_mesh(world,
                             size_t(obj->triangles.size()/3),
                             obj->vertices.size(),
                             obj->triangles.data(),
                             obj->vertices.data(),
                             obj->normals.data(),
                             mat_dft,
                             mat4(0.0001f));
    }
}
