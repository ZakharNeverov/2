#include <GL/glew.h>            // For loading modern OpenGL functions
#include <GL/freeglut.h>        // For GLUT
#include <GL/freeglut_ext.h>
#include "camera_control.h"
#include "gui_control.h"
#include "imgui/imgui.h"

#include <glm/glm.hpp>          // For matrices and vectors
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// Texture IDs
GLuint textureID;
GLuint planeTextureID;

// Light properties
float lightPosition[] = {1.0f, 1.0f, 1.0f};
float lightAmbient[] = {0.2f, 0.2f, 0.2f, 1.0f};
float lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};

float lightBaseColor[] = {0.8f, 0.8f, 0.8f};
float lightDiffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
float lightIntensity = 1.0f;

float scale = 1.0f;
float targetScale = 1.0f;
bool isScaling = false;

GLuint shaderProgram; // Shader program

// VAOs and VBOs
GLuint cubeVAO, cubeVBO;
GLuint planeVAO, planeVBO;
GLuint sphereVAO = 0;
int sphereVertexCount = 0;
GLuint coneVAO = 0;
int coneVertexCount = 0;


void generateSphere(float radius, int sectorCount, int stackCount) {
    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;

    float x, y, z, xy;                              // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius;    // normal
    float s, t;                                     // texCoord

    float sectorStep = 2 * M_PI / sectorCount;
    float stackStep = M_PI / stackCount;
    float sectorAngle, stackAngle;

    for(int i = 0; i <= stackCount; ++i) {
        stackAngle = M_PI / 2 - i * stackStep;        // from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);             // r * cos(u)
        z = radius * sinf(stackAngle);              // r * sin(u)

        for(int j = 0; j <= sectorCount; ++j) {
            sectorAngle = j * sectorStep;           // from 0 to 2pi

            // vertex position
            x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // normalized vertex normal
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            vertices.push_back(nx);
            vertices.push_back(ny);
            vertices.push_back(nz);

            // vertex tex coord between [0, 1]
            s = (float)j / sectorCount;
            t = (float)i / stackCount;
            vertices.push_back(s);
            vertices.push_back(t);
        }
    }

    // indices
    int k1, k2;
    for(int i = 0; i < stackCount; ++i){
        k1 = i * (sectorCount + 1);     // beginning of current stack
        k2 = k1 + sectorCount + 1;      // beginning of next stack

        for(int j = 0; j < sectorCount; ++j, ++k1, ++k2){
            if(i != 0){
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }
            if(i != (stackCount - 1)){
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    sphereVertexCount = indices.size();

    GLuint sphereVBO, sphereEBO;
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);

    glBindVertexArray(sphereVAO);

    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    // Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    // Normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    // Texture Coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

    glBindVertexArray(0);
}
void generateCone(float radius, float height, int sectorCount) {
    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;

    float sectorStep = 2 * M_PI / sectorCount;
    float sectorAngle;

    // Предварительный расчет нормалей боковых граней
    float slantHeight = sqrt(radius * radius + height * height);
    float normalLength = 1.0f / slantHeight;
    float nx, ny, nz;

    // Генерация вершин основания (нормали вниз)
    for (int i = 0; i < sectorCount; ++i) {
        sectorAngle = i * sectorStep;
        float x = radius * cosf(sectorAngle);
        float z = radius * sinf(sectorAngle);
        float y = 0.0f;

        // Позиция вершины
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);

        // Нормаль для основания
        vertices.push_back(0.0f);
        vertices.push_back(-1.0f);
        vertices.push_back(0.0f);

        // Текстурные координаты
        vertices.push_back((x / radius + 1.0f) * 0.5f);
        vertices.push_back((z / radius + 1.0f) * 0.5f);
    }

    // Центр основания
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    // Нормаль для центра основания
    vertices.push_back(0.0f);
    vertices.push_back(-1.0f);
    vertices.push_back(0.0f);
    // Текстурные координаты для центра основания
    vertices.push_back(0.5f);
    vertices.push_back(0.5f);

    // Индексы для основания (треугольники фан)
    for (int i = 0; i < sectorCount; ++i) {
        indices.push_back(i);
        indices.push_back((i + 1) % sectorCount);
        indices.push_back(sectorCount); // Центр основания
    }

    // Генерация вершин боковых граней (нормали по боковой поверхности)
    for (int i = 0; i < sectorCount; ++i) {
        sectorAngle = i * sectorStep;
        float x = radius * cosf(sectorAngle);
        float z = radius * sinf(sectorAngle);
        float y = 0.0f;

        // Позиция вершины основания для боковых граней
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);

        // Нормаль для боковой грани
        nx = x * normalLength;
        ny = radius * normalLength;
        nz = z * normalLength;
        glm::vec3 normal(nx, ny, nz);
        normal = glm::normalize(normal);
        vertices.push_back(normal.x);
        vertices.push_back(normal.y);
        vertices.push_back(normal.z);

        // Текстурные координаты для боковых граней
        vertices.push_back((float)i / sectorCount);
        vertices.push_back(0.0f);
    }

    // Вершина апекса
    vertices.push_back(0.0f);
    vertices.push_back(height);
    vertices.push_back(0.0f);
    // Нормаль для апекса (для сглаженного освещения используем нормали боковых граней)
    // Однако, нормаль апекса как отдельная вершина не имеет смысла для сглаживания
    // Поэтому можно оставить нормаль апекса как (0,1,0) или использовать технику нормалей без апекса
    vertices.push_back(0.0f);
    vertices.push_back(1.0f);
    vertices.push_back(0.0f);
    // Текстурные координаты апекса
    vertices.push_back(0.5f);
    vertices.push_back(1.0f);

    int sideBaseStart = sectorCount + 1; // После вершин основания и центра основания
    int apexIndex = sideBaseStart + sectorCount; // Индекс апекса

    // Индексы для боковых граней (треугольники)
    for (int i = 0; i < sectorCount; ++i) {
        indices.push_back(sideBaseStart + i);
        indices.push_back(apexIndex);
        indices.push_back(sideBaseStart + ((i + 1) % sectorCount));
    }

    // Обновляем количество индексов
    coneVertexCount = indices.size();

    GLuint coneVBO, coneEBO;
    glGenVertexArrays(1, &coneVAO);
    glGenBuffers(1, &coneVBO);
    glGenBuffers(1, &coneEBO);

    glBindVertexArray(coneVAO);

    glBindBuffer(GL_ARRAY_BUFFER, coneVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, coneEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    // Позиции
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);

    // Нормали
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    // Текстурные координаты
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

    glBindVertexArray(0);
}



