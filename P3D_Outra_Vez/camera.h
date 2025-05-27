#pragma once

/**
 * Inclusões necessárias:
 * - glm: biblioteca de matemática para computação gráfica
 *   Fornece operações com vetores, matrizes e transformações 3D
 */
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

/**
 * @brief Classe Camera - Sistema de Câmera Orbital 3D
 *
 * Esta classe implementa uma câmera que orbita ao redor de um ponto alvo,
 * similar ao comportamento de câmeras em softwares de modelagem 3D.
 *
 * Características principais:
 * - Rotação orbital ao redor de um ponto
 * - Controle de altura
 * - Zoom através do campo de visão (FOV)
 * - Geração de matrizes de visualização e projeção
 */
class Camera {
public:
    /**
     * @brief Construtor padrão
     *
     * Inicializa a câmera com valores predefinidos:
     * - Posição: (0, 5, 20) - Ligeiramente elevada e afastada
     * - Alvo: (0, 0, 0) - Centro da cena
     * - Vetor UP: (0, 1, 0) - Orientação vertical padrão
     * - FOV: 60 graus - Campo de visão padrão
     */
    Camera() :
        position(0.0f, 5.0f, 20.0f),
        target(0.0f, 0.0f, 0.0f),
        up(0.0f, 1.0f, 0.0f),
        fov(60.0f) {
    }

    /**
     * @brief Construtor personalizado
     *
     * Permite definir todos os parâmetros iniciais da câmera
     *
     * @param position Posição inicial da câmera no espaço 3D
     * @param target Ponto para onde a câmera está apontando
     * @param upDirection Vetor que define a orientação "para cima" da câmera
     * @param fov Campo de visão inicial em graus
     */
    Camera(glm::vec3 position, glm::vec3 target, glm::vec3 upDirection, float fov) :
        position(position), target(target), up(upDirection), fov(fov) {
    }

    /**
     * @brief Gera a matriz de visualização
     *
     * Cria uma matriz que transforma as coordenadas do mundo para
     * coordenadas relativas à posição e orientação da câmera.
     *
     * @return Matriz de visualização 4x4
     */
    mat4 getViewMatrix() const {
        return glm::lookAt(position, target, up);
    }

    /**
     * @brief Gera a matriz de projeção perspectiva
     *
     * Cria uma matriz que transforma coordenadas 3D em coordenadas 2D,
     * aplicando perspectiva (objetos mais distantes aparecem menores).
     *
     * @param aspectRatio Proporção largura/altura da janela
     * @return Matriz de projeção 4x4
     */
    mat4 getProjectionMatrix(float aspectRatio) const {
        return glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 100.0f);
    }

    /**
     * @brief Processa o zoom da câmera
     *
     * Ajusta o campo de visão (FOV) baseado no input do mouse.
     * FOV menor = mais zoom (objetos parecem mais próximos)
     * FOV maior = menos zoom (objetos parecem mais distantes)
     *
     * @param yoffset Quantidade de scroll do mouse
     */
    void processZOOM(double yoffset) {
        fov -= static_cast<float>(yoffset);
        fov = glm::clamp(fov, 15.0f, 90.0f); // Limita FOV entre 15° e 90°
    }

    /**
     * @brief Atualiza a posição da câmera na órbita
     *
     * Calcula a nova posição XYZ baseada no:
     * - Ângulo de órbita (rotação horizontal)
     * - Altura atual
     * - Raio da órbita
     */
    void updatePosition() {
        position.x = orbitRadius * sin(orbitAngle);
        position.y = height;
        position.z = orbitRadius * cos(orbitAngle);
    }

    /**
     * @brief Rotaciona a câmera ao redor do alvo
     *
     * @param degreesToMove Ângulo de rotação a ser aplicado (em radianos)
     */
    void rotateAroundTarget(float degreesToMove) {
        orbitAngle += degreesToMove;
        updatePosition();
    }

    // Atributos públicos da câmera
    glm::vec3 position;   // Posição atual da câmera no espaço 3D
    glm::vec3 up;         // Vetor que define a orientação "para cima"
    float fov;            // Campo de visão em graus

    float orbitAngle = 0.0f;         // Ângulo atual da órbita (em radianos)
    float orbitRadius = 15.0f;       // Distância da câmera ao ponto alvo
    glm::vec3 target = glm::vec3(0.0f); // Ponto central da órbita
    float height = 3.0f;             // Altura da câmera em relação ao alvo
};