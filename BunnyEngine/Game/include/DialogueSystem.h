#pragma once
#include <string>
#include <vector>
#include <functional>

struct DialogueLine
{
    std::string speaker;
    std::string text;
};

class DialogueSystem
{
public:
    static DialogueSystem& Get() {
        static DialogueSystem instance;
        return instance;
    }

    // Start a dialogue sequence
    void StartDialogue(const std::vector<DialogueLine>& lines,
        std::function<void()>             onComplete = nullptr);

    // Advance to next line or close
    void Advance();

    // Update — call every frame
    void Update(float deltaTime);

    // Render dialogue box
    void RenderImGui(float vpX, float vpY, float vpW, float vpH);

    bool IsActive()  const { return m_Active; }
    bool IsFinished() const { return m_Finished; }
    void Reset() { m_Active = false; m_Finished = false; m_CurrentLine = 0; }

private:
    DialogueSystem() = default;

    std::vector<DialogueLine>  m_Lines;
    int                        m_CurrentLine = 0;
    bool                       m_Active = false;
    bool                       m_Finished = false;
    float                      m_TextTimer = 0.0f;
    float                      m_CharDelay = 0.03f; // typewriter speed
    int                        m_CharsShown = 0;     // typewriter effect
    std::function<void()>      m_OnComplete;
};