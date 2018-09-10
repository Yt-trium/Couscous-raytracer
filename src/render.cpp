#include "render.h"
Render::Render()
{

}

glm::vec3 Render::getRayColor(const Ray &r, VisualObjectList world)
{
    HitRecord rec;
    float t;

    if(world.hit(r, 0.0 ,MAXFLOAT, rec))
    {
        return 0.5f * vec3(rec.normal.x+1, rec.normal.y+1, rec.normal.z+1);
    }
    /*
    float t;
    t = rayHitSphere(vec3(0,0,-1), 0.5, r);
    if(t > 0.0)
    {
        vec3 N = glm::normalize(r.pointAtParameter(t) - vec3(0, 0, -1));
        return float(0.5) * vec3(N.x+1, N.y+1, N.z+1);
    }
    */

    vec3 unit_direction = glm::normalize(r.direction());
    t = 0.5f * (unit_direction.y + 1.0f);
    return  float(1.0f-t) * vec3(1.0, 1.0, 1.0) + t*vec3(0.5, 0.7, 1.0);
}

float Render::rayHitSphere(const glm::vec3 &center, float radius, const Ray &r)
{
    vec3 oc = r.origin() - center;
    float a = glm::dot(r.direction(), r.direction());
    float b = 2.0f * dot(oc, r.direction());
    float c = dot(oc, oc) - radius*radius;
    float d = b*b - 4*a*c;

    if(d < 0)
        return -1.0;
    else
        return (-b-glm::sqrt(d))/(2.0f*a);
}
