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
    float bookW = 768.0f;
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

    float btnW = 75.0f * scale;
    float btnH = 43.0f * scale;
    float margin = 16.0f * scale;

    // --- Prev button ---
    // Bottom LEFT of left page
    if (m_CurrentPage > 0 && m_PrevTexture)
    {
        float prevX = bookX + margin;
        float prevY = bookY + scaledH - btnH - margin;

        // Push zero padding so image fills the button exactly
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.15f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.25f));

        ImGui::SetCursorScreenPos(ImVec2(prevX, prevY));
        if (ImGui::ImageButton(
            "##prevpage",
            (ImTextureID)(uintptr_t)m_PrevTexture->GetID(),
            ImVec2(btnW, btnH),
            ImVec2(0, 1), ImVec2(1, 0)))
        {
            PrevPage();
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);
    }

    // --- Next button ---
    // Bottom RIGHT of right page
    if (m_CurrentPage < (int)m_PageTextures.size() - 1 && m_NextTexture)
    {
        float nextX = bookX + scaledW - btnW - margin;
        float nextY = bookY + scaledH - btnH - margin;

        ImGui::SetCursorScreenPos(ImVec2(nextX, nextY));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.15f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.25f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

        if (ImGui::ImageButton(
            "##nextpage",
            (ImTextureID)(uintptr_t)m_NextTexture->GetID(),
            ImVec2(btnW, btnH),
            ImVec2(0, 1), ImVec2(1, 0)))
        {
            NextPage();
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);
    }

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