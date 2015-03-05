/*
 * Copyright (C) 2015 Emmanuel Durand
 *
 * This file is part of Shader-0-matic.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * blobserver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with blobserver.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * @meshLoader.h
 * A simple OBJ loader
 */

#ifndef SPLASH_LOADER_OBJ_H
#define SPLASH_LOADER_OBJ_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace Loader
{

/**********/
class Base
{
    public:
        virtual ~Base() {};

        virtual bool load(std::string filename) = 0;
        virtual std::vector<glm::vec4> getVertices() const = 0;
        virtual std::vector<glm::vec2> getUVs() const = 0;
        virtual std::vector<glm::vec3> getNormals() const = 0;
        virtual std::vector<std::vector<int>> getFaces() const = 0;
};

/**********/
class Obj : public Base
{
    public:
        ~Obj() {};

        /**/
        bool load(std::string filename)
        {
            std::ifstream file(filename, std::ios::in);
            if (!file.is_open())
                return false;

            _vertices.clear();
            _uvs.clear();
            _normals.clear();
            _faces.clear();

            // All objects are converted to a single one.
            // This flag keeps track of the objects
            int vertexShift = 0;
            int uvShift = 0;
            int normalShift = 0;

            for (std::string line; std::getline(file, line);)
            {
                
                std::string::size_type pos;
                if ((pos = line.find("o ")) == 0)
                {
                    vertexShift = _vertices.size();
                    uvShift = _uvs.size();
                    normalShift = _normals.size();
                }
                else if ((pos = line.find("v ")) == 0)
                {
                    pos += 1;
                    glm::vec4 vertex;
                    int index = 0;
                    do
                    {
                        pos++;
                        line = line.substr(pos);
                        vertex[index] = std::stof(line);
                        index++;
                        pos = line.find(" ");
                    } while (pos != std::string::npos && index < 4);

                    if (index < 3)
                        vertex[3] = 1.f;

                    _vertices.push_back(vertex);
                }
                else if ((pos = line.find("vt ")) == 0)
                {
                    pos += 2;
                    glm::vec2 uv;
                    int index = 0;
                    do
                    {
                        pos++;
                        line = line.substr(pos);
                        uv[index] = std::stof(line);
                        index++;
                        pos = line.find(" ");
                    } while (pos != std::string::npos && index < 2);

                    _uvs.push_back(uv);
                }
                else if ((pos = line.find("vn ")) == 0)
                {
                    pos += 2;
                    glm::vec3 normal;
                    int index = 0;
                    do
                    {
                        pos++;
                        line = line.substr(pos);
                        normal[index] = std::stof(line);
                        index++;
                        pos = line.find(" ");
                    } while (pos != std::string::npos && index < 3);

                    _normals.push_back(normal);
                }
                else if ((pos = line.find("f ")) == 0)
                {
                    pos += 1;

                    std::vector<FaceVertex> face;
                    std::string::size_type nextSlash, nextSpace;
                    do
                    {
                        pos++;
                        line = line.substr(pos);
                        nextSpace = line.find(" ");

                        FaceVertex faceVertex;
                        faceVertex.vertexId = std::stoi(line) - 1 + vertexShift;

                        nextSlash = line.find("/");
                        if (nextSlash != std::string::npos && (nextSpace == std::string::npos || nextSlash < nextSpace))
                        {
                            line = line.substr(nextSlash + 1);
                            nextSpace = line.find(" ");
                            faceVertex.uvId = std::stoi(line) - 1 + uvShift;
                        }
                        nextSlash = line.find("/");
                        if (nextSlash != std::string::npos && (nextSpace == std::string::npos || nextSlash < nextSpace))
                        {
                            line = line.substr(nextSlash + 1);
                            nextSpace = line.find(" ");
                            faceVertex.normalId = std::stoi(line) - 1 + normalShift;
                        }

                        face.push_back(faceVertex);

                        pos = nextSpace;
                    } while (pos != std::string::npos);

                    // We triangulate faces right away if needed
                    // Only tris and quads are supported
                    if (face.size() == 3)
                    {
                        _faces.push_back(face);
                    }
                    else if (face.size() >= 4)
                    {
                        std::vector<FaceVertex> newFace;
                        newFace.push_back(face[0]);
                        newFace.push_back(face[1]);
                        newFace.push_back(face[2]);
                        _faces.push_back(newFace);

                        newFace.clear();
                        newFace.push_back(face[2]);
                        newFace.push_back(face[3]);
                        newFace.push_back(face[0]);
                        _faces.push_back(newFace);
                    }
                }
            }

            return true;
        }

        /**/
        std::vector<glm::vec4> getVertices() const
        {
            std::vector<glm::vec4> vertices;

            for (auto& face : _faces)
            {
                vertices.push_back(_vertices[face[0].vertexId]);
                vertices.push_back(_vertices[face[1].vertexId]);
                vertices.push_back(_vertices[face[2].vertexId]);
            }

            return vertices;
        }

        /**/
        std::vector<glm::vec2> getUVs() const
        {
            std::vector<glm::vec2> uvs;

            for (auto& face : _faces)
            {
                if (face[0].uvId == -1)
                    for (auto& v : face)
                        uvs.push_back(glm::vec2(0.f, 0.f));
                else
                {
                    uvs.push_back(_uvs[face[0].uvId]);
                    uvs.push_back(_uvs[face[1].uvId]);
                    uvs.push_back(_uvs[face[2].uvId]);
                }
            }

            return uvs;
        }

        /**/
        std::vector<glm::vec3> getNormals() const
        {
            std::vector<glm::vec3> normals;

            for (auto& face : _faces)
            {
                if (face[0].normalId == -1)
                    for (auto& v : face)
                        normals.push_back(glm::vec3(0.f, 0.f, 0.f));
                else
                {
                    normals.push_back(_normals[face[0].normalId]);
                    normals.push_back(_normals[face[1].normalId]);
                    normals.push_back(_normals[face[2].normalId]);
                }
            }

            return normals;
        }

        /**/
        std::vector<std::vector<int>> getFaces() const
        {
            return std::vector<std::vector<int>>();
        }

    private:
        std::vector<glm::vec4> _vertices;
        std::vector<glm::vec2> _uvs;
        std::vector<glm::vec3> _normals;

        struct FaceVertex
        {
            int vertexId {-1};
            int uvId {-1};
            int normalId {-1};
        };
        std::vector<std::vector<FaceVertex>> _faces;
};

} // end of namespace

#endif
