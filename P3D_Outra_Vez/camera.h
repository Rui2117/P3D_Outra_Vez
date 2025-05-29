#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

class Camera {
public:
    Camera() :
        position(0.0f, 5.0f, 20.0f),
        target(0.0f, 0.0f, 0.0f),
        up(0.0f, 1.0f, 0.0f),
        fov(60.0f) {
    }

    Camera(glm::vec3 position, glm::vec3 target, glm::vec3 upDirection, float fov) :
        position(position), target(target), up(upDirection), fov(fov) {
    }

    mat4 getViewMatrix() const {
        return glm::lookAt(position, target, up);
    }

    mat4 getProjectionMatrix(float aspectRatio) const {
        return glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 100.0f);
    }

    void processZOOM(double yoffset) {
        fov -= static_cast<float>(yoffset);
        fov = glm::clamp(fov, 15.0f, 90.0f);
    }

    void updatePosition() {
        position.x = orbitRadius * sin(orbitAngle);
        position.y = height;
        position.z = orbitRadius * cos(orbitAngle);
    }

    void rotateAroundTarget(float degreesToMove) {
        orbitAngle += degreesToMove;
        updatePosition();
    }

   
    glm::vec3 position;   
    glm::vec3 up;         
    float fov;            

    float orbitAngle = 0.0f;         
    float orbitRadius = 15.0f;        
    glm::vec3 target = glm::vec3(0.0f); 
    float height = 3.0f;              
};