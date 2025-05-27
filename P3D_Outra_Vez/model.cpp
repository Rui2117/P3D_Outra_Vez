/***********************************************************************
 * Implementação do Sistema de Carregamento e Renderização de Modelos 3D
 *
 * Este arquivo implementa um sistema completo para carregar modelos 3D
 * no formato OBJ, incluindo:
 * - Carregamento de geometria (vértices, normais, coordenadas UV)
 * - Suporte a materiais e texturas
 * - Sistema de renderização otimizado com OpenGL moderno
 ***********************************************************************/

#include "model.h"
#define STB_IMAGE_IMPLEMENTATION  // Necessário para implementação da biblioteca stb_image
#include "stb_image.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

 /**
  * @brief Construtor da classe ObjModel
  *
  * Inicializa um modelo 3D a partir de um arquivo OBJ em duas etapas:
  * 1. Carrega os dados geométricos e materiais do arquivo
  * 2. Prepara os buffers do OpenGL para renderização eficiente
  *
  * @param path Caminho completo para o arquivo .obj
  */
ObjModel::ObjModel(const std::string& path) {
    loadOBJ(path);    // Carrega os dados do arquivo
    install();        // Configura os buffers do OpenGL
}

/**
 * @brief Carrega e processa um arquivo OBJ
 *
 * Realiza a leitura e interpretação do arquivo OBJ linha por linha.
 * Processa os seguintes elementos:
 * - v: Vértices (coordenadas x, y, z no espaço 3D)
 * - vt: Coordenadas de textura (u, v para mapeamento 2D)
 * - vn: Normais dos vértices (direção perpendicular à superfície)
 * - f: Faces (triângulos definidos por índices)
 * - mtllib: Arquivo de materiais associado
 * - usemtl: Seleção do material atual
 *
 * @param path Caminho do arquivo OBJ a ser carregado
 */
void ObjModel::loadOBJ(const std::string& path) {
    // Abre o arquivo OBJ
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o ficheiro OBJ: " << path << std::endl;
        return;
    }

    // Processa o arquivo linha por linha
    std::string line;
    while (getline(file, line)) {
        std::istringstream ss(line);
        std::string type;
        ss >> type;

        // Processa vértices (posições 3D)
        if (type == "v") {
            glm::vec3 v;
            ss >> v.x >> v.y >> v.z;
            vertices.push_back(v);
        }
        // Processa coordenadas de textura (UV)
        else if (type == "vt") {
            glm::vec2 vt;
            ss >> vt.x >> vt.y;
            texcoords.push_back(vt);
        }
        // Processa normais dos vértices
        else if (type == "vn") {
            glm::vec3 vn;
            ss >> vn.x >> vn.y >> vn.z;
            normals.push_back(vn);
        }
        // Processa faces (triângulos)
        else if (type == "f") {
            std::string v1, v2, v3;
            ss >> v1 >> v2 >> v3;
            std::string verts[] = { v1, v2, v3 };

            // Processa cada vértice da face (formato: v/vt/vn)
            for (std::string& vert : verts) {
                // Converte separadores '/' em espaços para facilitar a leitura
                std::replace(vert.begin(), vert.end(), '/', ' ');
                std::istringstream vs(vert);
                unsigned int vi, ti, ni;
                vs >> vi >> ti >> ni;
                // Ajusta índices (OBJ usa base 1, C++ usa base 0)
                vertexIndices.push_back(vi - 1);
                texcoordIndices.push_back(ti - 1);
                normalIndices.push_back(ni - 1);
            }
        }
        // Carrega arquivo de materiais
        else if (type == "mtllib") {
            std::string mtlFile;
            ss >> mtlFile;

            // Obtém o diretório base do OBJ para localizar o arquivo MTL
            size_t lastSlash = path.find_last_of("/\\");
            std::string basePath = (lastSlash != std::string::npos) ?
                path.substr(0, lastSlash + 1) : "";
            loadMTL(basePath + mtlFile);
        }
        // Define o material atual
        else if (type == "usemtl") {
            ss >> currentMaterialName;
        }
    }
    file.close();

    // Organiza os dados em formato intercalado para o OpenGL
    // Cada vértice terá: [posição(xyz), normal(xyz), texcoord(uv)]
    for (size_t i = 0; i < vertexIndices.size(); i++) {
        glm::vec3 v = vertices[vertexIndices[i]];
        glm::vec2 t = texcoords[texcoordIndices[i]];
        glm::vec3 n = normals[normalIndices[i]];

        // Adiciona os dados ao buffer intercalado
        interleaved.push_back(v.x); interleaved.push_back(v.y); interleaved.push_back(v.z);
        interleaved.push_back(n.x); interleaved.push_back(n.y); interleaved.push_back(n.z);
        interleaved.push_back(t.x); interleaved.push_back(t.y);
    }
}

