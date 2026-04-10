#include "GameScene.h"
#include "Core/SceneManager.h"
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

GameScene::GameScene(Renderer2D* renderer, Framebuffer* framebuffer)
    : m_Renderer(renderer), m_Framebuffer(framebuffer) {
}

GameScene::~GameScene()
{
    delete m_Scene;
}

void GameScene::OnEnter()
{
    std::cout << "[GameScene] Entered!\n";

    m_Scene = new Scene("Level_01");
    m_TofuID = m_Scene->AddEntity("Tofu (Player)").id;
    m_ToastID = m_Scene->AddEntity("Toast (Player)").id;

    GetEntity(m_TofuID)->transform.position = { -1.5f, 0.0f };
    GetEntity(m_ToastID)->transform.position = { 1.5f, 0.0f };

    m_Scene->AddEntity("Guard_01").transform.position = { 3.0f, 2.0f };
    m_Scene->AddEntity("Guard_02").transform.position = { -3.0f, 2.0f };
    m_Scene->AddEntity("Carrot_01").transform.position = { 0.0f, 3.0f };

    // Start with Tofu (key 1)
    m_ActiveBunnyID = m_TofuID;
    m_ActiveBunnyType = BunnyType::Tofu;
}

void GameScene::OnUpdate(float deltaTime)
{

    // Camera zoom via scroll
    float scroll = Input::GetScrollDelta();
    if (scroll != 0.0f)
        m_Camera.OnMouseZoom(scroll);

    // Camera pan via middle mouse
    if (Input::IsMouseButtonDown(GLFW_MOUSE_BUTTON_MIDDLE))
    {
        glm::vec2 delta = Input::GetMouseDelta();
        m_Camera.OnMousePan(delta.x, delta.y);
    }

    // Reset camera
    if (Input::IsKeyPressed(GLFW_KEY_R))
        m_Camera.Reset();

    // Update camera with framebuffer size
    m_Camera.Update(
        (float)m_Framebuffer->GetWidth(),
        (float)m_Framebuffer->GetHeight()
    );

    // Update ability cooldowns
    AbilitySystem::Update(deltaTime);

    // --- Switch bunnies ---
    if (Input::IsKeyPressed(GLFW_KEY_1)) {
        m_ActiveBunnyID = m_TofuID;
        m_ActiveBunnyType = BunnyType::Tofu;
        std::cout << "[GameScene] Switched to Tofu\n";
    }
    if (Input::IsKeyPressed(GLFW_KEY_2)) {
        m_ActiveBunnyID = m_ToastID;
        m_ActiveBunnyType = BunnyType::Toast;
        std::cout << "[GameScene] Switched to Toast\n";
    }

    // --- WASD Movement ---
    Entity* bunny = GetEntity(m_ActiveBunnyID);
    if (!bunny) return;

    glm::vec2 dir = { 0.0f, 0.0f };
    if (Input::IsKeyDown(GLFW_KEY_W)) dir.y += 1.0f;
    if (Input::IsKeyDown(GLFW_KEY_S)) dir.y -= 1.0f;
    if (Input::IsKeyDown(GLFW_KEY_A)) dir.x -= 1.0f;
    if (Input::IsKeyDown(GLFW_KEY_D)) dir.x += 1.0f;

    if (glm::length(dir) > 0.0f)
        dir = glm::normalize(dir);

    bunny->transform.position += dir * m_Speed * deltaTime;

    // --- Abilities ---
    if (Input::IsKeyPressed(GLFW_KEY_J))
        AbilitySystem::TriggerAbility(m_ActiveBunnyType, AbilitySlot::J, bunny);
    if (Input::IsKeyPressed(GLFW_KEY_K))
        AbilitySystem::TriggerAbility(m_ActiveBunnyType, AbilitySlot::K, bunny);
    if (Input::IsKeyPressed(GLFW_KEY_L))
        AbilitySystem::TriggerAbility(m_ActiveBunnyType, AbilitySlot::L, bunny);
}

