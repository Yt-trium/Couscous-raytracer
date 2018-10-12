#ifndef SCENE_H
#define SCENE_H

// couscous includes.
#include "renderer/visualobject.h"
#include "renderer/material.h"

// glm includes.
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>

// Standard includes.
#include <string>
#include <vector>

enum object_type {PLANE, CUBE};

class SceneObject
{
public:
    SceneObject(std::string n, glm::vec3 t, glm::vec3 r, float rd, glm::vec3 s, object_type ot, std::string m)
        : m_name(n)
        , m_translate(t)
        , m_rotate(r)
        , m_rotate_d(rd)
        , m_scale(s)
        , m_type(ot)
        , m_material(m)
    {}

    void add_object(MeshGroup& world);

private:
    std::string m_name;
    glm::vec3 m_translate;
    glm::vec3 m_rotate;
    float m_rotate_d;
    glm::vec3 m_scale;
    object_type m_type;
    std::string m_material;
};

class SceneMaterial
{
public:
    SceneMaterial();

private:
    std::string name;
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
