#include "Renderer/CharacterAnimator.h"
#include <iostream>

void CharacterAnimator::Init(const std::string& sheetPath,
    int                frameW,
    int                frameH)
{
    m_Sheet = new SpriteSheet(sheetPath, frameW, frameH);
    m_Animator.SetSpriteSheet(m_Sheet);
    std::cout << "[CharacterAnimator] Loaded: " << sheetPath << "\n";
}

void CharacterAnimator::Shutdown()
{
    delete m_Sheet;
    m_Sheet = nullptr;
}

void CharacterAnimator::UpdateFacing(glm::vec2 moveDir)
{
    if (glm::length(moveDir) < 0.01f) return;

    if (moveDir.x > 0.5f) { m_Facing = FacingDir::Right; m_FlipX = true; }
    else if (moveDir.x < -0.5f) { m_Facing = FacingDir::Left;  m_FlipX = false; }
    else if (moveDir.y > 0.5f) { m_Facing = FacingDir::Up;    m_FlipX = false; }
    else if (moveDir.y < -0.5f) { m_Facing = FacingDir::Down;  m_FlipX = false; }
}

unsigned int CharacterAnimator::GetTextureID() const
{
    if (!m_Sheet) return 0;
    return m_Sheet->GetTexture().GetID();
}