void GameScene::OnRenderGL()
{
    m_Renderer->SetViewProjection(m_Camera.GetViewProjection());

    if (!m_Scene) return;

    for (auto& entity : m_Scene->GetEntities())
    {
        if (!entity.active) continue;

        glm::vec4 color = { 0.8f, 0.8f, 0.8f, 1.0f };
        if (entity.name.find("Tofu") != std::string::npos) color = { 0.95f, 0.95f, 0.90f, 1.0f };
        if (entity.name.find("Toast") != std::string::npos) color = { 0.95f, 0.85f, 0.60f, 1.0f };
        if (entity.name.find("Guard") != std::string::npos) color = { 0.80f, 0.20f, 0.20f, 1.0f };
        if (entity.name.find("Carrot") != std::string::npos) color = { 1.00f, 0.55f, 0.10f, 1.0f };

        if (entity.id == m_ActiveBunnyID)
            color = glm::vec4(glm::vec3(color) * 1.5f, 1.0f);

        m_Renderer->DrawQuad(
            entity.transform.position,
            entity.transform.scale,
            entity.transform.rotation,
            color
        );
    }
}

void GameScene::OnRenderImGui()
{
    ImVec2 vpSize = ImGui::GetContentRegionAvail();
    ImVec2 vpPos = ImGui::GetCursorScreenPos();

    Entity* bunny = GetEntity(m_ActiveBunnyID);
    if (!bunny) return;

    // Active bunny name
    ImGui::SetCursorScreenPos(ImVec2(vpPos.x + 10, vpPos.y + 10));
    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f),
        "Active: %s", bunny->name.c_str());

    // Controls hint
    ImGui::SetCursorScreenPos(ImVec2(vpPos.x + 10, vpPos.y + 30));
    ImGui::TextColored(ImVec4(1, 1, 1, 0.5f),
        "WASD: Move | 1: Tofu | 2: Toast | J/K/L: Abilities");

    // Ability cooldown bars
    float barW = 140.0f;
    float barH = 16.0f;
    float barX = vpPos.x + 10;
    float barY = vpPos.y + vpSize.y - 90.0f;

    AbilitySlot slots[3] = { AbilitySlot::J, AbilitySlot::K, AbilitySlot::L };
    const char* keys[3] = { "J", "K", "L" };

    for (int i = 0; i < 3; i++)
    {
        AbilityCooldown cd = AbilitySystem::GetCooldown(m_ActiveBunnyType, slots[i]);
        std::string     name = AbilitySystem::GetAbilityName(m_ActiveBunnyType, slots[i]);
        float           pct = (cd.max > 0.0f) ? (1.0f - cd.current / cd.max) : 1.0f;

        ImGui::SetCursorScreenPos(ImVec2(barX, barY + i * 25.0f));
        ImGui::TextColored(ImVec4(1, 1, 1, 0.8f), "[%s] %s", keys[i], name.c_str());

        ImGui::SetCursorScreenPos(ImVec2(barX + 100, barY + i * 25.0f));
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram,
            cd.IsReady() ? ImVec4(0.2f, 0.8f, 0.2f, 1.0f)
            : ImVec4(0.6f, 0.3f, 0.1f, 1.0f));
        ImGui::ProgressBar(pct, ImVec2(barW, barH), cd.IsReady() ? "Ready" : "");
        ImGui::PopStyleColor();
    }

    // Back to menu
    ImGui::SetCursorScreenPos(ImVec2(vpPos.x + 10, vpPos.y + vpSize.y - 30.0f));
    if (ImGui::Button("< Main Menu"))
        SceneManager::Get().SwitchTo("MainMenu");
}

void GameScene::OnExit()
{
    std::cout << "[GameScene] Exited!\n";
    delete m_Scene;
    m_Scene = nullptr;
}

std::vector<Entity>* GameScene::GetEntities()
{
    return m_Scene ? &m_Scene->GetEntities() : nullptr;
}

const std::string& GameScene::GetSceneName()
{
    return m_Scene ? m_Scene->GetName()
        : SceneManager::Get().GetCurrentSceneName();
}

Entity* GameScene::GetEntity(int id)
{
    if (!m_Scene) return nullptr;
    for (auto& e : m_Scene->GetEntities())
        if (e.id == id) return &e;
    return nullptr;
}

void GameScene::CycleBunny(int dir)
{
    if (m_ActiveBunnyID == m_TofuID) {
        m_ActiveBunnyID = m_ToastID;
        m_ActiveBunnyType = BunnyType::Toast;
    }
    else {
        m_ActiveBunnyID = m_TofuID;
        m_ActiveBunnyType = BunnyType::Tofu;
    }
}