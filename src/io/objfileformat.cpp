#include "objfileformat.h"

#include <QMessageBox>
#include <QString>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

using namespace std;
using namespace glm;

OBJFileFormat::OBJFileFormat()
{

}

SceneOBJ OBJFileFormat::readOBJ(std::string filename)
{
    SceneOBJ obj;
    string line;
    ifstream objfile (filename);

    if(objfile.is_open())
    {
        while(getline(objfile, line, '\n'))
        {
            stringstream lineStream(line);
            while(getline(lineStream, line, ' '))
            {
                if(line.compare("v") == 0)
                {
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
                    getline(lineStream, line, ' ');
                    const size_t a = static_cast<size_t>(stoi(line) - 1);
                    getline(lineStream, line, ' ');
                    const size_t b = static_cast<size_t>(stoi(line) - 1);
                    getline(lineStream, line, ' ');
                    const size_t c = static_cast<size_t>(stoi(line) - 1);

                    obj.triangles.push_back(a);
                    obj.triangles.push_back(b);
                    obj.triangles.push_back(c);

                    assert(a < obj.vertices.size());
                    assert(b < obj.vertices.size());
                    assert(c < obj.vertices.size());

                    vec3 n =
                        cross(
                            (obj.vertices.at(b) - obj.vertices.at(a)),
                            (obj.vertices.at(c) - obj.vertices.at(a)));

                    obj.normals.push_back(n);
                }
            }

            obj.m_name = filename;
            obj.m_type = OBJ;
        }

        objfile.close();
    }
    else
    {
        QMessageBox::critical(nullptr, "ERREUR", "ERREUR : Impossible d'ouvrir le fichier : " + QString::fromStdString(filename));
    }

    return obj;
}

