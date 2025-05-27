#pragma once

/**
 * Inclus�es necess�rias:
 * - glm: biblioteca de matem�tica para computa��o gr�fica
 *   Fornece opera��es com vetores, matrizes e transforma��es 3D
 */
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

/**
 * @brief Classe Camera - Sistema de C�mera Orbital 3D
 *
 * Esta classe implementa uma c�mera que orbita ao redor de um ponto alvo,
 * similar ao comportamento de c�meras em softwares de modelagem 3D.
 *
 * Caracter�sticas principais:
 * - Rota��o orbital ao redor de um ponto
 * - Controle de altura
 * - Zoom atrav�s do campo de vis�o (FOV)
 * - Gera��o de matrizes de visualiza��o e proje��o
 */
class Camera {
public:
    /**
     * @brief Construtor padr�o
     *
     * Inicializa a c�mera com valores predefinidos:
     * - Posi��o: (0, 5, 20) - Ligeiramente elevada e afastada
     * - Alvo: (0, 0, 0) - Centro da cena
     * - Vetor UP: (0, 1, 0) - Orienta��o vertical padr�o
     * - FOV: 60 graus - Campo de vis�o padr�o
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
     * Permite definir todos os par�metros iniciais da c�mera
     *
     * @param position Posi��o inicial da c�mera no espa�o 3D
     * @param target Ponto para onde a c�mera est� apontando
     * @param upDirection Vetor que define a orienta��o "para cima" da c�mera
     * @param fov Campo de vis�o inicial em graus
     */
    Camera(glm::vec3 position, glm::vec3 target, glm::vec3 upDirection, float fov) :
        position(position), target(target), up(upDirection), fov(fov) {
    }

    /**
     * @brief Gera a matriz de visualiza��o
     *
     * Cria uma matriz que transforma as coordenadas do mundo para
     * coordenadas relativas � posi��o e orienta��o da c�mera.
     *
     * @return Matriz de visualiza��o 4x4
     */
    mat4 getViewMatrix() const {
        return glm::lookAt(position, target, up);
    }

    /**
     * @brief Gera a matriz de proje��o perspectiva
     *
     * Cria uma matriz que transforma coordenadas 3D em coordenadas 2D,
     * aplicando perspectiva (objetos mais distantes aparecem menores).
     *
     * @param aspectRatio Propor��o largura/altura da janela
     * @return Matriz de proje��o 4x4
     */
    mat4 getProjectionMatrix(float aspectRatio) const {
        return glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 100.0f);
    }

    /**
     * @brief Processa o zoom da c�mera
     *
     * Ajusta o campo de vis�o (FOV) baseado no input do mouse.
     * FOV menor = mais zoom (objetos parecem mais pr�ximos)
     * FOV maior = menos zoom (objetos parecem mais distantes)
     *
     * @param yoffset Quantidade de scroll do mouse
     */
    void processZOOM(double yoffset) {
        fov -= static_cast<float>(yoffset);
        fov = glm::clamp(fov, 15.0f, 90.0f); // Limita FOV entre 15� e 90�
    }

    /**
     * @brief Atualiza a posi��o da c�mera na �rbita
     *
     * Calcula a nova posi��o XYZ baseada no:
     * - �ngulo de �rbita (rota��o horizontal)
     * - Altura atual
     * - Raio da �rbita
     */
    void updatePosition() {
        position.x = orbitRadius * sin(orbitAngle);
        position.y = height;
        position.z = orbitRadius * cos(orbitAngle);
    }

    /**
     * @brief Rotaciona a c�mera ao redor do alvo
     *
     * @param degreesToMove �ngulo de rota��o a ser aplicado (em radianos)
     */
    void rotateAroundTarget(float degreesToMove) {
        orbitAngle += degreesToMove;
        updatePosition();
    }

    // Atributos p�blicos da c�mera
    glm::vec3 position;   // Posi��o atual da c�mera no espa�o 3D
    glm::vec3 up;         // Vetor que define a orienta��o "para cima"
    float fov;            // Campo de vis�o em graus

    float orbitAngle = 0.0f;         // �ngulo atual da �rbita (em radianos)
    float orbitRadius = 15.0f;       // Dist�ncia da c�mera ao ponto alvo
    glm::vec3 target = glm::vec3(0.0f); // Ponto central da �rbita
    float height = 3.0f;             // Altura da c�mera em rela��o ao alvo
};