// Function to load and compile shaders
GLuint loadShaders(const char* vertex_file_path, const char* fragment_file_path)
{
    // Create shader program
    GLuint programID = glCreateProgram();

    // Create vertex shader
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    // Load shader code from file and compile it
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open()){
        std::stringstream sstr;
        sstr << VertexShaderStream.rdbuf();
        VertexShaderCode = sstr.str();
        VertexShaderStream.close();
    }else{
        std::cerr << "Unable to access vertex shader file: " << vertex_file_path << std::endl;
        return 0;
    }

    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(vertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(vertexShaderID);

    // Check for vertex shader compilation errors
    GLint Result = GL_FALSE;
    int InfoLogLength;
    glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(vertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        std::cerr << "Vertex shader compilation Error: " << &VertexShaderErrorMessage[0] << std::endl;
    }

    // Create fragment shader
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    // Load shader code from file and compile it
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open()){
        std::stringstream sstr;
        sstr << FragmentShaderStream.rdbuf();
        FragmentShaderCode = sstr.str();
        FragmentShaderStream.close();
    }else{
        std::cerr << "Unable to access fragment shader file: " << fragment_file_path << std::endl;
        return 0;
    }

    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(fragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(fragmentShaderID);

    // Check for fragment shader compilation errors
    glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(fragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        std::cerr << "Fragment shader compilation Error: " << &FragmentShaderErrorMessage[0] << std::endl;
    }

    // Attach shaders to program and link it
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);
    glLinkProgram(programID);

    // Check program
    glGetProgramiv(programID, GL_LINK_STATUS, &Result);
    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> ProgramErrorMessage(InfoLogLength+1);
        glGetProgramInfoLog(programID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        std::cerr << "Shader program linking error: " << &ProgramErrorMessage[0] << std::endl;
    }

    // Delete shaders after linking
    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);

    return programID;
}

// Function to generate a checkerboard texture
GLuint generateCheckerboardTexture(int texWidth, int texHeight, GLubyte color1[3], GLubyte color2[3]) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    GLubyte* textureData = new GLubyte[texWidth * texHeight * 3];

    for (int i = 0; i < texHeight; i++) {
        for (int j = 0; j < texWidth; j++) {
            int c = (((i & 8) == 0) ^ ((j & 8) == 0));
            GLubyte* color = c ? color1 : color2;
            int index = (i * texWidth + j) * 3;
            textureData[index + 0] = color[0];
            textureData[index + 1] = color[1];
            textureData[index + 2] = color[2];
        }
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, textureData);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    delete[] textureData;

    return textureID;
}

