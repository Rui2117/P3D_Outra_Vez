#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

// Classe que representa uma c�mara 3D orbitando um alvo
class Camera
{
public:
    // Construtor padr�o: inicializa a posi��o, alvo, vetor up e campo de vis�o (fov)
    Camera() : position(0.0f, 5.0f, 20.0f), target(0.0f, 0.0f, 0.0f), up(0.0f, 1.0f, 0.0f), fov(60.0f)
    {
    }

    // Construtor parametrizado: permite definir posi��o, alvo, up e fov
    Camera(glm::vec3 position, glm::vec3 target, glm::vec3 upDirection, float fov) :
        position(position), target(target), up(upDirection), fov(fov) {
    }

    // Retorna a matriz de vis�o (view), usada para transformar o mundo para a perspetiva da c�mara
    mat4 getViewMatrix() const {
        return glm::lookAt(position, target, up);
    }

    // Retorna a matriz de proje��o (perspetiva), usada para projetar 3D em 2D
    mat4 getProjectionMatrix(float aspectRatio) const {
        return glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 100.0f);
    }

    // Processa o zoom alterando o campo de vis�o (fov), limitado entre 15 e 90 graus
    void processZOOM(double yoffset) {
        fov -= static_cast<float>(yoffset);
        fov = glm::clamp(fov, 15.0f, 90.0f);
    }

    // Atualiza a posi��o da c�mara com base no �ngulo de �rbita e altura
    void updatePosition() {
        position.x = orbitRadius * sin(orbitAngle);
        position.y = height;
        position.z = orbitRadius * cos(orbitAngle);
    }

    // Roda a c�mara � volta do alvo, alterando o �ngulo de �rbita
    void rotateAroundTarget(float degreesToMove)
    {
        orbitAngle += degreesToMove;
        updatePosition();
    }

    // Par�metros p�blicos para f�cil acesso e manipula��o
    glm::vec3 position;   // Posi��o atual da c�mara
    glm::vec3 up;         // Vetor "up" (orienta��o vertical)
    float fov;            // Campo de vis�o (em graus)

    float orbitAngle = 0.0f;         // �ngulo atual de �rbita em torno do alvo
    float orbitRadius = 15.0f;       // Raio da �rbita em torno do alvo
    glm::vec3 target = glm::vec3(0.0f); // Ponto para onde a c�mara est� a olhar
    float height = 3.0f;             // Altura da c�mara em rela��o ao alvo
};