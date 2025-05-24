#pragma once
#include <GL\gl.h>

#define _DEBUG

typedef struct {
    GLenum       type;
    const char* filename;
    GLuint       shader;
} ShaderInfo;

GLuint LoadShaders(ShaderInfo*);

void DestroyShaders(ShaderInfo*);