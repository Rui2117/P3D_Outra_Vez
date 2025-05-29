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

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"
#include "model.h"

constexpr int WIDTH = 640;          
constexpr int HEIGHT = 480;         
constexpr int MINIMAP_SIZE = 150;   
constexpr int MINIMAP_PADDING = 10; 

#define NumBuffers 3                 
const GLuint NumVertices = 8;        
const GLuint NumIndices = 6 * 2 * 3; 

constexpr float MIN_FOV = 15.0f;    
constexpr float MAX_FOV = 90.0f;    
constexpr float MIN_HEIGHT = 0.5f;  
constexpr float MAX_HEIGHT = 30.0f; 

struct LightingParams {
    glm::vec3 ambientLight;     
    float ambientIntensity;     
    bool isAmbientLightOn;      

    LightingParams() :
        ambientLight(1.0f),
        ambientIntensity(1.0f),
        isAmbientLightOn(true) {
    }
};

struct Light {
    glm::vec3 position;  
    glm::vec3 color;     
    float intensity;     

    Light() :
        position(5.0f, 5.0f, 0.0f),
        color(1.0f),
        intensity(1.0f) {
    }
};

LightingParams lighting;        
Light mainLight;                
GLuint program;                 
GLuint VAO;                     
GLuint Buffers[NumBuffers];     
Camera camera;                  
Camera topDownCamera;           

std::vector<ObjModel*> bolas;

struct InputState {
    bool isPressing = false;       
    double prevXpos = 0.0;         
    double prevYpos = 0.0;         
    double xPos = 0.0;             
    double yPos = 0.0;             
} input;

void print_error(int error, const char* description);
void init(void);
void display(const glm::mat4& view, const glm::mat4& projection);

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
        case GLFW_KEY_1:
            lighting.isAmbientLightOn = !lighting.isAmbientLightOn;
            break;
        }
    }
}

void scrollCallBack(GLFWwindow* window, double xoffset, double yoffset) {
    camera.fov = glm::clamp(camera.fov - static_cast<float>(yoffset), MIN_FOV, MAX_FOV);
}

void cursorCallBack(GLFWwindow* window, double xpos, double ypos) {
    input.xPos = xpos;
    input.yPos = ypos;

    if (input.isPressing) {
        const double deltaX = xpos - input.prevXpos;
        camera.rotateAroundTarget(static_cast<float>(deltaX) / -WIDTH * glm::pi<float>());
        input.prevXpos = xpos;

        const double deltaY = ypos - input.prevYpos;
        camera.height = glm::clamp(
            camera.height + static_cast<float>(-deltaY) / HEIGHT,
            MIN_HEIGHT,
            MAX_HEIGHT
        );
        camera.updatePosition();
    }
}

void mouseCallBack(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        input.isPressing = (action == GLFW_PRESS);
        if (input.isPressing) {
            input.prevXpos = input.xPos;
            input.prevYpos = input.yPos;
        }
    }
}

int main(void) {
    camera.updatePosition();

    GLFWwindow* window;
    glfwSetErrorCallback(print_error);

    if (!glfwInit()) {
        std::cerr << "Falha ao inicializar GLFW" << std::endl;
        return -1;
    }

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

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Falha ao inicializar GLEW" << std::endl;
        return -1;
    }

    glfwSetScrollCallback(window, scrollCallBack);
    glfwSetCursorPosCallback(window, cursorCallBack);
    glfwSetMouseButtonCallback(window, mouseCallBack);
    glfwSetKeyCallback(window, keyCallback);

    init();

    const GLint ambientLightLoc = glGetUniformLocation(program, "ambientLight");
    const GLint textureLoc = glGetUniformLocation(program, "tex");
    const GLint mvpLoc = glGetUniformLocation(program, "MVP");

    while (!glfwWindowShouldClose(window)) {
        const glm::vec3 finalAmbientLight = lighting.isAmbientLightOn ? lighting.ambientLight * lighting.ambientIntensity : glm::vec3(0.0f);

        glUseProgram(program);
        glUniform1i(textureLoc, 0);
        glUniform3fv(ambientLightLoc, 1, glm::value_ptr(finalAmbientLight));
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0, -2, 0));

        glViewport(0, 0, WIDTH, HEIGHT);
        const glm::mat4 view = camera.getViewMatrix();
        const glm::mat4 projection = camera.getProjectionMatrix(static_cast<float>(WIDTH) / HEIGHT);
        const glm::mat4 mvp = projection * view * model;

        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));
        display(view, projection);



        constexpr int MINIMAP_BORDER = 10;
        glViewport(WIDTH - MINIMAP_SIZE - MINIMAP_BORDER, HEIGHT - MINIMAP_SIZE - MINIMAP_BORDER,
            MINIMAP_SIZE, MINIMAP_SIZE);

        const glm::mat4 miniView = topDownCamera.getViewMatrix();
        const glm::mat4 miniProj = topDownCamera.getProjectionMatrix(1.0f);
        const glm::mat4 miniMVP = miniProj * miniView * model;

        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(miniMVP));
        display(miniView, miniProj);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

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

