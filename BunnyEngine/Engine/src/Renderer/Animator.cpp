#include "Renderer/Animator.h"
#include <iostream>

void Animator::AddClip(const AnimationClip& clip)
{
    m_Clips[clip.name] = clip;
}

void Animator::Play(const std::string& clipName, bool forceRestart)
{
    if (m_CurrentClipName == clipName && !forceRestart) return;
    if (!m_Clips.count(clipName)) {
        std::cerr << "[Animator] Clip not found: " << clipName << "\n";
        return;
    }

    m_CurrentClipName = clipName;
    m_CurrentFrameIdx = 0;
    m_Timer = 0.0f;
    m_Finished = false;
}

void Animator::Update(float deltaTime)
{
    if (m_CurrentClipName.empty()) return;
    if (!m_Clips.count(m_CurrentClipName)) return;

    AnimationClip& clip = m_Clips[m_CurrentClipName];
    if (clip.frames.empty()) return;
    if (m_Finished && !clip.loop) return;

    m_Timer += deltaTime;

    while (m_Timer >= clip.frameDuration)
    {
        m_Timer -= clip.frameDuration;
        m_CurrentFrameIdx++;

        if (m_CurrentFrameIdx >= (int)clip.frames.size())
        {
            if (clip.loop)
                m_CurrentFrameIdx = 0;
            else
            {
                m_CurrentFrameIdx = (int)clip.frames.size() - 1;
                m_Finished = true;
                break;
            }
        }
    }
}

AnimationFrame Animator::GetCurrentFrame() const
{
    AnimationFrame result;
    result.uvMin = { 0.0f, 0.0f };
    result.uvMax = { 1.0f, 1.0f };

    if (!m_Sheet || m_CurrentClipName.empty()) return result;
    if (!m_Clips.count(m_CurrentClipName))     return result;

    const AnimationClip& clip = m_Clips.at(m_CurrentClipName);
    if (clip.frames.empty()) return result;

    int frameIndex = clip.frames[m_CurrentFrameIdx];
    SpriteFrame sf = m_Sheet->GetFrameByIndex(frameIndex);
    result.uvMin = sf.uvMin;
    result.uvMax = sf.uvMax;
    return result;
}