#pragma once

/**
 * Inclus�es necess�rias:
 * - map: para armazenar materiais indexados por nome
 * - GL/glew: para fun��es OpenGL modernas
 * - vector: para arrays din�micos de v�rtices e outros dados
 * - string: para manipula��o de nomes e caminhos
 * - glm: biblioteca de matem�tica para computa��o gr�fica
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
  * Os materiais em arquivos MTL cont�m informa��es sobre as propriedades
  * de superf�cie dos objetos 3D, incluindo texturas e propriedades de reflex�o.
  */
struct Material {
    std::string name;              // Nome identificador do material
    std::string diffuseTexPath;    // Caminho do arquivo da textura de cor
    GLuint diffuseTexID = 0;       // Identificador da textura na mem�ria da GPU

    // Coeficientes do modelo de ilumina��o de Phong:
    glm::vec3 ka = glm::vec3(0.2f); // Reflex�o ambiente (luz indireta)
    glm::vec3 kd = glm::vec3(0.8f); // Reflex�o difusa (superf�cies mate)
    glm::vec3 ks = glm::vec3(1.0f); // Reflex�o especular (brilhos)
    float ns = 32.0f;               // Expoente especular (concentra��o do brilho)
};

/**
 * @brief Classe para gerenciamento de modelos 3D no formato OBJ
 *
 * Esta classe � respons�vel por:
 * 1. Carregar modelos 3D do formato OBJ
 * 2. Gerenciar materiais e texturas
 * 3. Renderizar o modelo usando OpenGL moderno
 */
class ObjModel {
public:
    glm::vec3 position = glm::vec3(0.0f);   // Posi��o do modelo no espa�o 3D
    glm::vec3 scale = glm::vec3(1.0f);      // Escala do modelo (1.0 = tamanho original)

    // ... outros membros existentes ...

    // Define a posi��o do modelo (existente)
    void setPosition(const glm::vec3& pos) { position = pos; }

    // Nova fun��o para definir a escala do modelo
    void setScale(const glm::vec3& newScale) { scale = newScale; }

    // Nova fun��o para definir uma escala uniforme (mesmo valor para X, Y e Z)
    void setUniformScale(float uniformScale) { scale = glm::vec3(uniformScale); }


    /**
     * @brief Construtor que carrega um modelo 3D
     * @param path Caminho do arquivo .obj a ser carregado
     */
    ObjModel(const std::string& path);

    /**
     * @brief Renderiza o modelo na cena
     * @param program ID do programa de shader ativo
     * @param view Matriz de visualiza��o da c�mera
     * @param projection Matriz de proje��o
     */
    void render(GLuint program, const glm::mat4& view, const glm::mat4& projection) const;

private:
    /**
     * @brief Carrega e processa um arquivo OBJ
     *
     * Processa:
     * - V�rtices (v)
     * - Normais (vn)
     * - Coordenadas de textura (vt)
     * - Faces (f)
     * - Refer�ncias a materiais
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
     * @brief Configura os buffers do OpenGL para renderiza��o
     *
     * Cria e configura:
     * - VAO (Vertex Array Object)
     * - VBO (Vertex Buffer Object)
     * - Atributos de v�rtices
     */
    void install();

    /**
     * @brief Carrega uma imagem como textura na GPU
     * @param filename Caminho do arquivo de imagem
     * @param texID ID da textura no OpenGL (sa�da)
     */
    void loadTexture(const std::string& filename, GLuint& texID);

    // Dados da geometria do modelo
    std::vector<glm::vec3> vertices;         // Posi��es dos v�rtices
    std::vector<glm::vec3> normals;          // Vetores normais
    std::vector<glm::vec2> texcoords;        // Coordenadas de textura (UV)

    // �ndices para constru��o das faces
    std::vector<unsigned int> vertexIndices;    // �ndices dos v�rtices
    std::vector<unsigned int> texcoordIndices;  // �ndices das coords de textura
    std::vector<unsigned int> normalIndices;    // �ndices das normais

    /**
     * Vetor que combina todos os dados em um formato adequado para o OpenGL
     * Estrutura: [px,py,pz, nx,ny,nz, u,v] para cada v�rtice
     * onde:
     * - px,py,pz: posi��o do v�rtice
     * - nx,ny,nz: normal do v�rtice
     * - u,v: coordenada de textura
     */
    std::vector<float> interleaved;

    // Gerenciamento de materiais
    std::map<std::string, Material> materials;  // Materiais indexados por nome
    std::string currentMaterialName;            // Material atual em uso

    // Identificadores OpenGL
    GLuint VAO;  // Vertex Array Object: configura��o dos atributos
    GLuint VBO;  // Vertex Buffer Object: dados dos v�rtices
};