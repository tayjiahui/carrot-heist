#include "PauseMenu.h"
#include <imgui.h>
#include <GuideBook.h>

void PauseMenu::RenderImGui(float vpX, float vpY, float vpW, float vpH)
{
    if (!m_Active) return;

    // Dark overlay
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(
        ImVec2(vpX, vpY),
        ImVec2(vpX + vpW, vpY + vpH),
        IM_COL32(0, 0, 0, 160)
    );

    // Menu box
    float boxW = 320.0f;
    float boxH = 300.0f;
    float boxX = vpX + vpW * 0.5f - boxW * 0.5f;
    float boxY = vpY + vpH * 0.5f - boxH * 0.5f;

    dl->AddRectFilled(
        ImVec2(boxX, boxY),
        ImVec2(boxX + boxW, boxY + boxH),
        IM_COL32(20, 20, 20, 240),
        12.0f
    );
    dl->AddRect(
        ImVec2(boxX, boxY),
        ImVec2(boxX + boxW, boxY + boxH),
        IM_COL32(180, 140, 80, 255),
        12.0f, 0, 2.0f
    );

    // Title
    ImGui::SetCursorScreenPos(ImVec2(boxX + boxW * 0.5f - 40, boxY + 20));
    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f), "PAUSED");

    // Separator
    dl->AddLine(
        ImVec2(boxX + 20, boxY + 55),
        ImVec2(boxX + boxW - 20, boxY + 55),
        IM_COL32(180, 140, 80, 150), 1.0f
    );

    // Buttons
    float btnW = boxW - 80.0f;
    float btnH = 45.0f;
    float btnX = boxX + 40.0f;
    float startY = boxY + 75.0f;
    float gap = 60.0f;

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);

    // Resume
    ImGui::SetCursorScreenPos(ImVec2(btnX, startY));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.55f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.75f, 0.20f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.10f, 0.40f, 0.10f, 1.0f));
    if (ImGui::Button("  Resume", ImVec2(btnW, btnH)))
    {
        Close();
        if (m_OnResume) m_OnResume();
    }
    ImGui::PopStyleColor(3);

    // Guide
    ImGui::SetCursorScreenPos(ImVec2(btnX, startY + gap));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.35f, 0.55f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.45f, 0.75f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.25f, 0.40f, 1.0f));
    if (ImGui::Button("  Guide", ImVec2(btnW, btnH)))
    {
        GuideBook::Get().Open();
    }
    ImGui::PopStyleColor(3);

    // Main Menu
    ImGui::SetCursorScreenPos(ImVec2(btnX, startY + gap * 2));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.45f, 0.10f, 0.10f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.65f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.30f, 0.05f, 0.05f, 1.0f));
    if (ImGui::Button("  Main Menu", ImVec2(btnW, btnH)))
    {
        Close();
        if (m_OnMainMenu) m_OnMainMenu();
    }
    ImGui::PopStyleColor(3);

    ImGui::PopStyleVar();

    // ESC hint
    ImGui::SetCursorScreenPos(ImVec2(boxX + boxW * 0.5f - 70, boxY + boxH - 30));
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 0.8f), "[ ESC ] to resume");

    // Guide renders on top of pause menu
    GuideBook::Get().RenderImGui(vpX, vpY, vpW, vpH);
}