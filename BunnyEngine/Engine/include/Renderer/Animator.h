#pragma once
#include "Renderer/SpriteSheet.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>

struct AnimationClip
{
    std::string       name;
    std::vector<int>  frames;       // frame indices into spritesheet
    float             frameDuration; // seconds per frame
    bool              loop = true;
};

struct AnimationFrame
{
    glm::vec2 uvMin;
    glm::vec2 uvMax;
};

class Animator
{
public:
    // Setup
    void SetSpriteSheet(SpriteSheet* sheet) { m_Sheet = sheet; }
    void AddClip(const AnimationClip& clip);

    // Playback
    void Play(const std::string& clipName, bool forceRestart = false);
    void Update(float deltaTime);

    // Get current frame UVs for rendering
    AnimationFrame GetCurrentFrame() const;

    // Getters
    const std::string& GetCurrentClipName() const { return m_CurrentClipName; }
    bool               IsFinished()          const { return m_Finished; }

private:
    SpriteSheet* m_Sheet = nullptr;

    std::unordered_map<std::string, AnimationClip> m_Clips;

    std::string m_CurrentClipName = "";
    int         m_CurrentFrameIdx = 0;  // index into clip's frames array
    float       m_Timer = 0.0f;
    bool        m_Finished = false;
};