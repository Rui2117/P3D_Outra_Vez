#include "model.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

/**
 * @brief Construtor da classe ObjModel
 *
 * Este construtor � respons�vel por inicializar um modelo 3D a partir de um arquivo OBJ.
 * O processo � dividido em duas etapas principais:
 * 1. Carregamento do arquivo OBJ (geometria, materiais e texturas)
 * 2. Prepara��o dos dados para renderiza��o no OpenGL
 *
 * @param path Caminho do arquivo OBJ a ser carregado
 */
ObjModel::ObjModel(const std::string& path) {
    loadOBJ(path);    // Carrega todos os dados do arquivo OBJ
    install();        // Configura os buffers do OpenGL para renderiza��o
}

/**
 * @brief Carrega e processa um arquivo OBJ
 *
 * Esta fun��o realiza a leitura linha por linha do arquivo OBJ, interpretando:
 * - v: v�rtices (posi��es 3D)
 * - vt: coordenadas de textura (UV)
 * - vn: normais dos v�rtices
 * - f: faces (tri�ngulos)
 * - mtllib: refer�ncia ao arquivo de materiais
 * - usemtl: especifica��o do material atual
 *
 * @param path Caminho do arquivo OBJ
 */
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

        // Processa v�rtices (x, y, z)
        if (type == "v") {
            glm::vec3 v;
            ss >> v.x >> v.y >> v.z;
            vertices.push_back(v);
        }
        // Processa coordenadas de textura (u, v)
        else if (type == "vt") {
            glm::vec2 vt;
            ss >> vt.x >> vt.y;
            texcoords.push_back(vt);
        }
        // Processa normais (nx, ny, nz)
        else if (type == "vn") {
            glm::vec3 vn;
            ss >> vn.x >> vn.y >> vn.z;
            normals.push_back(vn);
        }
        // Processa faces (tri�ngulos)
        // Formato: f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
        else if (type == "f") {
            std::string v1, v2, v3;
            ss >> v1 >> v2 >> v3;
            std::string verts[] = { v1, v2, v3 };

            // Processa cada v�rtice da face
            for (std::string& vert : verts) {
                std::replace(vert.begin(), vert.end(), '/', ' '); // Substitui '/' por espa�os
                std::istringstream vs(vert);
                unsigned int vi, ti, ni;
                vs >> vi >> ti >> ni;
                // Subtrai 1 pois �ndices no OBJ come�am em 1, mas em C++ come�am em 0
                vertexIndices.push_back(vi - 1);
                texcoordIndices.push_back(ti - 1);
                normalIndices.push_back(ni - 1);
            }
        }
        // Carrega arquivo de materiais (.mtl)
        else if (type == "mtllib") {
            std::string mtlFile;
            ss >> mtlFile;

            // Obt�m o diret�rio base do arquivo OBJ
            size_t lastSlash = path.find_last_of("/\\");
            std::string basePath = (lastSlash != std::string::npos) ? path.substr(0, lastSlash + 1) : "";
            loadMTL(basePath + mtlFile);
        }
        // Define o material atual
        else if (type == "usemtl") {
            ss >> currentMaterialName;
        }
    }
    file.close();

    /**
     * Organiza os dados em um formato intercalado para o OpenGL
     * Estrutura: [px,py,pz, nx,ny,nz, u,v] para cada v�rtice
     * Onde:
     * - px,py,pz: posi��o do v�rtice
     * - nx,ny,nz: normal do v�rtice
     * - u,v: coordenada de textura
     */
    for (size_t i = 0; i < vertexIndices.size(); i++) {
        glm::vec3 v = vertices[vertexIndices[i]];
        glm::vec2 t = texcoords[texcoordIndices[i]];
        glm::vec3 n = normals[normalIndices[i]];

        // Adiciona dados ao vetor intercalado
        interleaved.push_back(v.x); interleaved.push_back(v.y); interleaved.push_back(v.z);  // Posi��o
        interleaved.push_back(n.x); interleaved.push_back(n.y); interleaved.push_back(n.z);  // Normal
        interleaved.push_back(t.x); interleaved.push_back(t.y);                              // Textura
    }
}

