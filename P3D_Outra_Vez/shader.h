#pragma once
#include <GL\gl.h>

#define _DEBUG

// Estrutura que representa um shader a ser carregado.
// type: tipo do shader (ex: GL_VERTEX_SHADER, GL_FRAGMENT_SHADER)
// filename: caminho para o ficheiro fonte do shader
// shader: ID do shader criado ap�s compila��o
typedef struct {
    GLenum       type;
    const char* filename;
    GLuint       shader;
} ShaderInfo;

// Fun��o que carrega, compila e linka um conjunto de shaders.
// Recebe um array de ShaderInfo e retorna o ID do programa OpenGL criado.
GLuint LoadShaders(ShaderInfo*);

// Fun��o que destr�i e libera os shaders criados (libera recursos da GPU).
void DestroyShaders(ShaderInfo*);