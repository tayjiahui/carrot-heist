#include "Renderer/Renderer2D.h"
#include "Renderer/Shader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

// -------------------------------------------------------
// Shader sources
// -------------------------------------------------------
static const char* s_VertSrc = R"(
#version 460 core
layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec2 v_TexCoord;

void main()
{
    v_TexCoord  = a_TexCoord;
    gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 0.0, 1.0);
}
)";

static const char* s_FragSrc = R"(
#version 460 core
in vec2 v_TexCoord;

uniform sampler2D u_Texture;
uniform vec4      u_Tint;

out vec4 FragColor;

void main()
{
    FragColor = texture(u_Texture, v_TexCoord) * u_Tint;
}
)";

// -------------------------------------------------------
// Unit quad vertices: position (x,y) + texcoord (u,v)
// -------------------------------------------------------
static float s_QuadVertices[] = {
    // pos       // uv
    -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  1.0f, 1.0f,

    -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.0f, 1.0f,
};

void Renderer2D::Init()
{
    m_Shader = new Shader(s_VertSrc, s_FragSrc);

    // VAO + VBO
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(s_QuadVertices), s_QuadVertices, GL_DYNAMIC_DRAW);

    // position attribute (location 0)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // texcoord attribute (location 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // 1x1 white texture fallback
    glGenTextures(1, &m_WhiteTex);
    glBindTexture(GL_TEXTURE_2D, m_WhiteTex);
    unsigned int white = 0xFFFFFFFF;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &white);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer2D::Shutdown()
{
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteTextures(1, &m_WhiteTex);
    delete m_Shader;
}

void Renderer2D::SetViewProjection(const glm::mat4& vp)
{
    m_VP = vp;
}

void Renderer2D::DrawQuad(const glm::vec2& position,
    const glm::vec2& size,
    float            rotation,
    const glm::vec4& color)
{
    DrawSprite(position, size, rotation, m_WhiteTex, color);
}

void Renderer2D::DrawSprite(const glm::vec2& position,
    const glm::vec2& size,
    float            rotation,
    unsigned int     textureID,
    const glm::vec4& tint)
{
    // Build transform matrix
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, glm::vec3(position, 0.0f));
    transform = glm::rotate(transform, glm::radians(rotation), glm::vec3(0, 0, 1));
    transform = glm::scale(transform, glm::vec3(size, 1.0f));

    m_Shader->Bind();
    m_Shader->SetMat4("u_ViewProjection", m_VP);
    m_Shader->SetMat4("u_Transform", transform);
    m_Shader->SetInt("u_Texture", 0);
    m_Shader->SetFloat4("u_Tint", tint);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    m_Shader->Unbind();
}

void Renderer2D::DrawFrame(const glm::vec2& position,
    const glm::vec2& size,
    float            rotation,
    unsigned int     textureID,
    const glm::vec2& uvMin,
    const glm::vec2& uvMax,
    const glm::vec4& tint)
{
    // Build vertices with custom UVs
    float vertices[] = {
        -0.5f, -0.5f,  uvMin.x, uvMin.y,
         0.5f, -0.5f,  uvMax.x, uvMin.y,
         0.5f,  0.5f,  uvMax.x, uvMax.y,

        -0.5f, -0.5f,  uvMin.x, uvMin.y,
         0.5f,  0.5f,  uvMax.x, uvMax.y,
        -0.5f,  0.5f,  uvMin.x, uvMax.y,
    };

    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, glm::vec3(position, 0.0f));
    transform = glm::rotate(transform, glm::radians(rotation), glm::vec3(0, 0, 1));
    transform = glm::scale(transform, glm::vec3(size, 1.0f));

    m_Shader->Bind();
    m_Shader->SetMat4("u_ViewProjection", m_VP);
    m_Shader->SetMat4("u_Transform", transform);
    m_Shader->SetInt("u_Texture", 0);
    m_Shader->SetFloat4("u_Tint", tint);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    m_Shader->Unbind();
}