#include "SpriteSheet.h"
#include <iostream>

SpriteSheet::SpriteSheet(const std::string& filepath, int frameWidth, int frameHeight)
    : m_FrameWidth(frameWidth), m_FrameHeight(frameHeight)
{
    m_Texture = new Texture(filepath);
    m_Cols = m_Texture->GetWidth() / frameWidth;
    m_Rows = m_Texture->GetHeight() / frameHeight;

    std::cout << "[SpriteSheet] " << m_Cols << " cols x "
        << m_Rows << " rows ("
        << (m_Cols * m_Rows) << " frames total)\n";
}

SpriteFrame SpriteSheet::GetFrame(int col, int row) const
{
    float texW = (float)m_Texture->GetWidth();
    float texH = (float)m_Texture->GetHeight();

    float uMin = (col * m_FrameWidth) / texW;
    float vMin = (row * m_FrameHeight) / texH;
    float uMax = uMin + m_FrameWidth / texW;
    float vMax = vMin + m_FrameHeight / texH;

    // Flip V for OpenGL (bottom-left origin)
    return {
        glm::vec2(uMin, 1.0f - vMax),
        glm::vec2(uMax, 1.0f - vMin)
    };
}

SpriteFrame SpriteSheet::GetFrameByIndex(int index) const
{
    int col = index % m_Cols;
    int row = index / m_Cols;
    return GetFrame(col, row);
}