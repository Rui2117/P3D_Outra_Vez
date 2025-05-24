#pragma once

#include <vector>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>

class ObjModel {
public:
    ObjModel(const std::string& path);
    void draw() const;

private:
    void loadOBJ(const std::string& path);
    void setupMesh();

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;

    std::vector<unsigned int> vertexIndices;
    std::vector<unsigned int> texcoordIndices;
    std::vector<unsigned int> normalIndices;

    GLuint VAO, VBO;
    std::vector<float> interleaved;
};