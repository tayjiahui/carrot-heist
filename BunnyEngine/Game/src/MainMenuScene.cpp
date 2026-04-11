#include "MainMenuScene.h"
#include "Core/SceneManager.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "Core/AudioManager.h"

MainMenuScene::MainMenuScene(Renderer2D* renderer, Framebuffer* framebuffer)
    : m_Renderer(renderer), m_Framebuffer(framebuffer) {
}

MainMenuScene::~MainMenuScene()
{
    delete m_Background;
    delete m_BtnPlay;
    delete m_BtnGuide;
    delete m_BtnOptions;
    delete m_BtnQuit;
}

void MainMenuScene::OnEnter()
{
    std::cout << "[MainMenuScene] Entering...\n";

    m_Background = new Texture("Assets/Textures/Main_Menu.png");
    m_BtnPlay = new Texture("Assets/Textures/Play_button.png");
    m_BtnGuide = new Texture("Assets/Textures/Guide_button.png");
    m_BtnOptions = new Texture("Assets/Textures/Options_button.png");
    m_BtnQuit = new Texture("Assets/Textures/Quit_button.png");

    AudioManager::Get().LoadSound("menu_bgm", "Assets/Audio/MainMenu.mp3", true);
    AudioManager::Get().PlayBGM("menu_bgm");

}

void MainMenuScene::OnUpdate(float deltaTime)
{
    // nothing for now
}

void MainMenuScene::OnRenderGL()
{
    // Update screen size from ImGui
    ImGuiIO& io = ImGui::GetIO();
    m_ScreenW = io.DisplaySize.x;
    m_ScreenH = io.DisplaySize.y;

    // Use framebuffer size instead for correct projection
    // This matches what's actually being rendered into
    glm::mat4 proj = glm::ortho(
        0.0f, m_ScreenW,
        0.0f, m_ScreenH,
        -1.0f, 1.0f
    );
    m_Renderer->SetViewProjection(proj);

    m_Renderer->DrawSprite(
        { m_ScreenW * 0.5f, m_ScreenH * 0.5f },
        { m_ScreenW, m_ScreenH },
        0.0f,
        m_Background->GetID()
    );
}

void MainMenuScene::OnRenderImGui()
{
    // We are already inside the Viewport window when this is called
    // so just use GetContentRegionAvail for sizing
    ImVec2 vpSize = ImGui::GetContentRegionAvail();
    ImVec2 vpPos = ImGui::GetCursorScreenPos();

    m_ScreenW = vpSize.x;
    m_ScreenH = vpSize.y;

    float scale = 0.55f;
    float centerX = vpSize.x * 0.25f;  // change from 0.5f to 0.25f
    float startY = vpPos.y + vpSize.y * 0.52f;
    float gap = 70.0f;

    // Draw buttons directly onto the viewport's draw list
    auto ImageBtn = [&](Texture* tex, float origW, float origH, float posY) -> bool
        {
            if (!tex) return false;
            float w = origW * scale;
            float h = origH * scale;
            float x = vpPos.x + centerX - w * 0.5f;

            ImGui::SetCursorScreenPos(ImVec2(x, posY));

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.08f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.15f));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

            std::string id = "##btn" + std::to_string((uintptr_t)tex->GetID());
            bool clicked = ImGui::ImageButton(
                id.c_str(),
                (ImTextureID)(uintptr_t)tex->GetID(),
                ImVec2(w, h),
                ImVec2(0, 1),
                ImVec2(1, 0)
            );

            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);
            return clicked;
        };

    if (ImageBtn(m_BtnPlay, 760.0f, 98.0f, startY))
        SceneManager::Get().SwitchTo("Game");

    if (ImageBtn(m_BtnGuide, 700.0f, 98.0f, startY + gap)) {}

    if (ImageBtn(m_BtnOptions, 611.0f, 98.0f, startY + gap * 2)) {}

    if (ImageBtn(m_BtnQuit, 520.0f, 98.0f, startY + gap * 3))
        exit(0);
}

void MainMenuScene::OnExit()
{
    std::cout << "[MainMenuScene] Exiting...\n";

    delete m_Background;   m_Background = nullptr;
    delete m_BtnPlay;      m_BtnPlay = nullptr;
    delete m_BtnGuide;     m_BtnGuide = nullptr;
    delete m_BtnOptions;   m_BtnOptions = nullptr;
    delete m_BtnQuit;      m_BtnQuit = nullptr;

    AudioManager::Get().StopBGM();
}