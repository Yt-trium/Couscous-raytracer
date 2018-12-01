#ifndef SCENE_H
#define SCENE_H

// couscous includes.
#include "renderer/material.h"
#include "renderer/visualobject.h"

// glm includes.
#include <glm/glm.hpp>

// Standard includes.
#include <memory>
#include <string>
#include <vector>

//
// An editable material.
//

class SceneMaterial
{
  public:
    SceneMaterial(
        const std::string&  name,
        const glm::vec3&    color,
        const glm::vec3&    emission,
        const float         kd,
        const float         ks,
        const float         specularExponent);

    std::string name;
    glm::vec3   color;
    glm::vec3   emission;
    float       kd;
    float       ks;
    float       specularExponent;
};


//
// A Transform is simply a rotation, translate
// and scale value that can be applied to any 3D point.
//

class Transform
{
  public:
    Transform(
        const glm::vec3& translate = glm::vec3(0.0f),
        const glm::vec3& rotation = glm::vec3(0.0f),
        const glm::vec3& scale = glm::vec3(0.0f));

    glm::mat4 matrix() const;

    glm::vec3 translate;
    glm::vec3 rotation;
    glm::vec3 scale;
};


//
// An editable 3D object instance.
//

enum class ObjectType { PLANE, CUBE, CYLINDER };

class SceneObject
{
  public:
    SceneObject(
        const std::string&  name,
        const Transform&    transform,
        const ObjectType    ot,
        const std::string&  material_name);

    std::string     name;
    Transform       transform;
    ObjectType      type;
    std::string     material;

    std::size_t     subdivisions;
    float           width;
    float           height;
    bool            caps;
};

//
// A 3D object file instance.
//

class SceneMeshFile
{
  public:
    SceneMeshFile(
        const std::string&  name,
        const std::string&  path,
        const Transform&     transform,
        const std::string&  material_name);

    std::string                 name;
    std::string                 path;
    Transform                   transform;
    std::string                 material;
};

//
// A scene camera object.
//

class SceneCamera
{
  public:
    SceneCamera(
        const std::string&  name,
        const glm::vec3&    position,
        const float         yaw,
        const float         pitch,
        const float         fov,
        const size_t        width,
        const size_t        height);

    std::string name;
    glm::vec3   position;
    float       yaw;
    float       pitch;
    float       fov;
    size_t      width;
    size_t      height;
};

class Scene
{
  public:
    void create_scene(MeshGroup& world);

    // Create a cornell box scene.
    static Scene cornell_box();
    static Scene cornell_box_suzanne();
    static Scene cornell_box_orange_and_blue();
    static Scene simple_cube();
    static Scene sphere();

    std::vector<SceneMaterial>      materials;
    std::vector<SceneObject>        objects;
    std::vector<SceneMeshFile>      object_files;
    std::vector<SceneCamera>        cameras;
};

#endif // SCENE_H
