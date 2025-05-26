#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

// Classe que representa uma câmara 3D orbitando um alvo
class Camera
{
public:
    // Construtor padrão: inicializa a posição, alvo, vetor up e campo de visão (fov)
    Camera() : position(0.0f, 5.0f, 20.0f), target(0.0f, 0.0f, 0.0f), up(0.0f, 1.0f, 0.0f), fov(60.0f)
    {
    }

    // Construtor parametrizado: permite definir posição, alvo, up e fov
    Camera(glm::vec3 position, glm::vec3 target, glm::vec3 upDirection, float fov) :
        position(position), target(target), up(upDirection), fov(fov) {
    }

    // Retorna a matriz de visão (view), usada para transformar o mundo para a perspetiva da câmara
    mat4 getViewMatrix() const {
        return glm::lookAt(position, target, up);
    }

    // Retorna a matriz de projeção (perspetiva), usada para projetar 3D em 2D
    mat4 getProjectionMatrix(float aspectRatio) const {
        return glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 100.0f);
    }

    // Processa o zoom alterando o campo de visão (fov), limitado entre 15 e 90 graus
    void processZOOM(double yoffset) {
        fov -= static_cast<float>(yoffset);
        fov = glm::clamp(fov, 15.0f, 90.0f);
    }

    // Atualiza a posição da câmara com base no ângulo de órbita e altura
    void updatePosition() {
        position.x = orbitRadius * sin(orbitAngle);
        position.y = height;
        position.z = orbitRadius * cos(orbitAngle);
    }

    // Roda a câmara à volta do alvo, alterando o ângulo de órbita
    void rotateAroundTarget(float degreesToMove)
    {
        orbitAngle += degreesToMove;
        updatePosition();
    }

    // Parâmetros públicos para fácil acesso e manipulação
    glm::vec3 position;   // Posição atual da câmara
    glm::vec3 up;         // Vetor "up" (orientação vertical)
    float fov;            // Campo de visão (em graus)

    float orbitAngle = 0.0f;         // Ângulo atual de órbita em torno do alvo
    float orbitRadius = 15.0f;       // Raio da órbita em torno do alvo
    glm::vec3 target = glm::vec3(0.0f); // Ponto para onde a câmara está a olhar
    float height = 3.0f;             // Altura da câmara em relação ao alvo
};