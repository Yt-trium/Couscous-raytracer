#ifndef OBJFILEFORMAT_H
#define OBJFILEFORMAT_H

#include "gui/scene.h"

class OBJFileFormat
{
public:
    OBJFileFormat();
    /**
     * @brief readOBJ read a triangulated .obj file
     * (only the vertices and faces as triangles)
     * the method also compute normals
     * @param filename
     * @return
     */
    static SceneOBJ readOBJ(std::string filename);
};

#endif // OBJFILEFORMAT_H
