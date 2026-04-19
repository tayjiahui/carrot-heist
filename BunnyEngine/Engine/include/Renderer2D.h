#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "Renderer/Shader.h"

class Renderer2D
{
public:
    void Init();
    void Shutdown();

    // Draw a colored quad (no texture)
    void DrawQuad(const glm::vec2& position,
        const glm::vec2& size,
        float            rotation,
        const glm::vec4& color);

    // Draw a textured quad
    void DrawSprite(const glm::vec2& position,
        const glm::vec2& size,
        float            rotation,
        unsigned int     textureID,
        const glm::vec4& tint = glm::vec4(1.0f));

    void SetViewProjection(const glm::mat4& vp);

    // Draw a specific spritesheet frame
    void DrawFrame(const glm::vec2& position,
        const glm::vec2& size,
        float            rotation,
        unsigned int     textureID,
        const glm::vec2& uvMin,
        const glm::vec2& uvMax,
        const glm::vec4& tint = glm::vec4(1.0f),
        bool flipX = false);

private:
    unsigned int m_VAO = 0;
    unsigned int m_VBO = 0;
    unsigned int m_WhiteTex = 0; // 1x1 white texture fallback

    Shader* m_Shader = nullptr;
    glm::mat4 m_VP = glm::mat4(1.0f);
};