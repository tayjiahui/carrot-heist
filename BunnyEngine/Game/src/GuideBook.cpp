#include "GuideBook.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <iostream>
#include <algorithm>

void GuideBook::Init(
    const std::string& closeTexture,
    const std::string& nextTexture,
    const std::string& prevTexture,
    const std::vector<std::string>& pageTextures)
{
    m_CloseTexture = new Texture(closeTexture);
    m_NextTexture = new Texture(nextTexture);
    m_PrevTexture = new Texture(prevTexture);

    for (const auto& path : pageTextures)
        m_PageTextures.push_back(new Texture(path));

    std::cout << "[GuideBook] Loaded " << m_PageTextures.size() << " pages\n";
}

void GuideBook::Shutdown()
{
    delete m_CloseTexture;
    delete m_NextTexture;
    delete m_PrevTexture;

    for (auto* t : m_PageTextures) delete t;
    m_PageTextures.clear();
}

void GuideBook::NextPage()
{
    if (m_CurrentPage < (int)m_PageTextures.size() - 1)
        m_CurrentPage++;
}

void GuideBook::PrevPage()
{
    if (m_CurrentPage > 0)
        m_CurrentPage--;
}

void GuideBook::RenderImGui(float vpX, float vpY, float vpW, float vpH)
{
    if (!m_Active) return;
    if (m_PageTextures.empty()) return;

    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Dark overlay
    dl->AddRectFilled(
        ImVec2(vpX, vpY),
        ImVec2(vpX + vpW, vpY + vpH),
        IM_COL32(0, 0, 0, 180)
    );

    // Scale book to fit viewport
    float bookW = 775.0f;   // Bookpg*.png are 775 x 480
    float bookH = 480.0f;
    float scaleX = (vpW * 0.9f) / bookW;
    float scaleY = (vpH * 0.9f) / bookH;
    float scale = std::min(scaleX, scaleY);
    scale = std::min(scale, 1.0f);

    float scaledW = bookW * scale;
    float scaledH = bookH * scale;
    float bookX = vpX + vpW * 0.5f - scaledW * 0.5f;
    float bookY = vpY + vpH * 0.5f - scaledH * 0.5f;

    // Draw current page texture
    dl->AddImage(
        (ImTextureID)(uintptr_t)m_PageTextures[m_CurrentPage]->GetID(),
        ImVec2(bookX, bookY),
        ImVec2(bookX + scaledW, bookY + scaledH),
        ImVec2(0, 1), ImVec2(1, 0)
    );

    // Page-turn buttons: the carrot arrows are drawn into the page art
    // (Bookpg*.png, 775 x 480), so these are invisible click areas placed
    // over them, in page pixels, with a soft highlight on hover.
    auto PageButton = [&](const char* id, float px0, float py0, float px1, float py1) -> bool
        {
            ImVec2 min(bookX + px0 * scale, bookY + py0 * scale);
            ImVec2 max(bookX + px1 * scale, bookY + py1 * scale);

            ImGui::SetCursorScreenPos(min);
            bool clicked = ImGui::InvisibleButton(id, ImVec2(max.x - min.x, max.y - min.y));
            if (ImGui::IsItemHovered())
                dl->AddRectFilled(min, max, IM_COL32(255, 255, 255, 45), 8.0f * scale);
            return clicked;
        };

    // --- Prev: carrot at the bottom left of the left page ---
    if (m_CurrentPage > 0 && PageButton("##prevpage", 78.0f, 376.0f, 140.0f, 422.0f))
        PrevPage();

    // --- Next: carrot at the bottom right of the right page ---
    if (m_CurrentPage < (int)m_PageTextures.size() - 1 &&
        PageButton("##nextpage", 648.0f, 376.0f, 710.0f, 422.0f))
        NextPage();

    // --- Close button --- top right of book
    if (m_CloseTexture)
    {
        float closeSz = 32.0f;
        float closeX = bookX + scaledW - closeSz - 5;
        float closeY = bookY + 5;

        ImGui::SetCursorScreenPos(ImVec2(closeX, closeY));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.15f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.25f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

        if (ImGui::ImageButton(
            "##closebook",
            (ImTextureID)(uintptr_t)m_CloseTexture->GetID(),
            ImVec2(closeSz, closeSz),
            ImVec2(0, 1), ImVec2(1, 0)))
        {
            Close();
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);
    }
}