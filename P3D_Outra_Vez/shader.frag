#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform int objectType;
uniform sampler2D texture_diffuse1;

void main() {
    if (objectType == 0) {
        // Table - solid green
        FragColor = vec4(0.0, 1.0, 0.0, 1.0);
    } else if (objectType == 1) {
        // Ball - use texture if available, otherwise red
        vec4 texColor = texture(texture_diffuse1, TexCoord);
        if (texColor.a < 0.1) {  // If texture loading failed (alpha near 0)
            FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        } else {
            FragColor = texColor;
        }
    }
}