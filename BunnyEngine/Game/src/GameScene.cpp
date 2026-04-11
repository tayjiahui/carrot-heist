#include "GameScene.h"
#include "Core/SceneManager.h"
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "Core/AudioManager.h"


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

    AudioManager::Get().LoadSound("game_bgm", "Assets/Audio/Level1.wav", true);
    AudioManager::Get().LoadSound("collect", "Assets/Audio/sfx_collect.mp3");
    AudioManager::Get().PlayBGM("game_bgm");

    m_CarrotsTotal = (int)m_Tilemap.GetObjectsByType("Carrot").size();
    std::cout << "[GameScene] Total carrots: " << m_CarrotsTotal << "\n";

    // Load tilemap
    if (m_Tilemap.Load("Assets/Tilemaps/level01.tmj"))
    {
        const TilemapData& data = m_Tilemap.GetData();

        // Scale so tiles are visible — 16px tiles * scale = world units
        // 1 world unit = 1.0f, so scale = 1.0f/16.0f to make tiles 1 unit
        m_TileScale = 1.0f / (float)data.tileWidth;

        // Spawn entities from object layer
        m_Scene = new Scene("Level_01");

        for (auto& obj : m_Tilemap.GetObjects())
        {
            const TilemapData& data = m_Tilemap.GetData();

            // Convert Tiled pixel coords to world units
            // Tiled origin is top-left, OpenGL origin is bottom-left
            float worldX = obj.x * m_TileScale;
            float worldY = ((data.mapHeight * data.tileHeight) - obj.y) * m_TileScale;

            std::cout << "[GameScene] Object: " << obj.name
                << " Type: " << obj.type
                << " Pos: (" << worldX << ", " << worldY << ")\n";

            if (obj.type == "Player")
            {
                auto& e = m_Scene->AddEntity(obj.name);
                e.transform.position = { worldX, worldY };
                e.transform.scale = { 1.0f, 1.0f };

                if (obj.name == "Toast_Spawn") m_ToastID = e.id;
                if (obj.name == "Tofu_Spawn")  m_TofuID = e.id;
            }
            else if (obj.type == "Guard")
            {
                auto& e = m_Scene->AddEntity(obj.name);
                e.transform.position = { worldX, worldY };
                e.transform.scale = { 1.0f, 1.0f };
            }
            else if (obj.type == "Carrot")
            {
                auto& e = m_Scene->AddEntity(obj.name);
                e.transform.position = { worldX, worldY };
                e.transform.scale = { 0.5f, 0.5f };
            }
        }

        // Fallback if spawn points not found
        if (m_TofuID == -1) m_TofuID = m_Scene->AddEntity("Tofu (Player)").id;
        if (m_ToastID == -1) m_ToastID = m_Scene->AddEntity("Toast (Player)").id;
    }
    else
    {
        // Fallback if tilemap fails to load
        m_Scene = new Scene("Level_01");
        m_TofuID = m_Scene->AddEntity("Tofu (Player)").id;
        m_ToastID = m_Scene->AddEntity("Toast (Player)").id;
        GetEntity(m_TofuID)->transform.position = { -1.5f, 0.0f };
        GetEntity(m_ToastID)->transform.position = { 1.5f, 0.0f };
    }

    // Start with Tofu (key 1)
    m_ActiveBunnyID = m_TofuID;
    m_ActiveBunnyType = BunnyType::Tofu;

    // Store bunny IDs for AI
    m_BunnyIDs = { m_TofuID, m_ToastID };

    // Initialize guard AI
    GuardAISystem::Init(
        m_Tilemap.GetObjects(),
        m_TileScale,
        m_Tilemap.GetData().mapHeight,
        m_Tilemap.GetData().tileHeight
    );
    GuardAISystem::ResetCaught();
    m_GameOver = false;
    m_GameWon = false;

}