/**
 * @brief Configura os buffers do OpenGL para renderização
 *
 * Esta função é responsável por:
 * 1. Criar e configurar o VAO (Vertex Array Object)
 * 2. Criar e preencher o VBO (Vertex Buffer Object)
 * 3. Definir o layout dos atributos de vértice para o shader
 *
 * Layout dos dados no buffer:
 * [px,py,pz, nx,ny,nz, u,v] - 8 floats por vértice
 * - Posição (xyz): 3 floats, offset 0
 * - Normal (xyz): 3 floats, offset 3
 * - Textura (uv): 2 floats, offset 6
 */
void ObjModel::install() {
    // Cria os objetos OpenGL necessários
    glGenVertexArrays(1, &VAO);  // Cria um Vertex Array Object
    glGenBuffers(1, &VBO);       // Cria um Vertex Buffer Object

    // Ativa o VAO para configuração
    glBindVertexArray(VAO);

    // Configura o buffer de vértices
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Carrega os dados intercalados no buffer
    glBufferData(GL_ARRAY_BUFFER,
        interleaved.size() * sizeof(float),
        interleaved.data(),
        GL_STATIC_DRAW);

    // Define o tamanho de um vértice completo (8 floats)
    int stride = 8 * sizeof(float);

    // Configura o atributo de posição (location = 0)
    glVertexAttribPointer(0,                    // Índice do atributo
        3,                      // Número de componentes (xyz)
        GL_FLOAT,              // Tipo dos dados
        GL_FALSE,              // Não normalizar
        stride,                // Bytes entre vértices
        (void*)0);             // Offset do primeiro componente
    glEnableVertexAttribArray(0);

    // Configura o atributo de normal (location = 1)
    glVertexAttribPointer(1,                    // Índice do atributo
        3,                      // Número de componentes (xyz)
        GL_FLOAT,              // Tipo dos dados
        GL_FALSE,              // Não normalizar
        stride,                // Bytes entre vértices
        (void*)(3 * sizeof(float))); // Offset após posição
    glEnableVertexAttribArray(1);

    // Configura o atributo de textura (location = 2)
    glVertexAttribPointer(2,                    // Índice do atributo
        2,                      // Número de componentes (uv)
        GL_FLOAT,              // Tipo dos dados
        GL_FALSE,              // Não normalizar
        stride,                // Bytes entre vértices
        (void*)(6 * sizeof(float))); // Offset após normal
    glEnableVertexAttribArray(2);

    // Desvincula o VAO para evitar modificações acidentais
    glBindVertexArray(0);
}

