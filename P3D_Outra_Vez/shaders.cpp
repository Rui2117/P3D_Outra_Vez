#pragma once

#include <iostream>
#include <fstream>

#define GLEW_STATIC
#include <GL\glew.h>

#include "shader.h"

// Função auxiliar para ler o conteúdo de um ficheiro de shader para uma string
static const GLchar* ReadShader(const char* filename) {
	// Abre o ficheiro em modo binário e posiciona no final para obter o tamanho
	std::ifstream ficheiro(filename, std::ifstream::ate | std::ifstream::binary);
	if (ficheiro.is_open()) {
		// Obtém o tamanho do ficheiro
		std::streampos tamanhoDoFicheiroEmBytes = ficheiro.tellg();
		// Volta ao início do ficheiro
		ficheiro.seekg(0, std::ios::beg);

		// Aloca memória para o conteúdo do ficheiro (+1 para o terminador nulo)
		GLchar* source = new GLchar[int(tamanhoDoFicheiroEmBytes) + 1];
		// Lê o ficheiro para o array 'source'
		ficheiro.read(source, tamanhoDoFicheiroEmBytes);
		source[tamanhoDoFicheiroEmBytes] = 0; // Fecha a string

		ficheiro.close();
		return const_cast<const GLchar*>(source);
	}
	else {
		std::cerr << "Erro ao abrir o ficheiro '" << filename << "'" << std::endl;
	}
	return nullptr;
}

// Função que carrega, compila e linka um conjunto de shaders, retornando o ID do programa OpenGL
GLuint LoadShaders(ShaderInfo* shaders) {
	if (shaders == nullptr) return 0;

	// Cria um novo programa OpenGL
	GLuint program = glCreateProgram();

	// Para cada shader na lista
	for (GLint i = 0; shaders[i].type != GL_NONE; i++) {
		// Cria o objeto shader do tipo especificado (vertex, fragment, etc.)
		shaders[i].shader = glCreateShader(shaders[i].type);

		// Lê o código fonte do shader
		const GLchar* source = ReadShader(shaders[i].filename);
		if (source == NULL) {
			// Em caso de erro, apaga todos os shaders já criados
			for (int j = 0; shaders[j].type != GL_NONE; j++) {
				if (shaders[j].shader != 0)
					glDeleteShader(shaders[j].shader);
				shaders[j].shader = 0;
			}
			return 0;
		}

		// Associa o código fonte ao shader e compila
		glShaderSource(shaders[i].shader, 1, &source, NULL);
		delete[] source;
		glCompileShader(shaders[i].shader);

		// Verifica se compilou corretamente
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
			// Em caso de erro, apaga todos os shaders já criados
			for (int j = 0; shaders[j].type != GL_NONE; j++) {
				if (shaders[j].shader != 0)
					glDeleteShader(shaders[j].shader);
				shaders[j].shader = 0;
			}
			return 0;
		}

		// Anexa o shader compilado ao programa
		glAttachShader(program, shaders[i].shader);
	}

	// Tenta linkar o programa
	glLinkProgram(program);

	// Verifica se o link foi bem-sucedido
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
		// Em caso de erro, apaga todos os shaders já criados
		for (int j = 0; shaders[j].type != GL_NONE; j++) {
			if (shaders[j].shader != 0)
				glDeleteShader(shaders[j].shader);
			shaders[j].shader = 0;
		}
		return 0;
	}

	return program; // Retorna o ID do programa OpenGL pronto para uso
}