#include "objfileformat.h"

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
                    float x, y, z;

                    getline(lineStream, line, ' ');
                    x = stof(line);
                    getline(lineStream, line, ' ');
                    y = stof(line);
                    getline(lineStream, line, ' ');
                    z = stof(line);

                    obj.vertices.push_back(vec3(x, y, z));
                }
                if(line.compare("f") == 0)
                {
                    size_t a, b, c;

                    getline(lineStream, line, ' ');
                    a = size_t(stoi(line)-1);
                    getline(lineStream, line, ' ');
                    b = size_t(stoi(line)-1);
                    getline(lineStream, line, ' ');
                    c = size_t(stoi(line)-1);

                    obj.triangles.push_back(a);
                    obj.triangles.push_back(b);
                    obj.triangles.push_back(c);

                    vec3 n = cross((obj.vertices.at(b)-obj.vertices.at(a)),
                                   (obj.vertices.at(c)-obj.vertices.at(a)));

                    obj.normals.push_back(n);
                }
            }
        }
        objfile.close();
    }

    return obj;
}
