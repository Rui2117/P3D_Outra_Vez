#version 330 core

// Atributos de entrada (vindos do VBO)
layout(location = 0) in vec3 vPosition;  // Posição do vértice
layout(location = 1) in vec3 vNormal;    // Normal do vértice
layout(location = 2) in vec2 vTexCoord;  // Coordenada de textura
in vec3 vColors;                         // Cor do vértice

// Uniforms
uniform mat4 MVP;  // Matriz Model-View-Projection

// Variáveis de saída (para o fragment shader)
out vec3 fragNormal;
out vec2 fragTexCoord;
out vec3 fragColor;

void main() {
    // Passa as variáveis para o fragment shader
    fragNormal = vNormal;
    fragTexCoord = vTexCoord;
    fragColor = vColors;

    // Transforma a posição do vértice
    gl_Position = MVP * vec4(vPosition, 1.0);
}