/**
 * @brief Carrega e processa um arquivo de material (.mtl)
 *
 * Esta função:
 * 1. Lê as propriedades dos materiais (cores, brilho, etc.)
 * 2. Carrega texturas associadas aos materiais
 * 3. Armazena as informações para uso durante a renderização
 *
 * Propriedades do material:
 * - Ka: Cor ambiente (RGB)
 * - Kd: Cor difusa (RGB)
 * - Ks: Cor especular (RGB)
 * - Ns: Expoente especular (brilho)
 * - map_Kd: Caminho da textura difusa
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
    std::string currentName;  // Nome do material atual sendo processado

    while (getline(file, line)) {
        std::istringstream ss(line);
        std::string type;
        ss >> type;

        // Inicia novo material
        if (type == "newmtl") {
            ss >> currentName;
            // Cria novo material com nome padrão
            materials[currentName] = Material{ currentName };
        }
        // Processa cor ambiente (Ka)
        else if (type == "Ka") {
            glm::vec3 ka;
            ss >> ka.r >> ka.g >> ka.b;
            materials[currentName].ka = ka;
        }
        // Processa cor difusa (Kd)
        else if (type == "Kd") {
            glm::vec3 kd;
            ss >> kd.r >> kd.g >> kd.b;
            materials[currentName].kd = kd;
        }
        // Processa cor especular (Ks)
        else if (type == "Ks") {
            glm::vec3 ks;
            ss >> ks.r >> ks.g >> ks.b;
            materials[currentName].ks = ks;
        }
        // Processa expoente especular (Ns)
        else if (type == "Ns") {
            float ns;
            ss >> ns;
            materials[currentName].ns = ns;
        }
        // Processa textura difusa (map_Kd)
        else if (type == "map_Kd") {
            std::string texFile;
            ss >> texFile;

            // Constrói caminho completo para a textura
            size_t lastSlash = path.find_last_of("/\\");
            std::string basePath = (lastSlash != std::string::npos) ?
                path.substr(0, lastSlash + 1) : "";
            std::string texPath = basePath + texFile;

            // Armazena caminho e carrega a textura
            materials[currentName].diffuseTexPath = texPath;
            loadTexture(texPath, materials[currentName].diffuseTexID);
        }
    }
    file.close();
}

/**
 * @brief Carrega uma imagem de textura para a GPU
 *
 * Esta função:
 * 1. Carrega a imagem do disco usando stb_image
 * 2. Cria e configura uma textura OpenGL
 * 3. Configura parâmetros de filtragem e repetição
 *
 * @param filename Caminho do arquivo de imagem
 * @param texID Identificador da textura OpenGL (saída)
 */
void ObjModel::loadTexture(const std::string& filename, GLuint& texID) {
    // Inverte a imagem verticalmente (padrão OpenGL)
    stbi_set_flip_vertically_on_load(true);

    // Carrega a imagem do disco
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename.c_str(),
        &width, &height,
        &nrChannels, 0);

    if (!data) {
        std::cerr << "Erro ao carregar textura: " << filename << std::endl;
        return;
    }

    // Determina o formato baseado no número de canais
    // RGB: 3 canais, RGBA: 4 canais
    GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

    // Cria e configura a textura OpenGL
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    // Carrega os dados da imagem para a GPU
    glTexImage2D(GL_TEXTURE_2D,    // Tipo de textura
        0,                  // Nível de mipmap
        format,            // Formato interno
        width,             // Largura
        height,            // Altura
        0,                 // Borda (sempre 0)
        format,            // Formato dos dados
        GL_UNSIGNED_BYTE,  // Tipo dos dados
        data);            // Ponteiro para os dados

    // Gera mipmaps automaticamente
    glGenerateMipmap(GL_TEXTURE_2D);

    // Configura parâmetros de filtragem e repetição
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Libera a memória da imagem
    stbi_image_free(data);
}

/**
 * @brief Renderiza o modelo 3D
 *
 * Esta função:
 * 1. Aplica transformações de modelo (posição e escala)
 * 2. Calcula e envia a matriz MVP para o shader
 * 3. Configura texturas e materiais
 * 4. Executa o comando de desenho
 *
 * @param program ID do programa shader
 * @param view Matriz de visualização da câmera
 * @param projection Matriz de projeção
 */
void ObjModel::render(GLuint program, const glm::mat4& view, const glm::mat4& projection) const {
    // Ativa o VAO do modelo
    glBindVertexArray(VAO);

    // Calcula a matriz de modelo com transformações
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    model = glm::scale(model, scale);  // Aplica escala após translação

    // Calcula a matriz MVP final
    glm::mat4 mvp = projection * view * model;

    // Envia a matriz MVP para o shader
    GLint mvpLoc = glGetUniformLocation(program, "MVP");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

    // Configura a textura do material atual
    auto it = materials.find(currentMaterialName);
    if (it != materials.end() && it->second.diffuseTexID) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, it->second.diffuseTexID);
    }

    // Desenha o modelo usando triângulos
    glDrawArrays(GL_TRIANGLES, 0,
        static_cast<GLsizei>(vertexIndices.size()));
}