void GameScene::OnUpdate(float deltaTime)
{
    // 1. Update ability cooldowns
    AbilitySystem::Update(deltaTime);

    // 2. Camera zoom + pan
    float scroll = Input::GetScrollDelta();
    if (scroll != 0.0f)
        m_Camera.OnMouseZoom(scroll);

    if (Input::IsMouseButtonDown(GLFW_MOUSE_BUTTON_MIDDLE)) {
        glm::vec2 delta = Input::GetMouseDelta();
        m_Camera.OnMousePan(delta.x, delta.y);
    }

    if (Input::IsKeyPressed(GLFW_KEY_R))
        m_Camera.Reset();

    m_Camera.Update(
        (float)m_Framebuffer->GetWidth(),
        (float)m_Framebuffer->GetHeight()
    );

    // 3. Switch bunnies
    if (Input::IsKeyPressed(GLFW_KEY_1)) {
        m_ActiveBunnyID = m_TofuID;
        m_ActiveBunnyType = BunnyType::Tofu;
    }
    if (Input::IsKeyPressed(GLFW_KEY_2)) {
        m_ActiveBunnyID = m_ToastID;
        m_ActiveBunnyType = BunnyType::Toast;
    }

    // 4. Get active bunny
    Entity* bunny = GetEntity(m_ActiveBunnyID);
    if (!bunny) return;

    // 5. WASD movement
    glm::vec2 dir = { 0.0f, 0.0f };
    if (Input::IsKeyDown(GLFW_KEY_W)) dir.y += 1.0f;
    if (Input::IsKeyDown(GLFW_KEY_S)) dir.y -= 1.0f;
    if (Input::IsKeyDown(GLFW_KEY_A)) dir.x -= 1.0f;
    if (Input::IsKeyDown(GLFW_KEY_D)) dir.x += 1.0f;

    if (glm::length(dir) > 0.0f)
        dir = glm::normalize(dir);

    glm::vec2 newPos = bunny->transform.position + dir * m_Speed * deltaTime;

    // 6. Collision check
    float halfSize = 0.4f;
    bool collX = false;
    bool collY = false;

    glm::vec2 testX = { newPos.x, bunny->transform.position.y };
    if (m_Tilemap.IsCollision(testX.x + halfSize, testX.y, m_TileScale) ||
        m_Tilemap.IsCollision(testX.x - halfSize, testX.y, m_TileScale) ||
        m_Tilemap.IsCollision(testX.x + halfSize, testX.y + halfSize, m_TileScale) ||
        m_Tilemap.IsCollision(testX.x - halfSize, testX.y + halfSize, m_TileScale) ||
        m_Tilemap.IsCollision(testX.x + halfSize, testX.y - halfSize, m_TileScale) ||
        m_Tilemap.IsCollision(testX.x - halfSize, testX.y - halfSize, m_TileScale))
        collX = true;

    glm::vec2 testY = { bunny->transform.position.x, newPos.y };
    if (m_Tilemap.IsCollision(testY.x + halfSize, testY.y, m_TileScale) ||
        m_Tilemap.IsCollision(testY.x - halfSize, testY.y, m_TileScale) ||
        m_Tilemap.IsCollision(testY.x + halfSize, testY.y + halfSize, m_TileScale) ||
        m_Tilemap.IsCollision(testY.x - halfSize, testY.y + halfSize, m_TileScale) ||
        m_Tilemap.IsCollision(testY.x + halfSize, testY.y - halfSize, m_TileScale) ||
        m_Tilemap.IsCollision(testY.x - halfSize, testY.y - halfSize, m_TileScale))
        collY = true;

    if (!collX) bunny->transform.position.x = newPos.x;
    if (!collY) bunny->transform.position.y = newPos.y;

    // 7. Carrot pickup
    for (auto& entity : m_Scene->GetEntities())
    {
        if (entity.name.find("Carrot") == std::string::npos) continue;
        if (!entity.active) continue;

        float dist = glm::length(
            bunny->transform.position - entity.transform.position);

        if (dist < m_PickupRadius) {
            entity.active = false;
            m_CarrotsCollected++;
            AudioManager::Get().PlaySFX("collect");
            std::cout << "[GameScene] Carrot collected! "
                << m_CarrotsCollected << "/" << m_CarrotsTotal << "\n";
        }
    }

    // 8. Abilities
    if (Input::IsKeyPressed(GLFW_KEY_J))
        AbilitySystem::TriggerAbility(m_ActiveBunnyType, AbilitySlot::J, bunny);
    if (Input::IsKeyPressed(GLFW_KEY_K))
        AbilitySystem::TriggerAbility(m_ActiveBunnyType, AbilitySlot::K, bunny);
    if (Input::IsKeyPressed(GLFW_KEY_L))
        AbilitySystem::TriggerAbility(m_ActiveBunnyType, AbilitySlot::L, bunny);

    // 9. Camera follow active bunny
    glm::vec2 target = bunny->transform.position;
    glm::vec2 current = m_Camera.GetPosition();
    glm::vec2 newCamPos = current + (target - current) * 5.0f * deltaTime;
    m_Camera.SetPosition(newCamPos);

    // update guard ai
    // Update guard AI
    if (!m_GameOver && !m_GameWon)
    {
        GuardAISystem::Update(
            deltaTime,
            m_Scene->GetEntities(),
            m_BunnyIDs
        );

        // Check lose condition
        if (GuardAISystem::IsBunnyCaught())
        {
            m_GameOver = true;
            AudioManager::Get().StopBGM();
            std::cout << "[GameScene] GAME OVER!\n";
        }

        // Check win condition — all carrots collected + reach exit
        if (m_CarrotsCollected >= m_CarrotsTotal && m_CarrotsTotal > 0)
        {
            // Check if active bunny is in exit zone
            auto exitZones = m_Tilemap.GetObjectsByType("Exit");
            for (const auto& exit : exitZones)
            {
                float exitX = exit.x * m_TileScale;
                float exitY = (m_Tilemap.GetData().mapHeight *
                    m_Tilemap.GetData().tileHeight - exit.y) * m_TileScale;
                float exitW = exit.width * m_TileScale;
                float exitH = exit.height * m_TileScale;

                Entity* bunny = GetEntity(m_ActiveBunnyID);
                if (!bunny) continue;

                if (bunny->transform.position.x >= exitX &&
                    bunny->transform.position.x <= exitX + exitW &&
                    bunny->transform.position.y >= exitY - exitH &&
                    bunny->transform.position.y <= exitY)
                {
                    m_GameWon = true;
                    AudioManager::Get().StopBGM();
                    std::cout << "[GameScene] YOU WIN!\n";
                }
            }
        }
    }
}

