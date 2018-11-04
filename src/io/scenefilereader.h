#ifndef IO_SCENEFILEREADER_H
#define IO_SCENEFILEREADER_H

#include "gui/scene.h"

#include <string>

/**
 * @brief read a triangulated .obj file
 * (only the vertices and faces as triangles)
 * the method also compute normals
 * @param filename
 * @return
 */
SceneObjectFile read_obj(const std::string& filename);

#endif // IO_SCENEFILEREADER_H
