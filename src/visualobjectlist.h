#ifndef VISUALOBJECTLIST_H
#define VISUALOBJECTLIST_H

#include <vector>

#include "renderer/ray.h"
#include "visualobject.h"

class VisualObjectList : public VisualObject
{
public:
    VisualObjectList();
    virtual ~VisualObjectList() {}
    VisualObjectList(std::vector<VisualObject*> l);

    virtual bool hit(const Ray& r, float tmin, float tmax, HitRecord& rec) const;

    std::vector<VisualObject*> objectList;
};

#endif // VISUALOBJECTLIST_H
