#pragma once
#include "Renderer/Texture.h"
#include <glm/glm.hpp>
#include <string>

// Stores UV coordinates for a single frame
struct SpriteFrame
{
    glm::vec2 uvMin; // bottom-left UV
    glm::vec2 uvMax; // top-right UV
};

class SpriteSheet
{
public:
    // filepath    — path to PNG
    // frameWidth  — width of one frame in pixels
    // frameHeight — height of one frame in pixels
    SpriteSheet(const std::string& filepath, int frameWidth, int frameHeight);

    // Get UV coords for a specific frame (row, col)
    SpriteFrame GetFrame(int col, int row = 0) const;

    // Get frame by flat index (left to right, top to bottom)
    SpriteFrame GetFrameByIndex(int index) const;

    int GetFrameWidth()  const { return m_FrameWidth; }
    int GetFrameHeight() const { return m_FrameHeight; }
    int GetCols()        const { return m_Cols; }
    int GetRows()        const { return m_Rows; }

    const Texture& GetTexture() const { return *m_Texture; }

private:
    Texture* m_Texture = nullptr;
    int      m_FrameWidth = 0;
    int      m_FrameHeight = 0;
    int      m_Cols = 0;
    int      m_Rows = 0;
};