void init(void) {
    glEnable(GL_DEPTH_TEST);

    constexpr GLfloat tableWidth = 9.0f;   
    constexpr GLfloat tableHeight = 0.5f;  
    constexpr GLfloat tableDepth = 5.5f;   

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

    const GLfloat tableColor[3] = { 0.4f, 0.8f, 0.5f };
    GLfloat colors[NumVertices][3];
    for (int i = 0; i < NumVertices; i++) {
        std::copy(std::begin(tableColor), std::end(tableColor), colors[i]);
    }

    const GLuint indices[NumIndices] = {
        0, 1, 2, 1, 3, 2, 
        1, 3, 7, 1, 5, 7, 
        2, 3, 6, 3, 6, 7, 
        0, 2, 4, 2, 4, 6, 
        4, 5, 6, 5, 6, 7, 
        0, 1, 4, 1, 4, 5  
    };

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(NumBuffers, Buffers);

    glBindBuffer(GL_ARRAY_BUFFER, Buffers[0]);
    glBufferStorage(GL_ARRAY_BUFFER, sizeof(vertices), vertices, 0);

    glBindBuffer(GL_ARRAY_BUFFER, Buffers[1]);
    glBufferStorage(GL_ARRAY_BUFFER, sizeof(colors), colors, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffers[2]);
    glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, 0);

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

    const GLint coordsId = glGetProgramResourceLocation(program, GL_PROGRAM_INPUT, "vPosition");
    const GLint colorsId = glGetProgramResourceLocation(program, GL_PROGRAM_INPUT, "vColors");

    glBindBuffer(GL_ARRAY_BUFFER, Buffers[0]);
    glVertexAttribPointer(coordsId, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, Buffers[1]);
    glVertexAttribPointer(colorsId, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glEnableVertexAttribArray(coordsId);
    glEnableVertexAttribArray(colorsId);

    topDownCamera.position = glm::vec3(0.0f, 30.0f, 0.0f);
    topDownCamera.target = glm::vec3(0.0f, 0.0f, 0.0f);
    topDownCamera.up = glm::vec3(0.0f, 0.0f, -1.0f);
    topDownCamera.fov = 45.0f;

    try {
        for(int i = 0; i < 15; ++i) {
            createBall("PoolBalls/ball" + std::to_string(i + 1) + ".obj", ballsPosition[i]);
		}
    }
    catch (const std::exception& e) {
        std::cerr << "Falha ao carregar modelos das bolas: " << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}

void display(const glm::mat4& view, const glm::mat4& projection) {
    const GLint objectTypeLoc = glGetUniformLocation(program, "objectType");
    const GLint hasTextureLoc = glGetUniformLocation(program, "hasTexture");

    glUniform1i(objectTypeLoc, 0);
    glUniform1i(hasTextureLoc, false);
    glDrawElements(GL_TRIANGLES, NumIndices, GL_UNSIGNED_INT, nullptr);

    glUniform1i(objectTypeLoc, 1);
    glUniform1i(hasTextureLoc, true);

    for (const auto& bola : bolas) {
        bola->render(program, view, projection);
    }

    glBindVertexArray(VAO);
}

void print_error(int error, const char* description) {
    std::cerr << "Erro GLFW " << error << ": " << description << std::endl;
}