#pragma region includes

#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "opengl32.lib")

#include <iostream>
#include <vector>

#define GLEW_STATIC
#include <GL\glew.h>

#include <gl\GL.h>

#include <GLFW\glfw3.h>

#include <glm/glm.hpp> // vec3, vec4, ivec4, mat4, ...
#include <glm/gtc/matrix_transform.hpp> // translate, rotate, scale, perspective, ...
#include <glm/gtc/type_ptr.hpp> // value_ptr

#include "shader.h"
#include "camera.h"
#include "model.h"

#pragma endregion

#define WIDTH 640
#define HEIGHT 480

void print_error(int error, const char* description);
void init(void);
void display(void);

#define NumBuffers 3 // Vértices, Cores, EBO

GLuint program;

GLuint VAO;
GLuint Buffers[NumBuffers];
const GLuint NumVertices = 8; // 6 faces * 4 vértices
const GLuint NumIndices = 6 * 2 * 3; // 6 faces * 2 triângulos/face * 3 vértices/triângulo

GLfloat rotation = 0.0f;

Camera camera;
Camera topDownCamera;

ObjModel* mesa;


bool isPressing = false;

double prevXpos = 0.0;
double prevYpos = 0.0;
double xPos = 0.0;
double yPos = 0.0;

void scrollCallBack(GLFWwindow* window, double xoffset, double yoffset) 
{

    camera.fov -= static_cast<float>(yoffset);
    camera.fov = glm::clamp(camera.fov, 15.0f, 90.0f);

    std::cout << "FOV (Zoom) = " << camera.fov << std::endl;
}

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
        camera.height = glm::clamp(camera.height, 0.5f, 30.0f);
        camera.updatePosition();
    }
}

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

int main(void) 
{
    camera.updatePosition();

    GLFWwindow* window;

    glfwSetErrorCallback(print_error);

    if (!glfwInit()) return -1;

    window = glfwCreateWindow(WIDTH, HEIGHT, "Bilhar", NULL, NULL);
    if (!window) 
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Inicia o gestor de extensões GLEW
    glewExperimental = GL_TRUE;
    glewInit();

    glfwSetScrollCallback(window, scrollCallBack);
    glfwSetCursorPosCallback(window, cursorCallBack);
    glfwSetMouseButtonCallback(window, mouseCallBack);

    init();

    glm::mat4 projection = camera.getProjectionMatrix((float)WIDTH / (float)HEIGHT);

    while (!glfwWindowShouldClose(window))
    {
        glUseProgram(program);

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

void init(void) {
    glEnable(GL_DEPTH_TEST);

    // Mesa de Bilhar (fiz isto manualmente (not fun)) - Zes
    GLfloat vertices[NumVertices][3] = {
        {-9.0f,  0.5f,  5.5f }, { 9.0f,  0.5f,  5.5f },
        {-9.0f, -0.5f,  5.5f }, { 9.0f, -0.5f,  5.5f },

        {-9.0f,  0.5f, -5.5f }, { 9.0f,  0.5f, -5.5f },
        {-9.0f, -0.5f, -5.5f }, { 9.0f, -0.5f, -5.5f },
    };

    GLfloat cores[NumVertices][3] = {
        { 0.4f, 0.8f, 0.5f }, { 0.4f, 0.8f, 0.5f },
        { 0.4f, 0.8f, 0.5f }, { 0.4f, 0.8f, 0.5f },

        { 0.4f, 0.8f, 0.5f }, { 0.4f, 0.8f, 0.5f },
        { 0.4f, 0.8f, 0.5f }, { 0.4f, 0.8f, 0.5f },
    };

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

    // VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // VBOs
    glGenBuffers(NumBuffers, Buffers);

    glBindBuffer(GL_ARRAY_BUFFER, Buffers[0]);
    glBufferStorage(GL_ARRAY_BUFFER, sizeof(vertices) /*2 * 6 * sizeof(float)*/, vertices, 0);

    glBindBuffer(GL_ARRAY_BUFFER, Buffers[1]);
    glBufferStorage(GL_ARRAY_BUFFER, sizeof(cores) /*3 * 6 * sizeof(float)*/, cores, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffers[2]);
    glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, 0);


    ShaderInfo shaders[] = {
        { GL_VERTEX_SHADER,   "shader.vert" },
        { GL_FRAGMENT_SHADER, "shader.frag" },
        { GL_NONE, NULL }
    };

    program = LoadShaders(shaders);
    if (!program) exit(EXIT_FAILURE);
    glUseProgram(program);


    // Ligar os atributos aos shaders

    // Obtém a localização do atributo 'vPosition' no 'programa'.
    GLint coordsId = glGetProgramResourceLocation(program, GL_PROGRAM_INPUT, "vPosition");
    GLint coresId = glGetProgramResourceLocation(program, GL_PROGRAM_INPUT, "vColors");

    glBindBuffer(GL_ARRAY_BUFFER, Buffers[0]);
    glVertexAttribPointer(coordsId, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, Buffers[1]);
    glVertexAttribPointer(coresId, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glEnableVertexAttribArray(coordsId);
    glEnableVertexAttribArray(coresId);

    //minimap
    topDownCamera.position = glm::vec3(0.0f, 30.0f, 0.0f);
    topDownCamera.target = glm::vec3(0.0f, 0.0f, 0.0f);
    topDownCamera.up = glm::vec3(0.0f, 0.0f, -1.0f); // Para "virar" a câmara para baixo
    topDownCamera.fov = 45.0f;


    //models

    mesa = new ObjModel("PoolBalls/ball1.obj");
}

void display(void) 
{

    mesa->draw();
    glBindVertexArray(VAO);

    glDrawElements(GL_TRIANGLES, NumIndices, GL_UNSIGNED_INT, (void*)0);
}

void print_error(int error, const char* description) {
    std::cout << description << std::endl;
}