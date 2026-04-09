#pragma once
#include <glad/glad.h>
#include <string>
#include <glm/glm.hpp>

class Shader
{
public:
    Shader(const char* vertSrc, const char* fragSrc);
    ~Shader();

    void Bind()   const;
    void Unbind() const;

    void SetMat4(const std::string& name, const glm::mat4& val) const;
    void SetInt(const std::string& name, int val)              const;
    void SetFloat4(const std::string& name, const glm::vec4& val) const;

    unsigned int GetID() const { return m_ID; }

private:
    unsigned int m_ID = 0;
    unsigned int CompileShader(unsigned int type, const char* src);
};