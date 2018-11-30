
#ifndef IO_FILEREADER_H
#define IO_FILEREADER_H

// glm includes.
#include <glm/glm.hpp>

// Standard includes.
#include <cstddef>
#include <string>
#include <vector>

typedef struct MeshOffFile {
    std::vector<glm::vec3> vertices;
    std::vector<size_t> faces;
    std::vector<glm::vec3> normals;
} MeshOffFile;

MeshOffFile read_off(const std::string& filename);

#endif // IO_FILEREADER_H
