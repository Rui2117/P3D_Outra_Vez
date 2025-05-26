#pragma once

// Inclui bibliotecas padr�o e OpenGL/GLM necess�rias
#include <map>
#include <GL/glew.h>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>


// Estrutura que representa um material carregado de um ficheiro .mtl
struct Material {
    std::string name;             // Nome do material
    std::string diffuseTexPath;   // Caminho para a textura difusa
    GLuint diffuseTexID = 0;      // ID da textura carregada na GPU
};

// Classe para carregar, armazenar e desenhar modelos OBJ
class ObjModel {
public:
    glm::vec3 position = glm::vec3(0.0f);   // Posi��o do modelo no espa�o 3D
    
    // Construtor: carrega o modelo a partir do ficheiro OBJ
    ObjModel(const std::string& path);

    // Desenha o modelo (usa o VAO e as texturas, se existirem)
    void draw(GLuint program, const glm::mat4& view, const glm::mat4& projection) const;

    // Define a posi��o do modelo (usada para transforma��es)
    void setPosition(const glm::vec3& pos) { position = pos; }
private:
    // Carrega o ficheiro OBJ e preenche os vetores de v�rtices, normais, texturas e �ndices
    void loadOBJ(const std::string& path);

    // Carrega o ficheiro MTL (materiais) associado ao OBJ
    void loadMTL(const std::string& path);

    // Prepara os buffers OpenGL (VAO/VBO) para renderiza��o
    void setupMesh();

    // Carrega uma textura de imagem para a GPU
    void loadTexture(const std::string& filename, GLuint& texID);


	

    // Vetores para armazenar os dados do modelo
    std::vector<glm::vec3> vertices;        // Lista de v�rtices
    std::vector<glm::vec3> normals;         // Lista de normais
    std::vector<glm::vec2> texcoords;       // Lista de coordenadas de textura

    // �ndices para acessar os dados acima (por face)
    std::vector<unsigned int> vertexIndices;
    std::vector<unsigned int> texcoordIndices;
    std::vector<unsigned int> normalIndices;

    // Vetor intercalado para enviar os dados para o VBO (posi��o, normal, texcoord)
    std::vector<float> interleaved;

    // Mapa de materiais carregados do ficheiro MTL
    std::map<std::string, Material> materials;

    // Nome do material atualmente em uso
    std::string currentMaterialName;

    // IDs dos objetos OpenGL para renderiza��o
    GLuint VAO, VBO;
};