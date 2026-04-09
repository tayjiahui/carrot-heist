#include "Renderer/Texture.h"
#include <iostream>

// stb_image — define implementation only once
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Texture::Texture(const std::string& filepath)
    : m_Filepath(filepath)
{
    // Flip vertically — OpenGL expects bottom-left origin
    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(filepath.c_str(),
        &m_Width, &m_Height, &m_Channels, 0);

    if (!data) {
        std::cerr << "[Texture] Failed to load: " << filepath << "\n";
        std::cerr << "[Texture] Reason: " << stbi_failure_reason() << "\n";
        return;
    }

    // Pick format based on channels
    GLenum internalFormat = (m_Channels == 4) ? GL_RGBA8 : GL_RGB8;
    GLenum dataFormat = (m_Channels == 4) ? GL_RGBA : GL_RGB;

    glGenTextures(1, &m_ID);
    glBindTexture(GL_TEXTURE_2D, m_ID);

    // Texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // pixel art = nearest
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat,
        m_Width, m_Height, 0, dataFormat, GL_UNSIGNED_BYTE, data);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);

    std::cout << "[Texture] Loaded: " << filepath
        << " (" << m_Width << "x" << m_Height
        << ", " << m_Channels << " channels)\n";
}

Texture::~Texture()
{
    glDeleteTextures(1, &m_ID);
}

void Texture::Bind(unsigned int slot) const
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_ID);
}

void Texture::Unbind() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}