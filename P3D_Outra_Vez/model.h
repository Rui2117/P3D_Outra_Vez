#pragma once

#include <map>
#include <GL/glew.h>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Material {
    std::string name;             
    std::string diffuseTexPath;   
    GLuint diffuseTexID = 0;      

    glm::vec3 ka = glm::vec3(0.2f); 
    glm::vec3 kd = glm::vec3(0.8f); 
    glm::vec3 ks = glm::vec3(1.0f); 
    float ns = 32.0f;               
};

class ObjModel {
public:
    glm::vec3 position = glm::vec3(0.0f);   
    glm::vec3 scale = glm::vec3(1.0f);      
    glm::vec3 rotation = glm::vec3(0.0f, 90.0f, 0.0f);

    void setPosition(const glm::vec3& pos) { position = pos; }

    void setScale(const glm::vec3& newScale) { scale = newScale; }

	void setRotation(const glm::vec3& rot) { rotation = rot; }

    void setUniformScale(float uniformScale) { scale = glm::vec3(uniformScale); }

    ObjModel(const std::string& path);

    void render(GLuint program, const glm::mat4& view, const glm::mat4& projection) const;

private:
    void loadOBJ(const std::string& path);

    void loadMTL(const std::string& path);

    void install();

    void loadTexture(const std::string& filename, GLuint& texID);

    std::vector<glm::vec3> vertices;       
    std::vector<glm::vec3> normals;        
    std::vector<glm::vec2> texcoords;      

    std::vector<unsigned int> vertexIndices;   
    std::vector<unsigned int> texcoordIndices; 
    std::vector<unsigned int> normalIndices;   

    std::vector<float> interleaved;

    std::map<std::string, Material> materials;  
    std::string currentMaterialName;          

    GLuint VAO; 
    GLuint VBO;  
};