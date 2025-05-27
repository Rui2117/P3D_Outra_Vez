#version 330 core

// Atributos de entrada (vindos do VBO)
layout(location = 0) in vec3 vPosition;  // Posi��o do v�rtice
layout(location = 1) in vec3 vNormal;    // Normal do v�rtice
layout(location = 2) in vec2 vTexCoord;  // Coordenada de textura
in vec3 vColors;                         // Cor do v�rtice

// Uniforms
uniform mat4 MVP;  // Matriz Model-View-Projection

// Vari�veis de sa�da (para o fragment shader)
out vec3 fragNormal;
out vec2 fragTexCoord;
out vec3 fragColor;

void main() {
    // Passa as vari�veis para o fragment shader
    fragNormal = vNormal;
    fragTexCoord = vTexCoord;
    fragColor = vColors;

    // Transforma a posi��o do v�rtice
    gl_Position = MVP * vec4(vPosition, 1.0);
}