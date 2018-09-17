#include "material.h"

using namespace glm;

Material::Material(vec3 ambient, vec3 diffuse, vec3 specular, float shininess)
    : ambient(ambient)
    , diffuse(diffuse)
    , specular(specular)
    , shininess(shininess)
{

}

bool Material::scatter(const Ray &r_in, const HitRecord &rec, Ray &scattered)
{
    vec3 target = rec.p + rec.normal +random_in_unit_sphere();
    scattered = Ray(rec.p, target-rec.p);

    return true;
}

vec3 Material::reflect(const vec3 &v, const vec3 &n)
{
    return v - 2 * dot(v, n) * n;
}

Material Material::lambertian()
{
    return Material(vec3(0.0f, 0.0f, 0.0f),
                    vec3(0.0f, 0.0f, 0.0f),
                    vec3(0.0f, 0.0f, 0.0f),
                    0.0f);
}

Material Material::metal()
{
    return Material(vec3(0.0f, 0.0f, 0.0f),
                    vec3(0.0f, 0.0f, 0.0f),
                    vec3(0.0f, 0.0f, 0.0f),
                    1.0f);
}

vec3 Material::random_in_unit_sphere()
{
    vec3 p;
    do
    {
        p = 2.0f * vec3(drand48(), drand48(), drand48()) - vec3(1, 1, 1);
    } while(dot(p, p) >= 1);
    return p;
}