// Vertex data for the cube
GLfloat cubeVertices[] = {
    // Positions          // Normals           // Texture Coords
    // Back face
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f, // Bottom-left
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f, // Top-right
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f, // Bottom-right     
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f, // Top-right
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f, // Bottom-left
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f, // Top-left
    // Front face
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f, // Bottom-left
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f, // Bottom-right
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f, // Top-right
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f, // Top-right
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f, // Top-left
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f, // Bottom-left
    // Left face
    -0.5f,  0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  1.0f, 0.0f, // Top-right
    -0.5f,  0.5f, -0.5f,  -1.0f, 0.0f,  0.0f,  1.0f, 1.0f, // Top-left
    -0.5f, -0.5f, -0.5f,  -1.0f, 0.0f,  0.0f,  0.0f, 1.0f, // Bottom-left
    -0.5f, -0.5f, -0.5f,  -1.0f, 0.0f,  0.0f,  0.0f, 1.0f, // Bottom-left
    -0.5f, -0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  0.0f, 0.0f, // Bottom-right
    -0.5f,  0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  1.0f, 0.0f, // Top-right
    // Right face
     0.5f,  0.5f,  0.5f,   1.0f, 0.0f,  0.0f,  1.0f, 0.0f, // Top-left
     0.5f, -0.5f, -0.5f,   1.0f, 0.0f,  0.0f,  0.0f, 1.0f, // Bottom-right
     0.5f,  0.5f, -0.5f,   1.0f, 0.0f,  0.0f,  1.0f, 1.0f, // Top-right         
     0.5f, -0.5f, -0.5f,   1.0f, 0.0f,  0.0f,  0.0f, 1.0f, // Bottom-right
     0.5f,  0.5f,  0.5f,   1.0f, 0.0f,  0.0f,  1.0f, 0.0f, // Top-left
     0.5f, -0.5f,  0.5f,   1.0f, 0.0f,  0.0f,  0.0f, 0.0f, // Bottom-left     
    // Bottom face
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f, // Top-right
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f, // Top-left
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f, // Bottom-left
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f, // Bottom-left
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f, // Bottom-right
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f, // Top-right
    // Top face
    -0.5f,  0.5f, -0.5f,   0.0f, 1.0f,  0.0f,  0.0f, 1.0f, // Top-left
     0.5f,  0.5f , 0.5f,   0.0f, 1.0f,  0.0f,  1.0f, 0.0f, // Bottom-right
     0.5f,  0.5f ,-0.5f,   0.0f, 1.0f,  0.0f,  1.0f, 1.0f, // Top-right     
     0.5f,  0.5f , 0.5f,   0.0f, 1.0f,  0.0f,  1.0f, 0.0f, // Bottom-right
    -0.5f,  0.5f ,-0.5f,   0.0f, 1.0f,  0.0f,  0.0f, 1.0f, // Top-left
    -0.5f,  0.5f , 0.5f,   0.0f, 1.0f,  0.0f,  0.0f, 0.0f  // Bottom-left        
};


// Vertex data for the plane
GLfloat planeVertices[] = {
    // Positions            // Normals         // Texture Coords
    -5.0f, 0.0f, -5.0f,    0.0f, 1.0f, 0.0f,   0.0f,  5.0f,
     5.0f, 0.0f,  5.0f,    0.0f, 1.0f, 0.0f,   5.0f,  0.0f,
     5.0f, 0.0f, -5.0f,    0.0f, 1.0f, 0.0f,   5.0f,  5.0f,
     5.0f, 0.0f,  5.0f,    0.0f, 1.0f, 0.0f,   5.0f,  0.0f,
    -5.0f, 0.0f, -5.0f,    0.0f, 1.0f, 0.0f,   0.0f,  5.0f,
    -5.0f, 0.0f,  5.0f,    0.0f, 1.0f, 0.0f,   0.0f,  0.0f
};

