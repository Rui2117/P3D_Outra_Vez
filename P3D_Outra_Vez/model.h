#pragma once

/**
 * Inclusões necessárias:
 * - map: para armazenar materiais indexados por nome
 * - GL/glew: para funções OpenGL modernas
 * - vector: para arrays dinâmicos de vértices e outros dados
 * - string: para manipulação de nomes e caminhos
 * - glm: biblioteca de matemática para computação gráfica
 */
#include <map>
#include <GL/glew.h>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

 /**
  * @brief Estrutura que representa um material carregado de um arquivo .mtl
  *
  * Os materiais em arquivos MTL contêm informações sobre as propriedades
  * de superfície dos objetos 3D, incluindo texturas e propriedades de reflexão.
  */
struct Material {
    std::string name;              // Nome identificador do material
    std::string diffuseTexPath;    // Caminho do arquivo da textura de cor
    GLuint diffuseTexID = 0;       // Identificador da textura na memória da GPU

    // Coeficientes do modelo de iluminação de Phong:
    glm::vec3 ka = glm::vec3(0.2f); // Reflexão ambiente (luz indireta)
    glm::vec3 kd = glm::vec3(0.8f); // Reflexão difusa (superfícies mate)
    glm::vec3 ks = glm::vec3(1.0f); // Reflexão especular (brilhos)
    float ns = 32.0f;               // Expoente especular (concentração do brilho)
};

/**
 * @brief Classe para gerenciamento de modelos 3D no formato OBJ
 *
 * Esta classe é responsável por:
 * 1. Carregar modelos 3D do formato OBJ
 * 2. Gerenciar materiais e texturas
 * 3. Renderizar o modelo usando OpenGL moderno
 */
class ObjModel {
public:
    glm::vec3 position = glm::vec3(0.0f);   // Posição do modelo no espaço 3D
    glm::vec3 scale = glm::vec3(1.0f);      // Escala do modelo (1.0 = tamanho original)

    // ... outros membros existentes ...

    // Define a posição do modelo (existente)
    void setPosition(const glm::vec3& pos) { position = pos; }

    // Nova função para definir a escala do modelo
    void setScale(const glm::vec3& newScale) { scale = newScale; }

    // Nova função para definir uma escala uniforme (mesmo valor para X, Y e Z)
    void setUniformScale(float uniformScale) { scale = glm::vec3(uniformScale); }


    /**
     * @brief Construtor que carrega um modelo 3D
     * @param path Caminho do arquivo .obj a ser carregado
     */
    ObjModel(const std::string& path);

    /**
     * @brief Renderiza o modelo na cena
     * @param program ID do programa de shader ativo
     * @param view Matriz de visualização da câmera
     * @param projection Matriz de projeção
     */
    void render(GLuint program, const glm::mat4& view, const glm::mat4& projection) const;

private:
    /**
     * @brief Carrega e processa um arquivo OBJ
     *
     * Processa:
     * - Vértices (v)
     * - Normais (vn)
     * - Coordenadas de textura (vt)
     * - Faces (f)
     * - Referências a materiais
     */
    void loadOBJ(const std::string& path);

    /**
     * @brief Carrega e processa um arquivo MTL de materiais
     *
     * Processa propriedades como:
     * - Cores (ambiente, difusa, especular)
     * - Texturas
     * - Brilho
     */
    void loadMTL(const std::string& path);

    /**
     * @brief Configura os buffers do OpenGL para renderização
     *
     * Cria e configura:
     * - VAO (Vertex Array Object)
     * - VBO (Vertex Buffer Object)
     * - Atributos de vértices
     */
    void install();

    /**
     * @brief Carrega uma imagem como textura na GPU
     * @param filename Caminho do arquivo de imagem
     * @param texID ID da textura no OpenGL (saída)
     */
    void loadTexture(const std::string& filename, GLuint& texID);

    // Dados da geometria do modelo
    std::vector<glm::vec3> vertices;         // Posições dos vértices
    std::vector<glm::vec3> normals;          // Vetores normais
    std::vector<glm::vec2> texcoords;        // Coordenadas de textura (UV)

    // Índices para construção das faces
    std::vector<unsigned int> vertexIndices;    // Índices dos vértices
    std::vector<unsigned int> texcoordIndices;  // Índices das coords de textura
    std::vector<unsigned int> normalIndices;    // Índices das normais

    /**
     * Vetor que combina todos os dados em um formato adequado para o OpenGL
     * Estrutura: [px,py,pz, nx,ny,nz, u,v] para cada vértice
     * onde:
     * - px,py,pz: posição do vértice
     * - nx,ny,nz: normal do vértice
     * - u,v: coordenada de textura
     */
    std::vector<float> interleaved;

    // Gerenciamento de materiais
    std::map<std::string, Material> materials;  // Materiais indexados por nome
    std::string currentMaterialName;            // Material atual em uso

    // Identificadores OpenGL
    GLuint VAO;  // Vertex Array Object: configuração dos atributos
    GLuint VBO;  // Vertex Buffer Object: dados dos vértices
};