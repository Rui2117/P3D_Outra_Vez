#include "model.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

ObjModel::ObjModel(const std::string& path) {
    loadOBJ(path);
    setupMesh();
}

void ObjModel::loadOBJ(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o ficheiro OBJ: " << path << std::endl;
        return;
    }

    std::string line;
    while (getline(file, line)) {
        std::istringstream ss(line);
        std::string type;
        ss >> type;

        if (type == "v") {
            glm::vec3 v;
            ss >> v.x >> v.y >> v.z;
            vertices.push_back(v);
        }
        else if (type == "vt") {
            glm::vec2 vt;
            ss >> vt.x >> vt.y;
            texcoords.push_back(vt);
        }
        else if (type == "vn") {
            glm::vec3 vn;
            ss >> vn.x >> vn.y >> vn.z;
            normals.push_back(vn);
        }
        else if (type == "f") {
            std::string v1, v2, v3;
            ss >> v1 >> v2 >> v3;
            std::string verts[] = { v1, v2, v3 };

            for (std::string& vert : verts) {
                std::replace(vert.begin(), vert.end(), '/', ' ');
                std::istringstream vs(vert);
                unsigned int vi, ti, ni;
                vs >> vi >> ti >> ni;
                vertexIndices.push_back(vi - 1);
                texcoordIndices.push_back(ti - 1);
                normalIndices.push_back(ni - 1);
            }
        }
    }
    file.close();

    // Interleaving
    for (size_t i = 0; i < vertexIndices.size(); i++) {
        glm::vec3 v = vertices[vertexIndices[i]];
        glm::vec2 t = texcoords[texcoordIndices[i]];
        glm::vec3 n = normals[normalIndices[i]];

        interleaved.push_back(v.x);
        interleaved.push_back(v.y);
        interleaved.push_back(v.z);

        interleaved.push_back(n.x);
        interleaved.push_back(n.y);
        interleaved.push_back(n.z);

        interleaved.push_back(t.x);
        interleaved.push_back(t.y);
    }
}

void ObjModel::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, interleaved.size() * sizeof(float), interleaved.data(), GL_STATIC_DRAW);

    int stride = 8 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0); // Position
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float))); // Normal
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float))); // TexCoord
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void ObjModel::draw() const {
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexIndices.size());
}