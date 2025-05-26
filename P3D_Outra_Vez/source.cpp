#pragma region includes

// Vincula as bibliotecas necessárias para o projeto
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "opengl32.lib")

#include <iostream>
#include <vector>

// Define que o GLEW será usado de forma estática
#define GLEW_STATIC
#include <GL\glew.h>
#include <gl\GL.h>
#include <GLFW\glfw3.h>

// Inclui GLM para operações matemáticas com vetores e matrizes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Inclui headers do projeto
#include "shader.h"
#include "camera.h"
#include "model.h"

#pragma endregion

// Define o tamanho da janela
#define WIDTH 640
#define HEIGHT 480

// Declaração de funções auxiliares
void print_error(int error, const char* description);
void init(void);
void display(void);

#define NumBuffers 3 // Número de buffers: vértices, cores, EBO

GLuint program; // ID do programa de shader

GLuint VAO; // Vertex Array Object
GLuint Buffers[NumBuffers]; // Buffers para vértices, cores e índices
const GLuint NumVertices = 8; // Número de vértices do cubo/mesa
const GLuint NumIndices = 6 * 2 * 3; // 6 faces * 2 triângulos/face * 3 vértices/triângulo

GLfloat rotation = 0.0f; // Rotação (não usada diretamente aqui)

Camera camera; // Câmera principal
Camera topDownCamera; // Câmera para o mini-mapa

ObjModel* bola1; // Ponteiro para o modelo da bola1
ObjModel* bola2; // Ponteiro para o modelo da bola2

// Variáveis para controle do mouse
bool isPressing = false;
double prevXpos = 0.0, prevYpos = 0.0;
double xPos = 0.0, yPos = 0.0;

// Callback para o scroll do mouse (zoom)
void scrollCallBack(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.fov -= static_cast<float>(yoffset);
    camera.fov = glm::clamp(camera.fov, 15.0f, 90.0f); // Limita o zoom
    std::cout << "FOV (Zoom) = " << camera.fov << std::endl;
}

// Callback para o movimento do mouse (rotação e altura da câmera)
void cursorCallBack(GLFWwindow* window, double xpos, double ypos)
{
    xPos = xpos;
    yPos = ypos;

    if (isPressing) {
        double deltaX = xpos - prevXpos;
        camera.rotateAroundTarget(static_cast<float>(deltaX) / WIDTH * glm::pi<float>());
        prevXpos = xpos;

        double deltaY = ypos - prevYpos;
        camera.height += static_cast<float>(-deltaY) / HEIGHT;
        camera.height = glm::clamp(camera.height, 0.5f, 30.0f); // Limita altura
        camera.updatePosition();
    }
}

// Callback para clique do mouse (ativa/desativa rotação)
void mouseCallBack(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            isPressing = true;
            prevXpos = xPos;
            prevYpos = yPos;
        }
        else if (action == GLFW_RELEASE)
        {
            isPressing = false;
        }
    }
}

