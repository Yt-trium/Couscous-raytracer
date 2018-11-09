#include "scene.h"

#include "renderer/visualobject.h"

#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

//
// Editable material implementation.
//

SceneMaterial::SceneMaterial(
    const string&   name,
    const vec3&     color,
    const vec3&     emission)
  : name(name)
  , color(color)
  , emission(emission)
{
}


//
// 3D Transform implementation.
//

Transform::Transform(
    const glm::vec3& translate,
    const glm::vec3& rotation,
    const glm::vec3& scale)
  : translate(translate)
  , rotation(rotation)
  , scale(scale)
{
}

mat4 Transform::matrix() const
{
    // Apply translation.
    mat4 transform = glm::translate(mat4(1.0f), translate);

    // Apply rotation for each axis.
    transform = glm::rotate(transform, radians(rotation.x), vec3(1.0f, 0.0f, 0.0f));
    transform = glm::rotate(transform, radians(rotation.y), vec3(0.0f, 1.0f, 0.0f));
    transform = glm::rotate(transform, radians(rotation.z), vec3(0.0f, 0.0f, 1.0f));

    // Apply scale.
    transform = glm::scale(transform, scale);

    return transform;
}



//
// Editable 3D Object implementation.
//

SceneObject::SceneObject(
    const std::string&  name,
    const Transform&    transform,
    const ObjectType    ot,
    const std::string&  material_name)
  : name(name)
  , transform(transform)
  , type(ot)
  , material(material_name)
  , subdivisions(0)
  , width(0.0f)
  , height(0.0f)
  , caps(false)
{
}

SceneObjectFile::SceneObjectFile()
{
}

void Scene::create_scene(MeshGroup &world)
{
    SceneMaterial *material;
    SceneObject *object;
    SceneObjectFile *obj;
    std::size_t i, j;

    std::vector<std::shared_ptr<Material>> mats;

    for(i = 0 ; i < materials.size() ; ++i)
    {
        material = &materials.at(i);

        if(material->emission == vec3(0.0f))
        {
            shared_ptr<Material> mat(new Lambertian(material->color));
            mats.push_back(mat);
        }
        else
        {
            shared_ptr<Material> mat(new Light(material->emission));
            mats.push_back(mat);
        }
    }

    for(i = 0 ; i < objects.size() ; ++i)
    {
        object = &objects.at(i);

        const mat4 transform = object->transform.matrix();

        shared_ptr<Material> mat_dft(new Lambertian(vec3(0.73f, 0.73f, 0.73f)));
        for(j = 0 ; j < materials.size() ; ++j)
        {
            if(object->material == materials.at(j).name)
            {
                mat_dft = mats.at(j);
                break;
            }
        }
        switch(object->type)
        {
        case ObjectType::PLANE:
            create_plane(world, mat_dft, transform);
            break;
        case ObjectType::CUBE:
            create_cube(world, mat_dft, transform);
            break;
        case ObjectType::CYLINDER:
            create_cylinder(world, mat_dft, object->subdivisions,
                            object->height, object->width,
                            object->caps, transform);
            break;
        }
    }

    for(i = 0 ; i < object_files.size() ; ++i)
    {
        obj = &object_files.at(i);

        if(obj->normals.size() == 0 ||
           obj->triangles.size() == 0 ||
           obj->vertices.size() == 0)
            continue;

        /*
        std::cout << obj->triangles.size() << " " <<
                     obj->vertices.size() << " " <<
                     obj->normals.size() << " " <<
                     obj->triangles.data() << " " <<
                     obj->vertices.data() << " " <<
                     obj->normals.data() << std::endl;
        */

        obj = &object_files.at(i);
        shared_ptr<Material> mat_dft(new Lambertian(vec3(0.73f, 0.73f, 0.73f)));
        create_triangle_mesh(world,
                             size_t(obj->triangles.size()/3),
                             obj->vertices.size(),
                             obj->triangles.data(),
                             obj->vertices.data(),
                             obj->normals.data(),
                             mat_dft,
                             mat4(1.0f));
    }
}

Scene Scene::cornell_box()
{
    Scene scene;

    scene.materials.push_back(SceneMaterial("light", vec3(0.0f), vec3(15.0f)));
    scene.materials.push_back(SceneMaterial("red", vec3(0.65f, 0.05f, 0.05f), vec3(0.0f)));
    scene.materials.push_back(SceneMaterial("green", vec3(0.12f, 0.45f, 0.15f), vec3(0.0f)));
    scene.materials.push_back(SceneMaterial("white", vec3(0.73f, 0.73f, 0.73f), vec3(0.0f)));

    scene.objects.push_back(SceneObject("floor",
        Transform(
            vec3(0.0f, 0.0f, 0.0f),
            vec3(0.0f, 0.0f, 0.0f),
            vec3(200.0f)),
        ObjectType::PLANE,
        "white"));

    scene.objects.push_back(SceneObject("background",
        Transform(
            vec3(0.0f, 100.0f, -100.0f),
            vec3(90.0f, 0.0f, 0.0f),
            vec3(200.0f)),
        ObjectType::PLANE,
        "white"));

    scene.objects.push_back(SceneObject("ceilling",
        Transform(
            vec3(0.0f, 200.0f, 0.0f),
            vec3(180.0f, 0.0f, 0.0f),
            vec3(200.0f)),
        ObjectType::PLANE,
        "white"));

    scene.objects.push_back(SceneObject("top_light",
        Transform(
            vec3(0.0f, 199.0f, 0.0f),
            vec3(180.0f, 0.0f, 0.0f),
            vec3(30.0f)),
        ObjectType::PLANE,
        "light"));

    scene.objects.push_back(SceneObject("left",
        Transform(
            vec3(100.0f, 100.0f, 0.0f),
            vec3(0.0f, 0.0f, 90.0f),
            vec3(200.0f)),
        ObjectType::PLANE,
        "green"));

    scene.objects.push_back(SceneObject("right",
        Transform(
            vec3(-100.0f, 100.0f, 0.0f),
            vec3(0.0f, 0.0f, -90.0f),
            vec3(200.0f)),
        ObjectType::PLANE,
        "red"));

    scene.objects.push_back(SceneObject("left_box",
        Transform(
            vec3(50.0f, 50.0f, -30.0f),
            vec3(0.0f, -20.0f, 0.0f),
            vec3(60.0f, 100.0f, 60.0f)),
        ObjectType::CUBE,
        "white"));

    scene.objects.push_back(SceneObject("right_box",
        Transform(
            vec3(-50.0f, 30.0f, 30.0f),
            vec3(0.0f, 20.0f, 0.0f),
            vec3(60.0f)),
        ObjectType::CUBE,
        "white"));

    scene.cameras.push_back(SceneCamera("CAM_1",
        vec3(0.0f, 100.0f, 385.0f),
        -90.0f,
        0.0f,
        40.0f,
        512,
        512));

    return scene;
}

SceneCamera::SceneCamera(
    const string&       name,
    const vec3&         position,
    const float         yaw,
    const float         pitch,
    const float         fov,
    const size_t        width,
    const size_t        height)
  : name(name)
  , position(position)
  , yaw(yaw)
  , pitch(pitch)
  , fov(fov)
  , width(width)
  , height(height)
{
}

