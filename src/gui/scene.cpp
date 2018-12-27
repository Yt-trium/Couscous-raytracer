#include "scene.h"

// couscous includes.
#include "renderer/visualobject.h"
#include "io/filereader.h"

// glm includes.
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

//
// Editable material implementation.
//

SceneMaterial::SceneMaterial(
    const string&       name,
    const vec3&         color,
    const float         light_power,
    const float         kd,
    const float         ks,
    const float         specularExponent,
    const float         metal,
    const float         roughness)
  : name(name)
  , color(color)
  , light_power(light_power)
  , kd(kd)
  , ks(ks)
  , specularExponent(specularExponent)
  , metal(metal)
  , roughness(roughness)
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

SceneMeshFile::SceneMeshFile(
    const std::string&  name,
    const std::string&  path,
    const Transform&    transform,
    const std::string&  material_name,
    const bool          smooth_shading)
  : name(name)
  , path(path)
  , transform(transform)
  , material(material_name)
  , smooth_shading(smooth_shading)
{
}

void Scene::create_scene(MeshGroup &world)
{
    SceneMaterial *material;
    SceneObject *object;
    std::size_t i, j;

    std::vector<std::shared_ptr<Material>> mats;

    for(i = 0 ; i < materials.size() ; ++i)
    {
        material = &materials.at(i);

        shared_ptr<Material> mat(new Material(
            material->color,
            material->light_power,
            material->kd,
            material->ks,
            material->specularExponent,
            material->metal,
            material->roughness));

        mats.push_back(mat);
    }

    for(i = 0 ; i < objects.size() ; ++i)
    {
        object = &objects.at(i);

        const mat4 transform = object->transform.matrix();

        shared_ptr<Material> mat_default(
            new Material(
                vec3(1.0f, 0.0f, 1.0f),
                1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f));

        for(j = 0 ; j < materials.size() ; ++j)
        {
            if(object->material == materials.at(j).name)
            {
                mat_default = mats.at(j);
                break;
            }
        }

        switch(object->type)
        {
          case ObjectType::PLANE:
            create_plane(world, mat_default, transform);
            break;
          case ObjectType::CUBE:
            create_cube(world, mat_default, transform);
            break;
          case ObjectType::CYLINDER:
            create_cylinder(world, mat_default, object->subdivisions,
                object->height, object->width,
                object->caps, transform);
            break;
        }
    }

    for(i = 0 ; i < object_files.size() ; ++i)
    {
        const auto& obj = object_files.at(i);

        shared_ptr<Material> mat_default(
            new Material(
                vec3(1.0f, 0.0f, 1.0f),
                1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f));

        for(j = 0 ; j < materials.size() ; ++j)
        {
            if(obj.material == materials.at(j).name)
            {
                mat_default = mats.at(j);
                break;
            }
        }

        const MeshOffFile data = read_off(obj.path);

        create_triangle_mesh(world,
             data.faces.size() / 3,
             data.vertices.size(),
             data.faces.data(),
             data.vertices.data(),
             obj.smooth_shading ? data.vertex_normals.data() : data.face_normals.data(),
             mat_default,
             obj.transform.matrix(),
             obj.smooth_shading);
    }
}

