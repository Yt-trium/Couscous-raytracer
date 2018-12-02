
// Inteface.
#include "filereader.h"

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

            mesh.face_normals.push_back(normal);
        }
    }

    // Compute vertex normals.
    {
        // For each vertex.
        for (size_t v = 0; v < vert_count; ++v)
        {
            vec3 normal(0.0f);
            size_t normal_count = 0;

            // Add normals of attached faces.
            for (size_t i = 0; i < face_count; ++i)
            {
                const size_t ia = mesh.faces.at(i * 3);
                const size_t ib = mesh.faces.at(i * 3 + 1);
                const size_t ic = mesh.faces.at(i * 3 + 2);

                if (ia == v || ib == v || ic == v)
                {
                    normal += mesh.face_normals.at(i);
                    normal_count += 1;
                }
            }

            normal = normal_count > 0 ? normal / static_cast<float>(normal_count) : normal;
            mesh.vertex_normals.push_back(normal);
        }
    }

    // Cleaning.
    reader.close();

    return mesh;
}