void initVAOs() {
    generateSphere(0.5f, 36, 18); // radius, sectors, stacks
    generateCone(0.5f, 1.0f, 36); // radius, height, sectors
    // Cube VAO and VBO
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    // Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    // Normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    // Texture Coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

    glBindVertexArray(0);

    // Plane VAO and VBO
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

    // Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    // Normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    // Texture Coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

    glBindVertexArray(0);
}
void enableBlending() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void disableBlending() {
    glDisable(GL_BLEND);
}

void drawScene() {
    // Включаем смешивание для прозрачных объектов
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Используем шейдерную программу
    glUseProgram(shaderProgram);

    // Получаем локации uniform-переменных
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "modelMatrix");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "viewMatrix");
    GLuint projLoc = glGetUniformLocation(shaderProgram, "projectionMatrix");
    GLuint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
    GLuint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
    GLuint lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
    GLuint lightIntensityLoc = glGetUniformLocation(shaderProgram, "lightIntensity");
    GLuint alphaLoc = glGetUniformLocation(shaderProgram, "alpha");

    // Локации для материала
    GLuint materialSpecularLoc = glGetUniformLocation(shaderProgram, "materialSpecular");
    GLuint materialDiffuseLoc = glGetUniformLocation(shaderProgram, "materialDiffuse");
    GLuint materialAmbientLoc = glGetUniformLocation(shaderProgram, "materialAmbient");
    GLuint materialShininessLoc = glGetUniformLocation(shaderProgram, "materialShininess");
    GLuint useTextureLoc = glGetUniformLocation(shaderProgram, "useTexture");

    // Устанавливаем матрицы просмотра и проекции
    glm::mat4 view = getCameraViewMatrix();
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)w / (float)h, 1.0f, 100.0f);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Устанавливаем позицию света и позицию камеры
    glm::vec3 lightPos(lightPosition[0], lightPosition[1], lightPosition[2]);
    glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
    glm::vec3 cameraPos = getCameraPosition();
    glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPos));

    // Устанавливаем цвет и интенсивность света
    glUniform3fv(lightColorLoc, 1, lightBaseColor);
    glUniform1f(lightIntensityLoc, lightIntensity);

    // --- Рисуем куб (непрозрачный) ---
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.0f, 2.0f, 0.0f));
        model = glm::scale(model, glm::vec3(scale, scale, scale));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        // Устанавливаем параметры материала
        //glUniform3f(materialSpecularLoc, 0.5f, 0.5f, 0.5f); // Средний спекулярный цвет
        glUniform3f(materialDiffuseLoc, 1.0f, 1.0f, 1.0f);  // Диффузный цвет (будет использоваться текстура)
        glUniform3f(materialAmbientLoc, 0.3f, 0.3f, 0.3f);  // Фоновый цвет
        glUniform1f(materialShininessLoc, 32.0f);           // Средний коэффициент блеска

        // Используем текстуру
        glUniform1i(useTextureLoc, GL_TRUE);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glUniform1i(glGetUniformLocation(shaderProgram, "textureSampler"), 0);

        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

    // --- Рисуем плоскость (очень отражающая) ---
    {
        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        // Устанавливаем параметры материала
        glUniform3f(materialSpecularLoc, 1.0f, 1.0f, 1.0f); // Высокий спекулярный цвет для сильного отражения
        glUniform3f(materialDiffuseLoc, 1.0f, 1.0f, 1.0f);  // Диффузный цвет (будет использоваться текстура)
        glUniform3f(materialAmbientLoc, 0.3f, 0.3f, 0.3f);  // Фоновый цвет
        glUniform1f(materialShininessLoc, 128.0f);          // Высокий коэффициент блеска

        // Используем текстуру
        glUniform1i(useTextureLoc, GL_TRUE);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, planeTextureID);
        glUniform1i(glGetUniformLocation(shaderProgram, "textureSampler"), 0);

        glBindVertexArray(planeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    // --- Рисуем конус (металлический, блестящий) ---
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 2.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        // Устанавливаем параметры материала
        glUniform3f(materialSpecularLoc, 1.0f, 1.0f, 1.0f); // Белый спекулярный цвет
        glUniform3f(materialDiffuseLoc, 1.0f, 0.8f, 0.0f);  // Жёлтый диффузный цвет
        glUniform3f(materialAmbientLoc, 0.3f, 0.3f, 0.3f);  // Фоновый цвет
        glUniform1f(materialShininessLoc, 164.0f);          // Высокий коэффициент блеска

        // Не используем текстуру
        glUniform1i(useTextureLoc, GL_FALSE);

        // Устанавливаем альфа-канал (полностью непрозрачный)
        glUniform1f(alphaLoc, 1.0f);

        glBindVertexArray(coneVAO);
        glDrawElements(GL_TRIANGLES, coneVertexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    // --- Рисуем сферу (прозрачная и отражающая) ---
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, 2.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        // Устанавливаем параметры материала
        glUniform3f(materialSpecularLoc, 0.8f, 0.8f, 0.8f); // Высокий спекулярный цвет для отражения
        glUniform3f(materialDiffuseLoc, 0.0f, 1.0f, 0.0f);  // Зелёный диффузный цвет
        glUniform3f(materialAmbientLoc, 0.3f, 0.3f, 0.3f);  // Фоновый цвет
        glUniform1f(materialShininessLoc, 64.0f);           // Средний коэффициент блеска

        // Устанавливаем альфа-канал (70% непрозрачность)
        glUniform1f(alphaLoc, 0.7f);

        // Не используем текстуру
        glUniform1i(useTextureLoc, GL_FALSE);

        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, sphereVertexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    // Отключаем смешивание после рисования
    glDisable(GL_BLEND);

    // Отключаем шейдерную программу
    glUseProgram(0);
}



void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    applyCameraView();

    lightDiffuse[0] = lightBaseColor[0] * lightIntensity;
    lightDiffuse[1] = lightBaseColor[1] * lightIntensity;
    lightDiffuse[2] = lightBaseColor[2] * lightIntensity;
    lightDiffuse[3] = 1.0f;

    // Draw the scene
    drawScene();

    renderGUI();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (h == 0)
        h = 1;

    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45.0, (float)w / (float)h, 1.0, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    ImGuiIO &io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)w, (float)h);
}