Scene Scene::cornell_box()
{
    Scene scene;

    scene.materials.push_back(SceneMaterial("light", vec3(1.0f), 15.0f, 0.0f, 0.0f, 0.0f));
    scene.materials.push_back(SceneMaterial("red", vec3(1.0f, 0.05f, 0.05f), 0.0f, 1.0f, 1.0f, 3.0f));
    scene.materials.push_back(SceneMaterial("green", vec3(0.12f, 1.0, 0.15f), 0.0f, 1.0f, 1.0f, 3.0f));
    scene.materials.push_back(SceneMaterial("white", vec3(0.73f, 0.73f, 0.73f), 0.0f, 1.0f, 1.0f, 3.0f));

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

    scene.objects.push_back(SceneObject("foreground",
        Transform(
            vec3(0.0f, 100.0f, 100.0f),
            vec3(-90.0f, 0.0f, 0.0f),
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
            vec3(45.0f)),
        ObjectType::PLANE,
        "light"));

    scene.objects.push_back(SceneObject("left",
        Transform(
            vec3(-100.0f, 100.0f, 0.0f),
            vec3(0.0f, 0.0f, -90.0f),
            vec3(200.0f)),
        ObjectType::PLANE,
        "red"));

    scene.objects.push_back(SceneObject("right",
        Transform(
            vec3(100.0f, 100.0f, 0.0f),
            vec3(0.0f, 0.0f, 90.0f),
            vec3(200.0f)),
        ObjectType::PLANE,
        "green"));

    scene.objects.push_back(SceneObject("right_box",
        Transform(
            vec3(50.0f, 50.0f, -30.0f),
            vec3(0.0f, -20.0f, 0.0f),
            vec3(60.0f, 100.0f, 60.0f)),
        ObjectType::CUBE,
        "white"));

    scene.objects.push_back(SceneObject("left_box",
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

Scene Scene::cornell_box_window()
{
    Scene scene;

    scene.materials.push_back(SceneMaterial("light", vec3(1.0f), 1.0f, 0.0f, 0.0f, 0.0f));
    scene.materials.push_back(SceneMaterial("red", vec3(1.0f, 0.05f, 0.05f), 0.0f, 1.0f, 1.0f, 3.0f));
    scene.materials.push_back(SceneMaterial("green", vec3(0.12f, 1.0, 0.15f), 0.0f, 1.0f, 1.0f, 3.0f));
    scene.materials.push_back(SceneMaterial("white", vec3(0.73f, 0.73f, 0.73f), 0.0f, 1.0f, 1.0f, 3.0f));
    scene.materials.push_back(SceneMaterial("yellow", vec3(0.73f, 0.73f, 0.03f), 0.0f, 1.0f, 1.0f, 3.0f));

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

    scene.objects.push_back(SceneObject("foreground",
        Transform(
            vec3(0.0f, 100.0f, 100.0f),
            vec3(-90.0f, 0.0f, 0.0f),
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

    scene.object_files.emplace_back(
        "left_sphere",
        "assets/sphere_16_16.off",
        Transform(
            vec3(-50.0f, 40.0f, 0.0f),
            vec3(0.0f),
            vec3(40.0f)),
        "yellow",
        true);

    scene.objects.push_back(SceneObject("top_light",
        Transform(
            vec3(200.0f, 120.0f, 0.0f),
            vec3(90.0f, 20.0f, 90.0f),
            vec3(45.0f)),
        ObjectType::PLANE,
        "light"));

    scene.objects.push_back(SceneObject("left",
        Transform(
            vec3(-100.0f, 100.0f, 0.0f),
            vec3(0.0f, 0.0f, -90.0f),
            vec3(200.0f)),
        ObjectType::PLANE,
        "red"));


    scene.object_files.emplace_back(
        "window",
        "assets/wall_window.off",
        Transform(
            vec3(100.0f, 100.0f, 0.0f),
            vec3(-90.0f, 0.0f, -90.0f),
            vec3(200.0f)),
        "green",
        true);

    scene.cameras.push_back(SceneCamera("CAM_1",
        vec3(0.0f, 100.0f, 385.0f),
        -90.0f,
        0.0f,
        40.0f,
        512,
        512));

    return scene;
}


Scene Scene::cornell_box_metal()
{
    Scene scene;

    scene.materials.push_back(SceneMaterial("light", vec3(1.0f), 1.0f, 0.0f, 0.0f, 0.0f));
    scene.materials.push_back(SceneMaterial("red", vec3(1.0f, 0.05f, 0.05f), 0.0f, 1.0f, 1.0f, 3.0f));
    scene.materials.push_back(SceneMaterial("green", vec3(0.12f, 1.0, 0.15f), 0.0f, 1.0f, 1.0f, 3.0f));
    scene.materials.push_back(SceneMaterial("metal", vec3(0.12f, 1.0, 0.15f), 0.0f, 1.0f, 1.0f, 3.0f, 1.0f, 0.0f));
    scene.materials.push_back(SceneMaterial("roughmetal", vec3(0.12f, 1.0, 0.15f), 0.0f, 1.0f, 1.0f, 3.0f, 1.0f, 0.1f));
    scene.materials.push_back(SceneMaterial("white", vec3(0.73f, 0.73f, 0.73f), 0.0f, 1.0f, 1.0f, 3.0f));

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

    scene.objects.push_back(SceneObject("foreground",
        Transform(
            vec3(0.0f, 100.0f, 100.0f),
            vec3(-90.0f, 0.0f, 0.0f),
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
            vec3(45.0f)),
        ObjectType::PLANE,
        "light"));

    scene.objects.push_back(SceneObject("left",
        Transform(
            vec3(-100.0f, 100.0f, 0.0f),
            vec3(0.0f, 0.0f, -90.0f),
            vec3(200.0f)),
        ObjectType::PLANE,
        "red"));

    scene.objects.push_back(SceneObject("right",
        Transform(
            vec3(100.0f, 100.0f, 0.0f),
            vec3(0.0f, 0.0f, 90.0f),
            vec3(200.0f)),
        ObjectType::PLANE,
        "green"));

    scene.object_files.emplace_back(
        "right_sphere",
        "assets/sphere_16_16.off",
        Transform(
            vec3(50.0f, 40.0f, 0.0f),
            vec3(0.0f),
            vec3(40.0f)),
        "metal",
        true);

    scene.object_files.emplace_back(
        "left_sphere",
        "assets/sphere_16_16.off",
        Transform(
            vec3(-50.0f, 40.0f, 0.0f),
            vec3(0.0f),
            vec3(40.0f)),
        "roughmetal",
        true);

    scene.cameras.push_back(SceneCamera("CAM_1",
        vec3(0.0f, 100.0f, 385.0f),
        -90.0f,
        0.0f,
        40.0f,
        512,
        512));

    return scene;
}


Scene Scene::cornell_box_suzanne()
{
    Scene scene;

    scene.materials.push_back(SceneMaterial("light", vec3(1.0f), 1.0f, 0.0f, 0.0f, 0.0f));
    scene.materials.push_back(SceneMaterial("red", vec3(1.0f, 0.05f, 0.05f), 0.0f, 1.0f, 1.0f, 3.0f));
    scene.materials.push_back(SceneMaterial("green", vec3(0.12f, 1.0, 0.15f), 0.0f, 1.0f, 1.0f, 3.0f));
    scene.materials.push_back(SceneMaterial("white", vec3(0.73f, 0.73f, 0.73f), 0.0f, 1.0f, 1.0f, 3.0f));

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

    scene.objects.push_back(SceneObject("foreground",
        Transform(
            vec3(0.0f, 100.0f, 100.0f),
            vec3(-90.0f, 0.0f, 0.0f),
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
            vec3(45.0f)),
        ObjectType::PLANE,
        "light"));

    scene.objects.push_back(SceneObject("left",
        Transform(
            vec3(-100.0f, 100.0f, 0.0f),
            vec3(0.0f, 0.0f, -90.0f),
            vec3(200.0f)),
        ObjectType::PLANE,
        "red"));

    scene.objects.push_back(SceneObject("right",
        Transform(
            vec3(100.0f, 100.0f, 0.0f),
            vec3(0.0f, 0.0f, 90.0f),
            vec3(200.0f)),
        ObjectType::PLANE,
        "green"));

    scene.object_files.emplace_back(
        "suzanne",
        "assets/suzanne.off",
        Transform(
            vec3(0.0f, 100.0f, 0.0f),
            vec3(0.0f, 0.0f, 0.0f),
            vec3(50.0f)),
        "white");

    scene.cameras.push_back(SceneCamera("CAM_1",
        vec3(0.0f, 100.0f, 385.0f),
        -90.0f,
        0.0f,
        40.0f,
        512,
        512));

    return scene;
}

Scene Scene::cornell_box_orange_and_blue()
{
    Scene scene;

    scene.materials.push_back(SceneMaterial("blue_light", vec3(0.2f, 2.0f, 1.0f), 1.0f, 0.0f, 0.0f, 3.0f));
    scene.materials.push_back(SceneMaterial("orange_light", vec3(1.0f, 0.6f, 0.2f), 1.0f, 0.0f, 0.0f, 3.0f));
    scene.materials.push_back(SceneMaterial("white", vec3(0.73f, 0.73f, 0.73f), 0.0f, 1.0f, 1.0f, 3.0f));

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

    scene.objects.push_back(SceneObject("foreground",
        Transform(
            vec3(0.0f, 100.0f, 100.0f),
            vec3(-90.0f, 0.0f, 0.0f),
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

    scene.objects.push_back(SceneObject("left",
        Transform(
            vec3(-100.0f, 100.0f, 0.0f),
            vec3(0.0f, 0.0f, -90.0f),
            vec3(200.0f)),
        ObjectType::PLANE,
        "blue_light"));

    scene.objects.push_back(SceneObject("right",
        Transform(
            vec3(100.0f, 100.0f, 0.0f),
            vec3(0.0f, 0.0f, 90.0f),
            vec3(200.0f)),
        ObjectType::PLANE,
        "orange_light"));

    scene.object_files.emplace_back(
        "right_box",
        "assets/cube.off",
        Transform(
            vec3(50.0f, 50.0f, -30.0f),
            vec3(0.0f, -20.0f, 0.0f),
            vec3(60.0f, 100.0f, 60.0f)),
        "white");

    scene.objects.push_back(SceneObject("left_box",
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

Scene Scene::simple_cube()
{
    Scene scene;

    scene.materials.push_back(SceneMaterial("light", vec3(1.0f), 1.0f, 0.0f, 0.0f, 0.0f));
    scene.materials.push_back(SceneMaterial("red", vec3(0.65f, 0.05f, 0.05f), 0.0f, 1.0f, 0.5f, 0.1f));
    scene.materials.push_back(SceneMaterial("grey", vec3(0.2, 0.2, 0.2), 0.0f, 1.0f, 0.5f, 0.1f));

    scene.object_files.emplace_back(
        "cube",
        "assets/cube.off",
        Transform(
            vec3(0.5f, 0.25f, 0.0f),
            vec3(0.0f, 45.0f, 0.0f),
            vec3(0.5f)),
        "red");

    scene.object_files.emplace_back(
        "floor",
        "assets/plane_sub_01.off",
        Transform(
            vec3(0.0f, 0.0f, 0.0f),
            vec3(0.0f, 0.0f, 0.0f),
            vec3(5.0f)),
        "grey");

    scene.object_files.emplace_back(
        "top_light",
        "assets/plane.off",
        Transform(
            vec3(0.0f, 1.5f, 0.0f),
            vec3(180.0f, 0.0f, 0.0f),
            vec3(1.0f)),
        "light");

    scene.cameras.push_back(SceneCamera("CAM_1",
        vec3(0.0f, 0.8f, 3.0f),
        -90.0f,
        0.0f,
        40.0f,
        512,
        512));

    return scene;
}

Scene Scene::sphere()
{
    Scene scene;

    scene.materials.push_back(SceneMaterial("light", vec3(1.0f), 1.0f, 0.0f, 0.0f, 0.0f));
    scene.materials.push_back(SceneMaterial("grey", vec3(0.73f), 0.0f, 1.0f, 0.2f, 2.0f));

    scene.object_files.emplace_back(
        "floor",
        "assets/plane.off",
        Transform(
            vec3(0.0f, -2.0, 0.0f),
            vec3(0.0f, 0.0f, 0.0f),
            vec3(10.0f)),
        "grey");

    scene.object_files.emplace_back(
        "right_light",
        "assets/plane.off",
        Transform(
            vec3(3.0f, 3.0f, 0.0f),
            vec3(180.0f, 0.0f, 45.0f),
            vec3(3.0f)),
        "light");

    scene.object_files.emplace_back(
        "sphere",
        "assets/sphere_16_16.off",
        Transform(
            vec3(0.0f),
            vec3(0.0f),
            vec3(2.0f)),
        "grey",
        true);

    scene.cameras.push_back(SceneCamera("CAM_1",
        vec3(0.0f, 0.8f, 20.0f),
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

