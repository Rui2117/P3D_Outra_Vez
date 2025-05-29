#pragma region includes
/***********************************************************************
 * Billiards Game OpenGL Implementation
 *
 * This file implements a 3D billiards game using Modern OpenGL (3.3+).
 * It includes camera controls, lighting, and 3D model rendering.
 ***********************************************************************/

 // Vinculação de bibliotecas necessárias
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "opengl32.lib")

// Inclusões padrão
#include <iostream>
#include <vector>

// Configuração do GLEW para linkagem estática
#define GLEW_STATIC
#include <GL\glew.h>
#include <gl\GL.h>
#include <GLFW\glfw3.h>

// GLM para operações matemáticas 3D
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Cabeçalhos do projeto
#include "shader.h"
#include "camera.h"
#include "model.h"

/**
 * Constantes de configuração da janela e visualização
 */
constexpr int WIDTH = 640;           // Largura da janela principal
constexpr int HEIGHT = 480;          // Altura da janela principal
constexpr int MINIMAP_SIZE = 150;    // Tamanho do minimapa
constexpr int MINIMAP_PADDING = 10;  // Espaçamento do minimapa

/**
 * Configurações do OpenGL
 */
#define NumBuffers 3                 // Número de buffers OpenGL (vértices, cores, índices)
const GLuint NumVertices = 8;        // Número de vértices da mesa
const GLuint NumIndices = 6 * 2 * 3; // Número de índices para triângulos

/**
 * Limites da câmera
 */
constexpr float MIN_FOV = 15.0f;     // Campo de visão mínimo
constexpr float MAX_FOV = 90.0f;     // Campo de visão máximo
constexpr float MIN_HEIGHT = 0.5f;   // Altura mínima da câmera
constexpr float MAX_HEIGHT = 30.0f;  // Altura máxima da câmera

/**
 * Estrutura para controle de iluminação
 * Gerencia propriedades da luz ambiente
 */
struct LightingParams {
    glm::vec3 ambientLight;      // Cor da luz ambiente
    float ambientIntensity;      // Intensidade da luz ambiente
    bool isAmbientLightOn;       // Estado da luz ambiente (ligada/desligada)

    LightingParams() :
        ambientLight(1.0f),
        ambientIntensity(1.0f),
        isAmbientLightOn(true) {
    }
};

/**
 * Estrutura para luz pontual
 * Define posição e propriedades de uma fonte de luz
 */
struct Light {
    glm::vec3 position;  // Posição da luz no espaço 3D
    glm::vec3 color;     // Cor da luz
    float intensity;     // Intensidade da luz

    Light() :
        position(5.0f, 5.0f, 0.0f),
        color(1.0f),
        intensity(1.0f) {
    }
};

/**
 * Variáveis globais do estado do jogo
 */
LightingParams lighting;        // Controle de iluminação
Light mainLight;                // Luz principal
GLuint program;                 // Programa de shader
GLuint VAO;                     // Vertex Array Object
GLuint Buffers[NumBuffers];     // Buffer Objects
Camera camera;                  // Câmera principal
Camera topDownCamera;           // Câmera do minimapa

std::vector<ObjModel*> bolas;   // Bolas de Bilhar

/**
 * Estrutura para controle de entrada do usuário
 */
struct InputState {
    bool isPressing = false;         // Indica se botão do mouse está pressionado
    double prevXpos = 0.0;          // Posição X anterior do mouse
    double prevYpos = 0.0;          // Posição Y anterior do mouse
    double xPos = 0.0;              // Posição X atual do mouse
    double yPos = 0.0;              // Posição Y atual do mouse
} input;

// Declarações antecipadas de funções
void print_error(int error, const char* description);
void init(void);
void display(const glm::mat4& view, const glm::mat4& projection);

/**
 * Callback de teclado
 * Processa eventos de teclado para controle de iluminação
 */
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
        case GLFW_KEY_1: // Tecla 1 liga/desliga luz ambiente
            lighting.isAmbientLightOn = !lighting.isAmbientLightOn;
            break;
        }
    }
}

/**
 * Callback de scroll do mouse
 * Controla o zoom da câmera através do campo de visão (FOV)
 */
void scrollCallBack(GLFWwindow* window, double xoffset, double yoffset) {
    camera.fov = glm::clamp(camera.fov - static_cast<float>(yoffset), MIN_FOV, MAX_FOV);
}

/**
 * Callback de movimento do mouse
 * Controla a rotação e altura da câmera
 */
void cursorCallBack(GLFWwindow* window, double xpos, double ypos) {
    input.xPos = xpos;
    input.yPos = ypos;

    if (input.isPressing) {
        // Rotação horizontal da câmera
        const double deltaX = xpos - input.prevXpos;
        camera.rotateAroundTarget(static_cast<float>(deltaX) / -WIDTH * glm::pi<float>());
        input.prevXpos = xpos;

        // Ajuste de altura da câmera
        const double deltaY = ypos - input.prevYpos;
        camera.height = glm::clamp(
            camera.height + static_cast<float>(-deltaY) / HEIGHT,
            MIN_HEIGHT,
            MAX_HEIGHT
        );
        camera.updatePosition();
    }
}

