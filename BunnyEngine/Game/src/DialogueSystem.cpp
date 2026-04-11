#include "DialogueSystem.h"
#include <imgui.h>
#include <iostream>

void DialogueSystem::StartDialogue(
    const std::vector<DialogueLine>& lines,
    std::function<void()>             onComplete)
{
    m_Lines = lines;
    m_CurrentLine = 0;
    m_Active = true;
    m_Finished = false;
    m_CharsShown = 0;
    m_TextTimer = 0.0f;
    m_OnComplete = onComplete;
    std::cout << "[Dialogue] Started with " << lines.size() << " lines\n";
}

void DialogueSystem::Advance()
{
    if (!m_Active) return;

    const std::string& fullText = m_Lines[m_CurrentLine].text;

    // If typewriter not done yet, show full text instantly
    if (m_CharsShown < (int)fullText.size())
    {
        m_CharsShown = (int)fullText.size();
        return;
    }

    // Advance to next line
    m_CurrentLine++;
    m_CharsShown = 0;
    m_TextTimer = 0.0f;

    if (m_CurrentLine >= (int)m_Lines.size())
    {
        // Dialogue finished
        m_Active = false;
        m_Finished = true;
        std::cout << "[Dialogue] Finished\n";
        if (m_OnComplete) m_OnComplete();
    }
}

void DialogueSystem::Update(float deltaTime)
{
    if (!m_Active) return;

    const std::string& fullText = m_Lines[m_CurrentLine].text;

    // Typewriter effect
    if (m_CharsShown < (int)fullText.size())
    {
        m_TextTimer += deltaTime;
        while (m_TextTimer >= m_CharDelay &&
            m_CharsShown < (int)fullText.size())
        {
            m_CharsShown++;
            m_TextTimer -= m_CharDelay;
        }
    }
}

void DialogueSystem::RenderImGui(float vpX, float vpY, float vpW, float vpH)
{
    if (!m_Active) return;

    // Dialogue box at bottom of viewport
    float boxW = vpW * 0.7f;
    float boxH = 120.0f;
    float boxX = vpX + vpW * 0.5f - boxW * 0.5f;
    float boxY = vpY + vpH - boxH - 20.0f;

    // Background box
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(
        ImVec2(boxX, boxY),
        ImVec2(boxX + boxW, boxY + boxH),
        IM_COL32(20, 20, 20, 220),
        8.0f
    );
    dl->AddRect(
        ImVec2(boxX, boxY),
        ImVec2(boxX + boxW, boxY + boxH),
        IM_COL32(180, 140, 80, 255),
        8.0f, 0, 2.0f
    );

    // Speaker name
    const std::string& speaker = m_Lines[m_CurrentLine].speaker;
    if (!speaker.empty())
    {
        ImGui::SetCursorScreenPos(ImVec2(boxX + 16, boxY + 10));
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f),
            "%s", speaker.c_str());
    }

    // Dialogue text with typewriter effect
    const std::string& fullText = m_Lines[m_CurrentLine].text;
    std::string shown = fullText.substr(0, m_CharsShown);

    ImGui::SetCursorScreenPos(ImVec2(boxX + 16, boxY + 35));
    ImGui::PushTextWrapPos(boxX + boxW - 16);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", shown.c_str());
    ImGui::PopTextWrapPos();

    // Progress indicator
    bool textDone = m_CharsShown >= (int)fullText.size();
    std::string prompt = textDone
        ? (m_CurrentLine < (int)m_Lines.size() - 1 ? "[ E - Next ]" : "[ E - Close ]")
        : "[ E - Skip ]";

    ImGui::SetCursorScreenPos(ImVec2(boxX + boxW - 100, boxY + boxH - 25));
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 0.8f), "%s", prompt.c_str());

    // Line counter
    ImGui::SetCursorScreenPos(ImVec2(boxX + 16, boxY + boxH - 25));
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 0.8f),
        "%d / %d", m_CurrentLine + 1, (int)m_Lines.size());
}