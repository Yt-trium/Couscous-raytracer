#include "visualobjectlist.h"

VisualObjectList::VisualObjectList()
{

}

VisualObjectList::VisualObjectList(std::vector<VisualObject*> l)
{
    objectList = l;
}

bool VisualObjectList::hit(const Ray &r, float tmin, float tmax, HitRecord &rec) const
{
    HitRecord tmp_rec;
    bool hit_something = false;
    float closest_so_far = tmax;
    std::size_t i;
    for(i=0;i<objectList.size();i++)
    {
        if(objectList[i]->hit(r, tmin, closest_so_far, tmp_rec))
        {
            hit_something = true;
            closest_so_far = tmp_rec.t;
            rec = tmp_rec;
        }
    }
    return hit_something;
}