/**
 * Callback de botão do mouse
 * Gerencia o estado de pressão do mouse para controle da câmera
 */
void mouseCallBack(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        input.isPressing = (action == GLFW_PRESS);
        if (input.isPressing) {
            input.prevXpos = input.xPos;
            input.prevYpos = input.yPos;
        }
    }
}

/**
 * Função principal
 * Ponto de entrada da aplicação, configura o ambiente OpenGL e executa o loop principal
 */
int main(void) {
    // Inicializa a câmera antes de criar a janela
    camera.updatePosition();

    // Inicializa GLFW e cria a janela
    GLFWwindow* window;
    glfwSetErrorCallback(print_error);

    if (!glfwInit()) {
        std::cerr << "Falha ao inicializar GLFW" << std::endl;
        return -1;
    }

    // Cria contexto OpenGL 3.3 core profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Bilhar", nullptr, nullptr);
    if (!window) {
        std::cerr << "Falha ao criar janela GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Inicializa GLEW
    glewExperimental = GL_TRUE; // Habilita recursos modernos do OpenGL
    if (glewInit() != GLEW_OK) {
        std::cerr << "Falha ao inicializar GLEW" << std::endl;
        return -1;
    }

    // Configura callbacks de entrada
    glfwSetScrollCallback(window, scrollCallBack);
    glfwSetCursorPosCallback(window, cursorCallBack);
    glfwSetMouseButtonCallback(window, mouseCallBack);
    glfwSetKeyCallback(window, keyCallback);

    // Inicializa estado do OpenGL e recursos
    init();

    // Armazena localizações de uniforms para melhor performance
    const GLint ambientLightLoc = glGetUniformLocation(program, "ambientLight");
    const GLint textureLoc = glGetUniformLocation(program, "tex");
    const GLint mvpLoc = glGetUniformLocation(program, "MVP");

    // Loop principal de renderização
    while (!glfwWindowShouldClose(window)) {
        // Atualiza estado da iluminação
        const glm::vec3 finalAmbientLight = lighting.isAmbientLightOn ?
            lighting.ambientLight * lighting.ambientIntensity :
            glm::vec3(0.0f);

        // Configura estado comum do OpenGL
        glUseProgram(program);
        glUniform1i(textureLoc, 0);
        glUniform3fv(ambientLightLoc, 1, glm::value_ptr(finalAmbientLight));
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Matrizes de transformação comuns
        const glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0, -2, 0));

        // Renderiza vista principal
        {
            glViewport(0, 0, WIDTH, HEIGHT);
            const glm::mat4 view = camera.getViewMatrix();
            const glm::mat4 projection = camera.getProjectionMatrix(static_cast<float>(WIDTH) / HEIGHT);
            const glm::mat4 mvp = projection * view * model;

            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));
            display(view, projection);
        }

        // Renderiza minimapa
        {
            constexpr int MINIMAP_BORDER = 10;
            glViewport(
                WIDTH - MINIMAP_SIZE - MINIMAP_BORDER,
                HEIGHT - MINIMAP_SIZE - MINIMAP_BORDER,
                MINIMAP_SIZE,
                MINIMAP_SIZE
            );

            const glm::mat4 miniView = topDownCamera.getViewMatrix();
            const glm::mat4 miniProj = topDownCamera.getProjectionMatrix(1.0f);
            const glm::mat4 miniMVP = miniProj * miniView * model;

            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(miniMVP));
            display(miniView, miniProj);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    for (auto* bola : bolas) {
        delete bola;
    }
    bolas.clear();

    glfwTerminate();
    return 0;
}

void createBall(const std::string& modelPath, const glm::vec3& position) {
    ObjModel* bola = new ObjModel(modelPath);
    bola->setPosition(position);
    bola->setScale(vec3(0.5f));
    bolas.push_back(bola);
}

/**
 * Inicializa recursos do OpenGL
 * Configura a geometria da mesa de bilhar, shaders e modelos 3D
 */
