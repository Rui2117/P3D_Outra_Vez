#pragma once
#include <GL\gl.h>

#define _DEBUG

// Estrutura que representa um shader a ser carregado.
// type: tipo do shader (ex: GL_VERTEX_SHADER, GL_FRAGMENT_SHADER)
// filename: caminho para o ficheiro fonte do shader
// shader: ID do shader criado após compilação
typedef struct {
    GLenum       type;
    const char* filename;
    GLuint       shader;
} ShaderInfo;

// Função que carrega, compila e linka um conjunto de shaders.
// Recebe um array de ShaderInfo e retorna o ID do programa OpenGL criado.
GLuint LoadShaders(ShaderInfo*);

// Função que destrói e libera os shaders criados (libera recursos da GPU).
void DestroyShaders(ShaderInfo*);