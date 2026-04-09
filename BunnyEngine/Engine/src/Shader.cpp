#include "Shader.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

Shader::Shader(const char* vertSrc, const char* fragSrc)
{
    unsigned int vert = CompileShader(GL_VERTEX_SHADER, vertSrc);
    unsigned int frag = CompileShader(GL_FRAGMENT_SHADER, fragSrc);

    m_ID = glCreateProgram();
    glAttachShader(m_ID, vert);
    glAttachShader(m_ID, frag);
    glLinkProgram(m_ID);

    int success;
    glGetProgramiv(m_ID, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(m_ID, 512, nullptr, log);
        std::cerr << "[Shader] Link error: " << log << "\n";
    }

    glDeleteShader(vert);
    glDeleteShader(frag);
}

Shader::~Shader()
{
    glDeleteProgram(m_ID);
}

void Shader::Bind()   const { glUseProgram(m_ID); }
void Shader::Unbind() const { glUseProgram(0); }

void Shader::SetMat4(const std::string& name, const glm::mat4& val) const {
    glUniformMatrix4fv(glGetUniformLocation(m_ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(val));
}

void Shader::SetInt(const std::string& name, int val) const {
    glUniform1i(glGetUniformLocation(m_ID, name.c_str()), val);
}

void Shader::SetFloat4(const std::string& name, const glm::vec4& val) const {
    glUniform4f(glGetUniformLocation(m_ID, name.c_str()), val.x, val.y, val.z, val.w);
}

unsigned int Shader::CompileShader(unsigned int type, const char* src)
{
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(id, 512, nullptr, log);
        std::cerr << "[Shader] Compile error: " << log << "\n";
    }
    return id;
}