#include "model.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

ObjModel::ObjModel(const std::string& path) {
    loadOBJ(path);   
    install();       
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
        else if (type == "mtllib") {
            std::string mtlFile;
            ss >> mtlFile;

            size_t lastSlash = path.find_last_of("/\\");
            std::string basePath = (lastSlash != std::string::npos) ?
                path.substr(0, lastSlash + 1) : "";
            loadMTL(basePath + mtlFile);
        }
        else if (type == "usemtl") {
            ss >> currentMaterialName;
        }
    }
    file.close();

    for (size_t i = 0; i < vertexIndices.size(); i++) {
        glm::vec3 v = vertices[vertexIndices[i]];
        glm::vec2 t = texcoords[texcoordIndices[i]];
        glm::vec3 n = normals[normalIndices[i]];

        interleaved.push_back(v.x); interleaved.push_back(v.y); interleaved.push_back(v.z);
        interleaved.push_back(n.x); interleaved.push_back(n.y); interleaved.push_back(n.z);
        interleaved.push_back(t.x); interleaved.push_back(t.y);
    }
}

void ObjModel::install() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER,
        interleaved.size() * sizeof(float),
        interleaved.data(),
        GL_STATIC_DRAW);

    int stride = 8 * sizeof(float);

    glVertexAttribPointer(0,  
        3,                    
        GL_FLOAT,             
        GL_FALSE,             
        stride,               
        (void*)0);            
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1,   
        3,                     
        GL_FLOAT,              
        GL_FALSE,              
        stride,                
        (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2,
        2,
        GL_FLOAT,
        GL_FALSE,
        stride,
        (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void ObjModel::loadMTL(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir ficheiro MTL: " << path << std::endl;
        return;
    }

    std::string line;
    std::string currentName;

    while (getline(file, line)) {
        std::istringstream ss(line);
        std::string type;
        ss >> type;

        if (type == "newmtl") {
            ss >> currentName;
            materials[currentName] = Material{ currentName };
        }
        else if (type == "Ka") {
            glm::vec3 ka;
            ss >> ka.r >> ka.g >> ka.b;
            materials[currentName].ka = ka;
        }
        else if (type == "Kd") {
            glm::vec3 kd;
            ss >> kd.r >> kd.g >> kd.b;
            materials[currentName].kd = kd;
        }
        else if (type == "Ks") {
            glm::vec3 ks;
            ss >> ks.r >> ks.g >> ks.b;
            materials[currentName].ks = ks;
        }
        else if (type == "Ns") {
            float ns;
            ss >> ns;
            materials[currentName].ns = ns;
        }
        else if (type == "map_Kd") {
            std::string texFile;
            ss >> texFile;

            size_t lastSlash = path.find_last_of("/\\");
            std::string basePath = (lastSlash != std::string::npos) ?
                path.substr(0, lastSlash + 1) : "";
            std::string texPath = basePath + texFile;

            materials[currentName].diffuseTexPath = texPath;
            loadTexture(texPath, materials[currentName].diffuseTexID);
        }
    }
    file.close();
}

void ObjModel::loadTexture(const std::string& filename, GLuint& texID) {
    stbi_set_flip_vertically_on_load(true);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename.c_str(),
        &width, &height,
        &nrChannels, 0);

    if (!data) {
        std::cerr << "Erro ao carregar textura: " << filename << std::endl;
        return;
    }

    GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexImage2D(GL_TEXTURE_2D,  
        0,                 
        format,           
        width,             
        height,           
        0,                
        format,           
        GL_UNSIGNED_BYTE, 
        data);            

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
}

void ObjModel::render(GLuint program, const glm::mat4& view, const glm::mat4& projection) const {
    glBindVertexArray(VAO);

    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    model = glm::scale(model, scale);

    glm::mat4 mvp = projection * view * model;

    GLint mvpLoc = glGetUniformLocation(program, "MVP");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

    auto it = materials.find(currentMaterialName);
    if (it != materials.end() && it->second.diffuseTexID) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, it->second.diffuseTexID);
    }

    glDrawArrays(GL_TRIANGLES, 0,
        static_cast<GLsizei>(vertexIndices.size()));
}