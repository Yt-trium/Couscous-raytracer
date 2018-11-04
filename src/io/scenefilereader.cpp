#include "scenefilereader.h"

#include <QMessageBox>

#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
using namespace glm;

SceneObjectFile read_obj(const std::string& filename)
{
    SceneObjectFile obj;
    ifstream objfile(filename);

    // Can we read the file ?
    if(!objfile.is_open())
    {
        QMessageBox::critical(nullptr, "ERREUR", "ERREUR : Impossible d'ouvrir le fichier : " + QString::fromStdString(filename));
        return obj;
    }

    string line; // reading buffer

    while (getline(objfile, line, '\n'))
    {
        stringstream lineStream(line);

        while (getline(lineStream, line, ' '))
        {
            if(line.compare("v") == 0)
            {
                // Read a 3D point.
                getline(lineStream, line, ' ');
                const float x = stof(line);
                getline(lineStream, line, ' ');
                const float y = stof(line);
                getline(lineStream, line, ' ');
                const float z = stof(line);

                obj.vertices.push_back(vec3(x, y, z));
            }
            else if(line.compare("f") == 0)
            {
                // Read a face.
                getline(lineStream, line, ' ');
                const size_t a = static_cast<size_t>(stoi(line) - 1);
                getline(lineStream, line, ' ');
                const size_t b = static_cast<size_t>(stoi(line) - 1);
                getline(lineStream, line, ' ');
                const size_t c = static_cast<size_t>(stoi(line) - 1);

                obj.triangles.push_back(a);
                obj.triangles.push_back(b);
                obj.triangles.push_back(c);

                vec3 n = cross(
                    (obj.vertices.at(b) - obj.vertices.at(a)),
                    (obj.vertices.at(c) - obj.vertices.at(a)));

                obj.normals.push_back(n);
            }
        }

        obj.name = filename;
        obj.type = ObjectFileType::OBJ;
    }

    objfile.close();

    return obj;
}

