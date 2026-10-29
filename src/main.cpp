#include "main.h"

static void processInput(GLFWwindow* window);
static void framebufferSizeCallback(GLFWwindow* window, int width, int height);

static int framebufferWidth = 0;
static int framebufferHeight = 0;

// static constexpr float triangleVertices[] = {
//     -0.5f, -0.5f, 0.0f,
//     0.5f, -0.5f, 0.0f,
//     0.0f, 0.5f, 0.0f
// };

static constexpr GLfloat quadVertices[] = {
    0.5f, 0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    -0.5f, -0.5f, 0.0f,
    -0.5f, 0.5f, 0.0f
};

static constexpr GLuint triangleIndices[] = {
    0, 1, 2,
    2, 3, 0
};

static constexpr auto vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
void main()
{
    gl_Position = vec4(aPos, 1.0);
}
)";

static constexpr auto fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
void main()
{
    FragColor = vec4(1.0, 0.5, 0.2, 1.0);
}
)";

int main() {
    if (glfwInit() != GLFW_TRUE) return -1; // check glfw operating environment

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);  // OpenGl major version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);  // OpenGl minor version
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // opengl core profile
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // before glfw3.3
#endif

    GLFWwindow* window =
        glfwCreateWindow(640, 480, "Hello World", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window); // allocate a context to the current thread

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }   // initialize GLAD

    // >>> compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    if (vertexShader == 0) {
        std::cerr << "Failed to create a vertex shader" << std::endl;
        glfwTerminate();
        return -1;
    }
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLint vertexCompiled = GL_FALSE;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertexCompiled);
    if (vertexCompiled != GL_TRUE) {
        GLint infoLogLength = 0;
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &infoLogLength);
        std::string infoLog(infoLogLength, '\0');
        glGetShaderInfoLog(vertexShader, infoLogLength, nullptr, infoLog.data());
        std::cerr << "Failed to compile vertex shader:\n"
                  << infoLog << std::endl;
        glDeleteShader(vertexShader);
        glfwTerminate();
        return -1;
    }
    std::cout << "Vertex shader compilation succeeded\n";
    // <<< compile vertex shader

    // >>> compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    if (fragmentShader == 0) {
        std::cerr << "Failed to create a fragment shader" << std::endl;
        glfwTerminate();
        return -1;
    }
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    GLint fragmentCompiled = GL_FALSE;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fragmentCompiled);
    if (fragmentCompiled != GL_TRUE) {
        GLint infoLogLength = 0;
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &infoLogLength);
        std::string infoLog(infoLogLength, '\0');
        glGetShaderInfoLog(fragmentShader, infoLogLength, nullptr, infoLog.data());
        std::cerr << "Failed to compile fragment shader:\n"
                    << infoLog << std::endl;
        glDeleteShader(fragmentShader);
        glDeleteShader(vertexShader);
        glfwTerminate();
        return -1;
    }
    std::cout << "Fragment shader compilation succeeded\n";
    // <<< compile fragment shader

    const GLuint shaderProgram = glCreateProgram();
    if (shaderProgram == 0) {
        std::cerr << "Failed to create a shader program" << std::endl;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glfwTerminate();
        return -1;
    }

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLint linked = GL_FALSE;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);
    if (linked != GL_TRUE) {
        GLint programLogLength = 0;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &programLogLength);
        std::string programLog(programLogLength, '\0');
        glGetProgramInfoLog(shaderProgram, programLogLength, nullptr, programLog.data());
        std::cerr << "Failed to link shader program:\n"
                  << programLog << std::endl;
        glDeleteProgram(shaderProgram);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glfwTerminate();
        return -1;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    std::cout << "Shader program link succeeded\n";

    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    glViewport(0, 0, framebufferWidth, framebufferHeight);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // set clear color
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    GLuint ebo = 0;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo); // after binding the vao
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangleIndices), triangleIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glClear(GL_COLOR_BUFFER_BIT);   // fill the buffer with setted color
        glUseProgram(shaderProgram);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        glfwSwapBuffers(window);    // swap the image prepared in the back with the buffer of the display table
        glfwPollEvents(); // check mouse/keyborad input
    }

    glUseProgram(0);
    glDeleteProgram(shaderProgram);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void framebufferSizeCallback(GLFWwindow* window, const int width, const int height) {
    glViewport(0, 0, width, height);
    std::cout << "Current framebuffer size: " << width << "x" << height << std::endl;
}