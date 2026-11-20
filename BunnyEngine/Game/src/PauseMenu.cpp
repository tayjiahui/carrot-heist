#include "PauseMenu.h"
#include <imgui.h>
#include <GuideBook.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include "Renderer/Texture.h"

void PauseMenu::LoadTextures()
{
    if (m_TexturesLoaded) return;
    m_Background  = new Texture("Assets/Textures/PauseBG.png");
    m_BtnResume   = new Texture("Assets/Textures/ResumePause_button.png");
    m_BtnGuide    = new Texture("Assets/Textures/Guidepause_button.png");
    m_BtnMainMenu = new Texture("Assets/Textures/MainmenuPause_button.png");
    m_TexturesLoaded = true;
}

void PauseMenu::RenderImGui(float vpX, float vpY, float vpW, float vpH)
{
    if (!m_Active) return;

    LoadTextures();

    // Dark overlay
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(
        ImVec2(vpX, vpY),
        ImVec2(vpX + vpW, vpY + vpH),
        IM_COL32(0, 0, 0, 160)
    );

    // Paper note background (PauseBG.png is 228 x 259 px, already says PAUSED).
    // Whole-pixel scale so the pixel art stays crisp; fits the viewport height.
    float scale = std::floor(std::min(vpH * 0.75f / 259.0f, vpW * 0.6f / 228.0f));
    scale = std::max(scale, 1.0f);

    float boxW = 228.0f * scale;
    float boxH = 259.0f * scale;
    float boxX = vpX + vpW * 0.5f - boxW * 0.5f;
    float boxY = vpY + vpH * 0.5f - boxH * 0.5f;

    // Textures are loaded flipped (stb), so V runs 1 -> 0
    dl->AddImage(
        (ImTextureID)(uintptr_t)m_Background->GetID(),
        ImVec2(boxX, boxY),
        ImVec2(boxX + boxW, boxY + boxH),
        ImVec2(0, 1), ImVec2(1, 0)
    );

    // Buttons (each 120 x 27 px), stacked on the paper under the PAUSED title
    float btnW = 120.0f * scale;
    float btnH = 27.0f * scale;
    float btnX = boxX + boxW * 0.5f - btnW * 0.5f;
    float startY = boxY + 82.0f * scale;
    float gap = 40.0f * scale;

    auto ImageBtn = [&](Texture* tex, float posY) -> bool
        {
            ImGui::SetCursorScreenPos(ImVec2(btnX, posY));

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.12f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.20f));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

            std::string id = "##pause" + std::to_string((uintptr_t)tex->GetID());
            bool clicked = ImGui::ImageButton(
                id.c_str(),
                (ImTextureID)(uintptr_t)tex->GetID(),
                ImVec2(btnW, btnH),
                ImVec2(0, 1),
                ImVec2(1, 0)
            );

            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);
            return clicked;
        };

    if (ImageBtn(m_BtnResume, startY))
    {
        Close();
        if (m_OnResume) m_OnResume();
    }

    if (ImageBtn(m_BtnGuide, startY + gap))
    {
        GuideBook::Get().Open();
    }

    if (ImageBtn(m_BtnMainMenu, startY + gap * 2))
    {
        Close();
        if (m_OnMainMenu) m_OnMainMenu();
    }

    // ESC hint under the note
    ImGui::SetCursorScreenPos(ImVec2(boxX + boxW * 0.5f - 70, boxY + boxH + 10));
    ImGui::TextColored(ImVec4(0.85f, 0.85f, 0.85f, 0.9f), "[ ESC ] to resume");

    // Guide renders on top of pause menu
    GuideBook::Get().RenderImGui(vpX, vpY, vpW, vpH);
}