/**
 * @brief Configura os buffers do OpenGL para renderiza��o
 *
 * Cria e configura:
 * - VAO (Vertex Array Object): armazena configura��es dos atributos de v�rtice
 * - VBO (Vertex Buffer Object): armazena os dados dos v�rtices
 *
 * Define a estrutura dos dados para o shader:
 * - Atributo 0: posi��o (vec3)
 * - Atributo 1: normal (vec3)
 * - Atributo 2: coordenada de textura (vec2)
 */
void ObjModel::install() {
    // Cria VAO e VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // Configura o VAO e VBO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, interleaved.size() * sizeof(float), interleaved.data(), GL_STATIC_DRAW);

    // Tamanho de um v�rtice completo (3 + 3 + 2 = 8 floats)
    int stride = 8 * sizeof(float);

    // Configura atributos de v�rtice
    // Posi��o (3 floats)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    // Normal (3 floats)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Coordenada de textura (2 floats)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

/**
 * @brief Carrega e processa um arquivo de material (.mtl)
 *
 * Processa as propriedades do material:
 * - Ka: cor ambiente
 * - Kd: cor difusa
 * - Ks: cor especular
 * - Ns: expoente especular
 * - map_Kd: textura difusa
 *
 * @param path Caminho do arquivo MTL
 */
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

            // Constr�i caminho completo da textura
            size_t lastSlash = path.find_last_of("/\\");
            std::string basePath = (lastSlash != std::string::npos) ? path.substr(0, lastSlash + 1) : "";
            std::string texPath = basePath + texFile;

            materials[currentName].diffuseTexPath = texPath;
            loadTexture(texPath, materials[currentName].diffuseTexID);
        }
    }
    file.close();
}

/**
 * @brief Carrega uma imagem de textura para a GPU
 *
 * Utiliza a biblioteca stb_image para carregar a imagem e
 * configura os par�metros da textura no OpenGL
 *
 * @param filename Caminho do arquivo de imagem
 * @param texID ID da textura no OpenGL (sa�da)
 */
void ObjModel::loadTexture(const std::string& filename, GLuint& texID) {
    stbi_set_flip_vertically_on_load(true);  // Inverte a imagem verticalmente (padr�o OpenGL)

    // Carrega a imagem
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
    if (!data) {
        std::cerr << "Erro ao carregar textura: " << filename << std::endl;
        return;
    }

    // Determina o formato baseado no n�mero de canais
    GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

    // Cria e configura a textura no OpenGL
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    // Define os dados da imagem
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Configura par�metros da textura
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);  // Libera a mem�ria da imagem
}

/**
 * @brief Renderiza o modelo 3D
 *
 * Aplica transforma��es de modelo, vista e proje��o,
 * configura materiais/texturas e desenha a geometria
 *
 * @param program ID do programa de shader
 * @param view Matriz de vista
 * @param projection Matriz de proje��o
 */
void ObjModel::render(GLuint program, const glm::mat4& view, const glm::mat4& projection) const {
    glBindVertexArray(VAO);

    // Calcula a matriz de modelo com posi��o E escala
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    model = glm::scale(model, scale);  // Aplica a escala ap�s a transla��o

    // Calcula a matriz MVP final
    glm::mat4 mvp = projection * view * model;

    // Passa a matriz MVP para o shader
    GLint mvpLoc = glGetUniformLocation(program, "MVP");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

    // Aplica textura do material atual, se existir
    auto it = materials.find(currentMaterialName);
    if (it != materials.end() && it->second.diffuseTexID) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, it->second.diffuseTexID);
    }

    // Desenha os tri�ngulos
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertexIndices.size()));
}