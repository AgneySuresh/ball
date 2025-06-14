#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdbool.h>
#define PI 3.14159265358979323846

typedef struct
{
    float x, y;
    float vx,vy;
    float ax, ay;
    float radius;
} Ball;

// Mouse interaction globals
bool dragging = false;
float mouseX_NDC = 0.0f, mouseY_NDC = 0.0f;

const char* vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec2 aPos;\n"
    "void main() {\n"
    "    gl_Position = vec4(aPos, 0.0, 1.0);\n"
    "    gl_PointSize = 120.0;\n"
    "}\0";

const char* fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "    FragColor = vec4(1.0, 0.85, 0.73, 1.0);\n"
    "}\0";

void generateCircleVertices(float* vertices, int segments, float radius, float centerX, float centerY) {
    vertices[0] = centerX;  // Center point
    vertices[1] = centerY;

    for (int i = 0; i <= segments; i++) {
        float angle = (2.0f * PI * i) / segments;
        vertices[2 + i * 2]     = centerX + cosf(angle) * radius;
        vertices[2 + i * 2 + 1] = centerY + sinf(angle) * radius;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
    glViewport(0, 0, width, height);
}

Ball ball = {
    .x = 0.0f, .y = 0.8f,       // initial top-center
    .vx = 0.0f, .vy = 0.0f,
    .ax = 0.0f, .ay = -9.8f,    // gravity
    .radius = 0.0f
};

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Convert to OpenGL NDC coordinates (-1 to 1)
    mouseX_NDC = (float)xpos / width * 2.0f - 1.0f;
    mouseY_NDC = 1.0f - (float)ypos / height * 2.0f;  // flip y
    
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            float dx = mouseX_NDC - ball.x;
            float dy = mouseY_NDC - ball.y;
            float distSq = dx*dx + dy*dy;
            if (distSq <= 0.08f) {
                dragging = true;
            }
        }
        else if (action == GLFW_RELEASE) {
            dragging = false;
        }
    }
    
}

int main(){
    if (!glfwInit()) //initalisation
    {
        fprintf(stderr,"Failed to initialize GLFW/n");
        return -1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //creating a window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Simulator", NULL, NULL);
    if (window == NULL){
        fprintf(stderr, "Failed to create GLFW window/n");
        glfwTerminate();
        return -1;
    }
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    //loading opengl function using GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        fprintf(stderr,"Failed to initialize GLAD");
        return -1;
    }
    // Compile vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Compile fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Link shaders to shader program
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Delete shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
// Tell OpenGL how to interpret each vertex (2 floats per vertex)
    glVertexAttribPointer(
        0,                  // layout(location = 0)
        2,                  // 2 components (x, y)
        GL_FLOAT,           // each component is a float
        GL_FALSE,           // don't normalize
        2 * sizeof(float),  // stride between each vertex
        (void*)0            // offset is 0
    );
    glEnableVertexAttribArray(0);

    float lastTime = glfwGetTime();
    glViewport(0, 0, 800, 600);
    glPointSize(20.0f); // Makes the point visibly large

    int segments = 40;
    float radius = 0.2f;

    float circleVertices[2 * (segments + 2)];
    generateCircleVertices(circleVertices, segments, radius, ball.x, ball.y);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(circleVertices), circleVertices, GL_DYNAMIC_DRAW);

    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, 1);
        
        
        glfwPollEvents();
        if (dragging) {
            ball.x = mouseX_NDC;
            ball.y = mouseY_NDC;
            ball.vx = 0.0f;
            ball.vy = 0.0f;
        }

        //physics sim
        ball.vx += ball.ax * deltaTime;
        ball.vy += ball.ay * deltaTime;
        ball.x += ball.vx * deltaTime;
        ball.y += ball.vy * deltaTime;

        // wall boundaries
        float top = 1.0f;
        float bottom = -1.0f;
        float left = -1.0f;
        float right = 1.0f;

        // Y collisions
        if (ball.y - radius <= bottom) {
            ball.y = bottom + radius;
            ball.vy *= -0.8f; // bounce with damping
        }
        if (ball.y + radius >= top) {
            ball.y = top - radius;
            ball.vy *= -0.8f;
        }

        // X collisions
        if (ball.x - radius <= left) {
            ball.x = left + radius;
            ball.vx *= -0.8f;
        }
        if (ball.x + radius >= right) {
            ball.x = right - radius;
            ball.vx *= -0.8f;
        }
        

        generateCircleVertices(circleVertices, segments, radius, ball.x, ball.y);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(circleVertices), circleVertices);


        //rendering
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Update the VBO with new ball position
        generateCircleVertices(circleVertices, segments, radius, ball.x, ball.y);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(circleVertices), circleVertices);

        // Draw the ball
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, segments + 2);


        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}