void GameScene::OnRenderGL()
{
    m_Renderer->SetViewProjection(m_Camera.GetViewProjection());

    // Render tilemap first (behind entities)
    m_Tilemap.Render(m_Renderer, m_TileScale);

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

    // Carrot counter
    ImGui::SetCursorScreenPos(ImVec2(vpPos.x + vpSize.x - 150, vpPos.y + 10));
    ImGui::TextColored(ImVec4(1.0f, 0.55f, 0.1f, 1.0f),
        "Carrots: %d / %d", m_CarrotsCollected, m_CarrotsTotal);

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

    // Game Over screen
    if (m_GameOver)
    {
        ImVec2 center = ImVec2(vpPos.x + vpSize.x * 0.5f - 100,
            vpPos.y + vpSize.y * 0.5f - 40);
        ImGui::SetCursorScreenPos(center);
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "GAME OVER!");
        ImGui::SetCursorScreenPos(ImVec2(center.x, center.y + 30));
        ImGui::TextColored(ImVec4(1, 1, 1, 1), "Grandma caught you!");
        ImGui::SetCursorScreenPos(ImVec2(center.x, center.y + 60));
        if (ImGui::Button("Try Again", ImVec2(200, 40)))
            SceneManager::Get().SwitchTo("Game");
        ImGui::SetCursorScreenPos(ImVec2(center.x, center.y + 110));
        if (ImGui::Button("Main Menu", ImVec2(200, 40)))
            SceneManager::Get().SwitchTo("MainMenu");
    }

    // Win screen
    if (m_GameWon)
    {
        ImVec2 center = ImVec2(vpPos.x + vpSize.x * 0.5f - 100,
            vpPos.y + vpSize.y * 0.5f - 40);
        ImGui::SetCursorScreenPos(center);
        ImGui::TextColored(ImVec4(1, 0.85f, 0.2f, 1), "YOU WIN!");
        ImGui::SetCursorScreenPos(ImVec2(center.x, center.y + 30));
        ImGui::TextColored(ImVec4(1, 1, 1, 1), "You stole all the carrots!");
        ImGui::SetCursorScreenPos(ImVec2(center.x, center.y + 60));
        if (ImGui::Button("Play Again", ImVec2(200, 40)))
            SceneManager::Get().SwitchTo("Game");
        ImGui::SetCursorScreenPos(ImVec2(center.x, center.y + 110));
        if (ImGui::Button("Main Menu", ImVec2(200, 40)))
            SceneManager::Get().SwitchTo("MainMenu");
    }

    // Back to menu
    ImGui::SetCursorScreenPos(ImVec2(vpPos.x + 10, vpPos.y + vpSize.y - 30.0f));
    if (ImGui::Button("< Main Menu"))
        SceneManager::Get().SwitchTo("MainMenu");
}

void GameScene::OnExit()
{
    std::cout << "[GameScene] Exited!\n";

    AudioManager::Get().StopBGM();
    m_Tilemap.Unload();
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