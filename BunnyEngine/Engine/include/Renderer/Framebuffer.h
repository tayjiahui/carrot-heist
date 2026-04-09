#pragma once
#include <glad/glad.h>

class Framebuffer
{
public:
    Framebuffer(int width, int height);
    ~Framebuffer();

    void Bind();
    void Unbind();
    void Resize(int width, int height);

    unsigned int GetTextureID() const { return m_TextureID; }
    int GetWidth()  const { return m_Width; }
    int GetHeight() const { return m_Height; }

private:
    void Create();
    void Cleanup();

    unsigned int m_FBO = 0; // framebuffer object
    unsigned int m_TextureID = 0; // color attachment texture
    unsigned int m_RBO = 0; // renderbuffer for depth/stencil

    int m_Width = 0;
    int m_Height = 0;
};