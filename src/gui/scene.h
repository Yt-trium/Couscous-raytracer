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
        const std::string&   name,
        const glm::vec3&     color,
        const glm::vec3&     emission);

    std::string name;
    glm::vec3   color;
    glm::vec3   emission;
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

enum class ObjectFileType { OBJ, OFF };

class SceneObjectFile
{
  public:
    SceneObjectFile();

    std::string                 name;
    Transform                   transform;
    ObjectFileType              type;
    std::string                 material;

    std::vector<glm::vec3>      vertices;
    std::vector<std::size_t>    triangles;
    std::vector<glm::vec3>      normals;
};

//
// A scene camera object.
//

class SceneCamera
{
public:
    SceneCamera(const std::string&  name = "cam",
                const glm::vec3&    position = glm::vec3(0.0f),
                float         yaw = -90.0f,
                float         pitch = 0.0f,
                float         fov = 85.0f,
                size_t        width = 800,
                size_t        height = 600);

    glm::vec3   position;
    float       yaw;
    float       pitch;
    float       fov;
    size_t      width;
    size_t      height;

    std::string name;
};

class Scene
{
  public:
    void create_scene(MeshGroup& world);

    // Create a cornell box scene.
    static Scene cornell_box();

    std::vector<SceneMaterial>      materials;
    std::vector<SceneObject>        objects;
    std::vector<SceneObjectFile>    object_files;
    std::vector<SceneCamera>        cameras;
};

#endif // SCENE_H