// Função principal
int main(void)
{
    camera.updatePosition(); // Inicializa posição da câmera

    GLFWwindow* window;

    glfwSetErrorCallback(print_error);

    if (!glfwInit()) return -1; // Inicializa GLFW

    window = glfwCreateWindow(WIDTH, HEIGHT, "Bilhar", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Inicializa GLEW (extensões OpenGL)
    glewExperimental = GL_TRUE;
    glewInit();

    // Registra callbacks de input
    glfwSetScrollCallback(window, scrollCallBack);
    glfwSetCursorPosCallback(window, cursorCallBack);
    glfwSetMouseButtonCallback(window, mouseCallBack);

    init(); // Inicializa buffers, shaders e modelos

    glm::mat4 projection = camera.getProjectionMatrix((float)WIDTH / (float)HEIGHT);

    // Loop principal de renderização
    while (!glfwWindowShouldClose(window))
    {
        glUseProgram(program);
        glUniform1i(glGetUniformLocation(program, "tex"), 0);

        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- 1. Vista principal (janela inteira)
        glViewport(0, 0, WIDTH, HEIGHT);

        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0, -2, 0));
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 projection = camera.getProjectionMatrix((float)WIDTH / (float)HEIGHT);
        glm::mat4 mvp = projection * view * model;

        GLint mvpId = glGetUniformLocation(program, "MVP");
        glUniformMatrix4fv(mvpId, 1, GL_FALSE, glm::value_ptr(mvp));
        display();

        // --- 2. Mini-mapa (canto superior direito)
        int miniWidth = 150, miniHeight = 150;
        glViewport(WIDTH - miniWidth - 10, HEIGHT - miniHeight - 10, miniWidth, miniHeight);

        glm::mat4 miniView = topDownCamera.getViewMatrix();
        glm::mat4 miniProj = topDownCamera.getProjectionMatrix((float)miniWidth / (float)miniHeight);
        glm::mat4 miniMVP = miniProj * miniView * model;

        glUniformMatrix4fv(mvpId, 1, GL_FALSE, glm::value_ptr(miniMVP));
        display();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// Inicializa buffers, shaders e modelos
void init(void) {
    glEnable(GL_DEPTH_TEST);

    // Define os vértices da mesa de bilhar (cubo achatado)
    GLfloat vertices[NumVertices][3] = {
        {-9.0f,  0.5f,  5.5f }, { 9.0f,  0.5f,  5.5f },
        {-9.0f, -0.5f,  5.5f }, { 9.0f, -0.5f,  5.5f },
        {-9.0f,  0.5f, -5.5f }, { 9.0f,  0.5f, -5.5f },
        {-9.0f, -0.5f, -5.5f }, { 9.0f, -0.5f, -5.5f },
    };

    // Cores dos vértices (verde)
    GLfloat cores[NumVertices][3] = {
        { 0.4f, 0.8f, 0.5f }, { 0.4f, 0.8f, 0.5f },
        { 0.4f, 0.8f, 0.5f }, { 0.4f, 0.8f, 0.5f },
        { 0.4f, 0.8f, 0.5f }, { 0.4f, 0.8f, 0.5f },
        { 0.4f, 0.8f, 0.5f }, { 0.4f, 0.8f, 0.5f },
    };

    // Índices para desenhar os triângulos da mesa
    GLuint indices[NumIndices] = {
        // Frente
        0, 1, 2, 1, 3, 2,
        // Direita
        1, 3, 7, 1, 5, 7,
        // Baixo
        2, 3, 6, 3, 6, 7,
        // Esquerda
        0, 2, 4, 2, 4, 6,
        // Trás
        4, 5, 6, 5, 6, 7,
        // Cima
        0, 1, 4, 1, 4, 5
    };

    // Cria e configura o VAO e VBOs
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(NumBuffers, Buffers);

    glBindBuffer(GL_ARRAY_BUFFER, Buffers[0]);
    glBufferStorage(GL_ARRAY_BUFFER, sizeof(vertices), vertices, 0);

    glBindBuffer(GL_ARRAY_BUFFER, Buffers[1]);
    glBufferStorage(GL_ARRAY_BUFFER, sizeof(cores), cores, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffers[2]);
    glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, 0);

    // Carrega e ativa os shaders
    ShaderInfo shaders[] = {
        { GL_VERTEX_SHADER,   "shader.vert" },
        { GL_FRAGMENT_SHADER, "shader.frag" },
        { GL_NONE, NULL }
    };

    program = LoadShaders(shaders);
    if (!program) exit(EXIT_FAILURE);
    glUseProgram(program);

    // Liga os atributos dos vértices aos shaders
    GLint coordsId = glGetProgramResourceLocation(program, GL_PROGRAM_INPUT, "vPosition");
    GLint coresId = glGetProgramResourceLocation(program, GL_PROGRAM_INPUT, "vColors");

    glBindBuffer(GL_ARRAY_BUFFER, Buffers[0]);
    glVertexAttribPointer(coordsId, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, Buffers[1]);
    glVertexAttribPointer(coresId, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glEnableVertexAttribArray(coordsId);
    glEnableVertexAttribArray(coresId);

    // Configura a câmera do mini-mapa (vista de cima)
    topDownCamera.position = glm::vec3(0.0f, 30.0f, 0.0f);
    topDownCamera.target = glm::vec3(0.0f, 0.0f, 0.0f);
    topDownCamera.up = glm::vec3(0.0f, 0.0f, -1.0f); // Aponta para baixo
    topDownCamera.fov = 45.0f;

    // Carrega o modelo da bola de bilhar
    bola1 = new ObjModel("PoolBalls/ball1.obj");
    bola2 = new ObjModel("PoolBalls/ball2.obj");
}

// Função de desenho chamada a cada frame
void display(void)
{
    GLint objectTypeLoc = glGetUniformLocation(program, "objectType");
    GLint hasTextureLoc = glGetUniformLocation(program, "hasTexture");

    // Desenha o modelo (bola de bilhar)
    glUniform1i(objectTypeLoc, 1);
    glUniform1i(hasTextureLoc, true);
    bola1->draw();
    bola2->draw();
    glBindVertexArray(VAO);

    // Desenha a mesa (cubo)
    glUniform1i(objectTypeLoc, 0);
    glUniform1i(hasTextureLoc, false);
    glDrawElements(GL_TRIANGLES, NumIndices, GL_UNSIGNED_INT, (void*)0);
}

// Callback de erro do GLFW
void print_error(int error, const char* description) {
    std::cout << description << std::endl;
}