void init(void) {
    glEnable(GL_DEPTH_TEST);

    // Vértices da mesa - otimizados para coerência de cache
    constexpr GLfloat tableWidth = 9.0f;   // Largura da mesa
    constexpr GLfloat tableHeight = 0.5f;  // Altura da mesa
    constexpr GLfloat tableDepth = 5.5f;   // Profundidade da mesa

    // Define os vértices da mesa
    const GLfloat vertices[NumVertices][3] = {
        {-tableWidth,  tableHeight,  tableDepth}, { tableWidth,  tableHeight,  tableDepth},
        {-tableWidth, -tableHeight,  tableDepth}, { tableWidth, -tableHeight,  tableDepth},
        {-tableWidth,  tableHeight, -tableDepth}, { tableWidth,  tableHeight, -tableDepth},
        {-tableWidth, -tableHeight, -tableDepth}, { tableWidth, -tableHeight, -tableDepth}
    };

    glm::vec3 ballsPosition[15] = {
        {-1.0f, -1.0f, 0.0f},

        {-1.8f, -1.0f, 0.5f},
        {-1.8f, -1.0f, -0.5f},

        {-2.6f, -1.0f, 1.0f},
        {-2.6f, -1.0f, 0.0f},
        {-2.6f, -1.0f, -1.0f},

        {-3.4f, -1.0f, 1.5f},
        {-3.4f, -1.0f, 0.5f},
        {-3.4f, -1.0f, -0.5f},
        {-3.4f, -1.0f, -1.5f},

        {-4.2f, -1.0f, 2.0f},
        {-4.2f, -1.0f, 1.0f},
        {-4.2f, -1.0f, 0.0f},
        {-4.2f, -1.0f, -1.0f},
        {-4.2f, -1.0f, -2.0f}
    };

    // Cor da mesa (feltro verde)
    const GLfloat tableColor[3] = { 0.4f, 0.8f, 0.5f };
    GLfloat colors[NumVertices][3];
    for (int i = 0; i < NumVertices; i++) {
        std::copy(std::begin(tableColor), std::end(tableColor), colors[i]);
    }

    // Índices da mesa - otimizados para renderização em triangle strip
    const GLuint indices[NumIndices] = {
        0, 1, 2, 1, 3, 2,  // Frente
        1, 3, 7, 1, 5, 7,  // Direita
        2, 3, 6, 3, 6, 7,  // Base
        0, 2, 4, 2, 4, 6,  // Esquerda
        4, 5, 6, 5, 6, 7,  // Fundo
        0, 1, 4, 1, 4, 5   // Topo
    };

    // Configura VAO e buffers
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(NumBuffers, Buffers);

    // Buffer de posições dos vértices
    glBindBuffer(GL_ARRAY_BUFFER, Buffers[0]);
    glBufferStorage(GL_ARRAY_BUFFER, sizeof(vertices), vertices, 0);

    // Buffer de cores dos vértices
    glBindBuffer(GL_ARRAY_BUFFER, Buffers[1]);
    glBufferStorage(GL_ARRAY_BUFFER, sizeof(colors), colors, 0);

    // Buffer de índices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffers[2]);
    glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, 0);

    // Carrega e compila shaders
    ShaderInfo shaders[] = {
        { GL_VERTEX_SHADER,   "shader.vert" },
        { GL_FRAGMENT_SHADER, "shader.frag" },
        { GL_NONE, NULL }
    };

    program = LoadShaders(shaders);
    if (!program) {
        std::cerr << "Falha ao carregar shaders" << std::endl;
        exit(EXIT_FAILURE);
    }
    glUseProgram(program);

    // Configura atributos dos vértices
    const GLint coordsId = glGetProgramResourceLocation(program, GL_PROGRAM_INPUT, "vPosition");
    const GLint colorsId = glGetProgramResourceLocation(program, GL_PROGRAM_INPUT, "vColors");

    glBindBuffer(GL_ARRAY_BUFFER, Buffers[0]);
    glVertexAttribPointer(coordsId, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, Buffers[1]);
    glVertexAttribPointer(colorsId, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glEnableVertexAttribArray(coordsId);
    glEnableVertexAttribArray(colorsId);

    // Inicializa câmera superior para o minimapa
    topDownCamera.position = glm::vec3(0.0f, 30.0f, 0.0f);
    topDownCamera.target = glm::vec3(0.0f, 0.0f, 0.0f);
    topDownCamera.up = glm::vec3(0.0f, 0.0f, -1.0f);
    topDownCamera.fov = 45.0f;

    // Carrega modelos das bolas de bilhar
    try {
        for(int i = 0; i < 16; ++i) {
            createBall("PoolBalls/ball" + std::to_string(i + 1) + ".obj", ballsPosition[i]);
		}
    }
    catch (const std::exception& e) {
        std::cerr << "Falha ao carregar modelos das bolas: " << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}

/**
 * Renderiza a cena
 * @param view Matriz de visualização da câmera atual
 * @param projection Matriz de projeção do viewport atual
 */
void display(const glm::mat4& view, const glm::mat4& projection) {
    // Obtém localizações de uniforms
    const GLint objectTypeLoc = glGetUniformLocation(program, "objectType");
    const GLint hasTextureLoc = glGetUniformLocation(program, "hasTexture");

    // Desenha mesa de bilhar
    glUniform1i(objectTypeLoc, 0);
    glUniform1i(hasTextureLoc, false);
    glDrawElements(GL_TRIANGLES, NumIndices, GL_UNSIGNED_INT, nullptr);

    // Desenha bolas de bilhar
    glUniform1i(objectTypeLoc, 1);
    glUniform1i(hasTextureLoc, true);

    // Renderiza todas as bolas do vetor
    for (const auto& bola : bolas) {
        bola->render(program, view, projection);
    }

    // Restaura binding do VAO para o próximo frame
    glBindVertexArray(VAO);
}

/**
 * Callback de erro do GLFW
 * Imprime mensagens de erro no console
 */
void print_error(int error, const char* description) {
    std::cerr << "Erro GLFW " << error << ": " << description << std::endl;
}