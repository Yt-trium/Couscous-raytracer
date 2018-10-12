#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <glm/glm.hpp>

enum object_type {PLANE, CUBE};

class SceneObject
{
public:
    SceneObject();

    glm::vec3 translate;
    glm::vec3 rotate;
    glm::vec3 scale;
    object_type type;
};

class SceneMaterial
{
public:
    SceneMaterial();

    glm::vec3 color;
    glm::vec3 emission;
};

class Scene
{
public:
    Scene();

    std::vector<SceneMaterial> materials;
    std::vector<SceneObject> objects;
};

#endif // SCENE_H