void timer(int value) {
    if (isScaling) {

        if (scale > targetScale) {
            scale -= 0.01f;
            if (scale < targetScale) {
                scale = targetScale;
                isScaling = false;
            }
        }
        glutPostRedisplay();
        glutTimerFunc(16, timer, 0);
    }
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'i':
        lightIntensity = std::min(lightIntensity + 0.1f, 1.0f);
        break;
    case 'k':
        lightIntensity = std::max(lightIntensity - 0.1f, 0.0f);
        break;
    case 'a':
        lightPosition[0] -= 0.1f;
        break;
    case 'd':
        lightPosition[0] += 0.1f;
        break;
    case 'q':
        lightPosition[1] += 0.1f;
        break;
    case 'e':
        lightPosition[1] -= 0.1f;
        break;
    case 'w':
        lightPosition[2] += 0.1f;
        break;
    case 's':
        lightPosition[2] -= 0.1f;
        break;
    case 'r':
        lightBaseColor[0] = 1.0f;
        lightBaseColor[1] = 0.0f;
        lightBaseColor[2] = 0.0f;
        break;
    case 'g':
        lightBaseColor[0] = 0.0f;
        lightBaseColor[1] = 1.0f;
        lightBaseColor[2] = 0.0f;
        break;
    case 'b':
        lightBaseColor[0] = 0.0f;
        lightBaseColor[1] = 0.0f;
        lightBaseColor[2] = 1.0f;
        break;
    case 27:
        exit(0);
        break;
    }
    glutPostRedisplay();
}

int main(int argc, char **argv) {
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Scene with Materials and Lighting");

    // Initialize GLEW
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(err) << std::endl;
        return -1;
    }

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLEW Version: " << glGetString(GLEW_VERSION) << std::endl;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // Initialize textures
    GLubyte cubeColor1[3] = {255, 255, 255}; // white
    GLubyte cubeColor2[3] = {0, 0, 0};       // black
    textureID = generateCheckerboardTexture(64, 64, cubeColor1, cubeColor2);

    GLubyte planeColor1[3] = {192, 192, 192}; // light gray
    GLubyte planeColor2[3] = {255, 255, 255}; // white
    planeTextureID = generateCheckerboardTexture(64, 64, planeColor1, planeColor2);

    // Initialize VAOs and VBOs
    initVAOs();

    initGUI();

    // Load shaders
    shaderProgram = loadShaders("../shaders/vertex_shader.glsl", "../shaders/fragment_shader.glsl");
    if (shaderProgram == 0) {
        std::cerr << "Shader loading error." << std::endl;
        return -1;
    }

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(handleMouse);
    glutMotionFunc(handleMouseMotion);
    glutMouseWheelFunc(handleMouseWheel);

    glutMainLoop();
    shutdownGUI();

    return 0;
}
