#pragma once

#include <iostream>
#include <fstream>

#define GLEW_STATIC
#include <GL\glew.h>

#include "shader.h"

static const GLchar* ReadShader(const char* filename) {
	std::ifstream ficheiro(filename, std::ifstream::ate | std::ifstream::binary);
	if (ficheiro.is_open()) {
		std::streampos tamanhoDoFicheiroEmBytes = ficheiro.tellg();
		ficheiro.seekg(0, std::ios::beg);

		GLchar* source = new GLchar[int(tamanhoDoFicheiroEmBytes) + 1];
		ficheiro.read(source, tamanhoDoFicheiroEmBytes);
		source[tamanhoDoFicheiroEmBytes] = 0;

		ficheiro.close();
		return const_cast<const GLchar*>(source);
	}
	else {
		std::cerr << "Erro ao abrir o ficheiro '" << filename << "'" << std::endl;
	}
	return nullptr;
}

GLuint LoadShaders(ShaderInfo* shaders) {
	if (shaders == nullptr) return 0;

	GLuint program = glCreateProgram();

	for (GLint i = 0; shaders[i].type != GL_NONE; i++) {
		shaders[i].shader = glCreateShader(shaders[i].type);

		const GLchar* source = ReadShader(shaders[i].filename);
		if (source == NULL) {
			for (int j = 0; shaders[j].type != GL_NONE; j++) {
				if (shaders[j].shader != 0)
					glDeleteShader(shaders[j].shader);
				shaders[j].shader = 0;
			}
			return 0;
		}

		glShaderSource(shaders[i].shader, 1, &source, NULL);
		delete[] source;
		glCompileShader(shaders[i].shader);

		GLint compiled;
		glGetShaderiv(shaders[i].shader, GL_COMPILE_STATUS, &compiled);
		if (!compiled) {
#ifdef _DEBUG
			GLsizei len;
			glGetShaderiv(shaders[i].shader, GL_INFO_LOG_LENGTH, &len);
			GLchar* log = new GLchar[len + 1];
			glGetShaderInfoLog(shaders[i].shader, len, &len, log);
			std::cerr << "Shader compilation failed: " << log << std::endl;
			delete[] log;
#endif
			for (int j = 0; shaders[j].type != GL_NONE; j++) {
				if (shaders[j].shader != 0)
					glDeleteShader(shaders[j].shader);
				shaders[j].shader = 0;
			}
			return 0;
		}

		glAttachShader(program, shaders[i].shader);
	}

	glLinkProgram(program);

	GLint linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked) {
#ifdef _DEBUG
		GLsizei len;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
		GLchar* log = new GLchar[len + 1];
		glGetProgramInfoLog(program, len, &len, log);
		std::cerr << "Shader linking failed: " << log << std::endl;
		delete[] log;
#endif
		for (int j = 0; shaders[j].type != GL_NONE; j++) {
			if (shaders[j].shader != 0)
				glDeleteShader(shaders[j].shader);
			shaders[j].shader = 0;
		}
		return 0;
	}

	return program;
}