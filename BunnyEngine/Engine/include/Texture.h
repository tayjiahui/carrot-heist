#pragma once
#include <glad/glad.h>
#include <string>

class Texture
{
public:
    Texture(const std::string& filepath);
    ~Texture();

    void Bind(unsigned int slot = 0) const;
    void Unbind() const;

    unsigned int GetID()     const { return m_ID; }
    int          GetWidth()  const { return m_Width; }
    int          GetHeight() const { return m_Height; }
    int          GetChannels() const { return m_Channels; }

private:
    unsigned int m_ID = 0;
    int          m_Width = 0;
    int          m_Height = 0;
    int          m_Channels = 0;
    std::string  m_Filepath;
};