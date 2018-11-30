
// Inteface.
#include "scenefilereader.h"

// Qt includes.
#include <QMessageBox>

// Standard includes.
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
using namespace glm;

MeshOffFile read_off(const std::string& filename)
{
    MeshOffFile mesh;
    ifstream reader(filename);

    // Can we read the file ?
    if(!reader.is_open())
    {
        QMessageBox::critical(
            nullptr,
            "ERREUR",
            "ERREUR : Impossible d'ouvrir le fichier : " + QString::fromStdString(filename));

        return mesh;
    }

    string line; // reading buffer

    // Reading variables
    std::string reader_str;
    int reader_int;

    // Check file type
    reader >> reader_str;
    if(reader_str.compare("OFF"))
    {
        QMessageBox::critical(
            nullptr,
            "ERREUR",
            "ERREUR : Le fichier n'est pas de type OFF : " + QString::fromStdString(filename));

        return mesh;
    }

    size_t vert_count, face_count;
    reader >> vert_count >> face_count >> reader_int; // line ends with 0 (number of edges)

    // Read vertices.
    {
        float x, y, z;
        for (size_t i = 0; i < vert_count; ++i)
        {
            reader >> x >> y >> z;
            mesh.vertices.push_back(vec3(x, y, z));
        }
    }

    // Read faces.
    {
        size_t ia, ib, ic;
        for (size_t i = 0; i < face_count; ++i)
        {
            reader >> reader_int >> ia >> ib >> ic;
            // We only support triangles.
            assert(reader_int == 3);

            mesh.faces.push_back(ia);
            mesh.faces.push_back(ib);
            mesh.faces.push_back(ic);

            vec3 normal = cross(
                (mesh.vertices.at(ib) - mesh.vertices.at(ia)),
                (mesh.vertices.at(ic) - mesh.vertices.at(ia)));

            mesh.normals.push_back(normal);
        }
    }

    // Cleaning.
    reader.close();

    return mesh;
}

