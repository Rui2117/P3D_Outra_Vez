#version 330 core

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

out vec2 TexCoord;

uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(vPosition, 1.0);
    TexCoord = vTexCoord;
}