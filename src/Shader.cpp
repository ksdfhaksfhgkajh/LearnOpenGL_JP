#include "Shader.h"

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    std::ifstream vertexFile(vertexPath, std::ios::binary);
    std::ifstream fragmentFile(fragmentPath, std::ios::binary);
    if (!vertexFile.is_open() || !fragmentFile.is_open()) {
        throw std::runtime_error("Failed to open vertex shader");
    }
    std::string vertexCode(std::istreambuf_iterator{vertexFile}, std::istreambuf_iterator<char>{});
    std::string fragmentCode(std::istreambuf_iterator{fragmentFile}, std::istreambuf_iterator<char>{});

    const GLchar* cvertexCode = std::move(vertexCode).c_str();
    const GLchar* cfragmentCode = std::move(fragmentCode).c_str();

    const GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    if (vertexShader == 0) {
        throw std::runtime_error("Vertex shader creation failed");
    }
    glShaderSource(vertexShader, 1, &cvertexCode, nullptr);
    glCompileShader(vertexShader);
    GLint vertexCompiled = GL_FALSE;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertexCompiled);
    if (vertexCompiled != GL_TRUE) {
        GLint infoLength{};
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &infoLength);
        std::string vertexInfoLog(infoLength, '\0');
        glGetShaderInfoLog(vertexShader, infoLength, &infoLength, vertexInfoLog.data());
        throw std::runtime_error("Failed to compile Vertex Shader:\n" + vertexInfoLog);
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    if (fragmentShader == 0) {
        throw std::runtime_error("Fragment shader creation failed");
    }
    glShaderSource(fragmentShader, 1, &cfragmentCode, nullptr);
    glCompileShader(fragmentShader);
    GLint fragmentCompiled = GL_FALSE;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fragmentCompiled);
    if (fragmentCompiled != GL_TRUE) {
        GLint infoLength{};
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &infoLength);
        std::string fragmentInfoLog(infoLength, '\0');
        glGetShaderInfoLog(fragmentShader, infoLength, &infoLength, fragmentInfoLog.data());
        throw std::runtime_error("Failed to compile Fragment Shader:\n" + fragmentInfoLog);
    }

    this->mProgramID = glCreateProgram();
    if (this->mProgramID == 0) {
        throw std::runtime_error("Failed to create a new shader program");
    }
    glAttachShader(this->mProgramID, vertexShader);
    glAttachShader(this->mProgramID, fragmentShader);
    glLinkProgram(this->mProgramID);
    GLint linked = GL_FALSE;
    glGetProgramiv(this->mProgramID, GL_LINK_STATUS, &linked);
    if (linked != GL_TRUE) {
        GLint infoLength{};
        glGetProgramiv(this->mProgramID, GL_INFO_LOG_LENGTH, &infoLength);
        std::string programInfoLog(infoLength, '\0');
        glGetProgramInfoLog(this->mProgramID, infoLength, &infoLength, programInfoLog.data());
        throw std::runtime_error("Failed to link shader program:\n" + programInfoLog);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::~Shader() {
    glDeleteProgram(this->mProgramID);
}

void Shader::use() const {
    glUseProgram(this->mProgramID);
}

void Shader::setBool(const std::string& name, const bool value) const {
    glUniform1i(glGetUniformLocation(this->mProgramID, name.c_str()), static_cast<int>(value));
}

void Shader::setInt(const std::string& name, const int value) const {
    glUniform1i(glGetUniformLocation(this->mProgramID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, const float value) const {
    glUniform1f(glGetUniformLocation(this->mProgramID, name.c_str()), value);
}
