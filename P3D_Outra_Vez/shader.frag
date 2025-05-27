#version 330 core

// Variáveis de entrada (do vertex shader)
in vec3 fragNormal;
in vec2 fragTexCoord;
in vec3 fragColor;

// Uniforms
uniform vec3 ambientLight;
uniform int objectType;  // 0 para mesa, 1 para bola
uniform bool hasTexture;
uniform sampler2D tex;

// Saída
out vec4 fragOutput;

void main() {
    // Normaliza a normal do fragmento
    vec3 normal = normalize(fragNormal);
    
    // Define a cor base do objeto
    vec3 baseColor;
    if (hasTexture) {
        baseColor = texture(tex, fragTexCoord).rgb;
    } else {
        baseColor = fragColor;
    }
    
    // Aplica iluminação ambiente
    vec3 ambient = ambientLight * baseColor;
    
    // Cor final é apenas a componente ambiente por enquanto
    vec3 finalColor = ambient;
    
    fragOutput = vec4(finalColor, 1.0);
}