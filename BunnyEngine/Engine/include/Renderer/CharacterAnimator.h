#pragma once
#include "Renderer/Animator.h"
#include "Renderer/SpriteSheet.h"
#include <glm/glm.hpp>
#include <string>

enum class FacingDir { Right, Left, Up, Down };

class CharacterAnimator
{
public:
    // Init with spritesheet
    void Init(const std::string& sheetPath,
        int                frameW,
        int                frameH);
    void Shutdown();

    // Add clips externally — each character defines their own
    void AddClip(const AnimationClip& clip) { m_Animator.AddClip(clip); }

    // Playback
    void Play(const std::string& clipName, bool forceRestart = false)
    {
        m_Animator.Play(clipName, forceRestart);
    }

    // Update facing + animator
    void UpdateFacing(glm::vec2 moveDir);
    void Update(float deltaTime) { m_Animator.Update(deltaTime); }

    // Get current frame for rendering
    AnimationFrame GetCurrentFrame() const { return m_Animator.GetCurrentFrame(); }
    unsigned int   GetTextureID()    const;
    FacingDir      GetFacing()       const { return m_Facing; }
    bool           IsFinished()      const { return m_Animator.IsFinished(); }

    // Frame size in world units (set after Init)
    glm::vec2 GetWorldSize() const { return m_WorldSize; }
    void      SetWorldSize(glm::vec2 size) { m_WorldSize = size; }

    bool GetFlipX() const { return m_FlipX; }
private:
    Animator     m_Animator;
    SpriteSheet* m_Sheet = nullptr;
    FacingDir    m_Facing = FacingDir::Down;
    glm::vec2    m_WorldSize = { 2.0f, 2.0f };
    bool m_FlipX = false;
};