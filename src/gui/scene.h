#ifndef SCENE_H
#define SCENE_H

// couscous includes.
#include "renderer/material.h"
#include "renderer/visualobject.h"

// glm includes.
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Standard includes.
#include <memory>
#include <string>
#include <vector>

enum class ObjectType { PLANE, CUBE, CYLINDER, OBJ, OFF };

class SceneMaterial
{
  public:
    SceneMaterial(
        const std::string&   name,
        const glm::vec3&     color,
        const glm::vec3&     emission);

    std::string m_name;
    glm::vec3   m_color;
    glm::vec3   m_emission;
};

class SceneObject
{
public:
    SceneObject(std::string n,
                glm::vec3 t,
                glm::vec3 r,
                float rd,
                glm::vec3 s,
                ObjectType ot,
                std::string m)
        : m_name(n)
        , m_translate(t)
        , m_rotate(r)
        , m_rotate_d(rd)
        , m_scale(s)
        , m_type(ot)
        , m_material(m)
        , m_subdivisions(0)
        , m_height(0)
        , m_width(0)
        , m_caps(false)
    {}

    SceneObject(std::string n,
                glm::vec3 t,
                glm::vec3 r,
                float rd,
                glm::vec3 s,
                ObjectType ot,
                std::string m,
                std::size_t sd,
                float h,
                float w,
                bool c)
        : m_name(n)
        , m_translate(t)
        , m_rotate(r)
        , m_rotate_d(rd)
        , m_scale(s)
        , m_type(ot)
        , m_material(m)
        , m_subdivisions(sd)
        , m_height(h)
        , m_width(w)
        , m_caps(c)
    {
    }

    std::string m_name;
    glm::vec3 m_translate;
    glm::vec3 m_rotate;
    float m_rotate_d;
    glm::vec3 m_scale;
    ObjectType m_type;
    std::string m_material;

    std::size_t m_subdivisions;
    float m_height;
    float m_width;
    bool m_caps;
};

class SceneOBJ
{
public:
    SceneOBJ(){}
    std::string m_name;
    glm::vec3 m_translate;
    glm::vec3 m_rotate;
    float m_rotate_d;
    glm::vec3 m_scale;
    ObjectType m_type;
    std::string m_material;

    std::vector<glm::vec3> vertices;
    std::vector<std::size_t> triangles;
    std::vector<glm::vec3> normals;
};

class Scene
{
public:
    Scene(){}

    void create_scene(MeshGroup& world);

    std::vector<SceneMaterial> materials;
    std::vector<SceneObject> objects;
    std::vector<SceneOBJ> objs;
};

#endif // SCENE_H
