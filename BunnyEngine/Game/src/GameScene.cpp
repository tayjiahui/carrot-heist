#include "GameScene.h"
#include "Core/SceneManager.h"
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "Core/AudioManager.h"
#include "PauseMenu.h"
#include <algorithm>
#include "GuideBook.h"


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

    m_BunnyHiding.clear();
    m_BunnyHideSpot.clear();
    m_BunniesEscaped.clear();
    m_BunnyHiding[m_TofuID] = false;
    m_BunnyHiding[m_ToastID] = false;
    m_BunnyHideSpot[m_TofuID] = -1;
    m_BunnyHideSpot[m_ToastID] = -1;
    m_QuestComplete = false;

    // Setup pause menu
    PauseMenu::Get().Close();
    PauseMenu::Get().SetOnResume([this]() {
        std::cout << "[GameScene] Resumed\n";
        });
    PauseMenu::Get().SetOnMainMenu([this]() {
        SceneManager::Get().SwitchTo("MainMenu");
        });
    PauseMenu::Get().SetOnGuide([this]() {
        GuideBook::Get().Open();
    });

    // Load tilemap
    if (m_Tilemap.Load("Assets/Tilemaps/Level01.tmj"))
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
    
    m_CarrotsTotal = (int)m_Tilemap.GetObjectsByType("Carrot").size();
    std::cout << "[GameScene] Total carrots: " << m_CarrotsTotal << "\n";

    // Load hide spots from tilemap
    m_HideSpots.clear();
    for (const auto& obj : m_Tilemap.GetObjects())
    {
        if (obj.type != "HideSpot") continue;

        const TilemapData& data = m_Tilemap.GetData();
        float wx = obj.x * m_TileScale;
        float wy = (data.mapHeight * data.tileHeight - obj.y) * m_TileScale;
        float ww = obj.width * m_TileScale;
        float wh = obj.height * m_TileScale;

        m_HideSpots.push_back({ wx, wy, ww, wh });
        std::cout << "[GameScene] HideSpot at ("
            << wx << ", " << wy << ") size ("
            << ww << "x" << wh << ")\n";
    }
    std::cout << "[GameScene] Total HideSpots: " << m_HideSpots.size() << "\n";

    // Load NPCs from tilemap
    // Load NPCs from tilemap
    m_NPCs.clear();
    for (const auto& obj : m_Tilemap.GetObjects())
    {
        if (obj.type != "NPC_Tutorial" && obj.type != "NPC") continue;

        const TilemapData& data = m_Tilemap.GetData();
        NPCData npc;
        npc.name = obj.name;
        npc.position = {
            obj.x * m_TileScale,
            (data.mapHeight * data.tileHeight - obj.y) * m_TileScale
        };
        npc.triggerRadius = 2.0f;
        npc.dialogue = obj.properties.count("Dialogue")
            ? obj.properties.at("Dialogue")
            : "...";

        m_NPCs.push_back(npc);
        std::cout << "[GameScene] NPC: " << npc.name
            << " at (" << npc.position.x << ", " << npc.position.y << ")\n";
    }

    DialogueSystem::Get().Reset();
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

    // animation set up
    // Init Tofu animator
    m_TofuAnimator.Init("Assets/Textures/tofu_sprite.png", 50, 50);
    m_TofuAnimator.SetWorldSize({ 3.0f, 3.0f });

    // Idle
    m_TofuAnimator.AddClip({ "idle_right", { 0 },        0.5f,  true });
    m_TofuAnimator.AddClip({ "idle_left",  { 1 },        0.5f,  true });
    m_TofuAnimator.AddClip({ "idle_up",    { 2 },        0.5f,  true });
    m_TofuAnimator.AddClip({ "idle_down",  { 3 },        0.5f,  true });

    // Walk
    m_TofuAnimator.AddClip({ "walk_right", { 4,  5,  6  }, 0.12f, true });
    m_TofuAnimator.AddClip({ "walk_left",  { 7,  8,  9  }, 0.12f, true });
    m_TofuAnimator.AddClip({ "walk_up",    { 20, 21     }, 0.12f, true });
    m_TofuAnimator.AddClip({ "walk_down",  { 22, 23     }, 0.12f, true });

    // Ability J — Binky Dash
    m_TofuAnimator.AddClip({ "dash_right", { 10, 11, 12 }, 0.16f, false });
    m_TofuAnimator.AddClip({ "dash_left",  { 13, 14, 15 }, 0.16f, false });

    // Ability K — Knock
    m_TofuAnimator.AddClip({ "knock_right", { 16 }, 0.5f, false });
    m_TofuAnimator.AddClip({ "knock_left",  { 17 }, 0.5f, false });

    // Ability L — Squeeze
    m_TofuAnimator.AddClip({ "squeeze_up",   { 18 }, 0.5f, false });
    m_TofuAnimator.AddClip({ "squeeze_down", { 19 }, 0.5f, false });

    m_TofuAnimator.Play("idle_down");

    // Init Toast animator
    m_ToastAnimator.Init("Assets/Textures/toast_sprite.png", 50, 50);
    m_ToastAnimator.SetWorldSize({ 3.0f, 3.0f });

    // Idle
    m_ToastAnimator.AddClip({ "idle_right", { 0 },           0.5f,  true });
    m_ToastAnimator.AddClip({ "idle_left",  { 1 },           0.5f,  true });
    m_ToastAnimator.AddClip({ "idle_up",    { 2 },           0.5f,  true });
    m_ToastAnimator.AddClip({ "idle_down",  { 3 },           0.5f,  true });

    // Walk
    m_ToastAnimator.AddClip({ "walk_right", { 4,  5,  6  },  0.12f, true });
    m_ToastAnimator.AddClip({ "walk_left",  { 7,  8,  9  },  0.12f, true });
    m_ToastAnimator.AddClip({ "walk_up",    { 26, 27     },  0.12f, true });
    m_ToastAnimator.AddClip({ "walk_down",  { 28, 29     },  0.12f, true });

    // Ability J — Thump
    m_ToastAnimator.AddClip({ "thump_right", { 10, 11 },     0.15f, false });
    m_ToastAnimator.AddClip({ "thump_left",  { 12, 13 },     0.15f, false });

    // Ability K — Sniff
    m_ToastAnimator.AddClip({ "sniff_right", { 14, 15, 16, 17 }, 0.15f, false });
    m_ToastAnimator.AddClip({ "sniff_left",  { 18, 19, 20, 21 }, 0.15f, false });

    // Ability L — Dig
    m_ToastAnimator.AddClip({ "dig_right", { 22, 23 }, 0.15f, false });
    m_ToastAnimator.AddClip({ "dig_left",  { 24, 25 }, 0.15f, false });

    m_ToastAnimator.Play("idle_down");

    // Init Grandma animator
    m_GrandmaAnimator.Init("Assets/Textures/grandma_sprite.png", 59, 49);
    m_GrandmaAnimator.SetWorldSize({ 3.0f, 2.5f });

    // Idle
    m_GrandmaAnimator.AddClip({ "idle_down",  { 4,  5,  6,  7  }, 0.2f, true });
    m_GrandmaAnimator.AddClip({ "idle_up",    { 12, 13, 14, 15 }, 0.2f, true });
    m_GrandmaAnimator.AddClip({ "idle_left",  { 20, 21, 22, 23 }, 0.2f, true });
    m_GrandmaAnimator.AddClip({ "idle_right", { 20, 21, 22, 23 }, 0.2f, true }); // flip

    // Walk
    m_GrandmaAnimator.AddClip({ "walk_down",  { 8,  9,  10, 11 }, 0.15f, true });
    m_GrandmaAnimator.AddClip({ "walk_up",    { 16, 17, 18, 19 }, 0.15f, true });
    m_GrandmaAnimator.AddClip({ "walk_left",  { 24, 25, 26, 27 }, 0.15f, true });
    m_GrandmaAnimator.AddClip({ "walk_right", { 24, 25, 26, 27 }, 0.15f, true }); // flip

    m_GrandmaAnimator.Play("idle_down");


}

void GameScene::OnUpdate(float deltaTime)
{

    // Pause menu toggle
    if (Input::IsKeyPressed(GLFW_KEY_ESCAPE))
        PauseMenu::Get().Toggle();

    // Block all input when paused
    if (PauseMenu::Get().IsActive()) return;

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

	// 5. WASD movement - with restrictions based on what skill is being cast (eg. no movement during dash)
    glm::vec2 dir = { 0.0f, 0.0f };

    if (!m_TofuDashing || m_ActiveBunnyID != m_TofuID) {
        if (Input::IsKeyDown(GLFW_KEY_W)) dir.y += 1.0f;
        if (Input::IsKeyDown(GLFW_KEY_S)) dir.y -= 1.0f;
        if (Input::IsKeyDown(GLFW_KEY_A)) dir.x -= 1.0f;
        if (Input::IsKeyDown(GLFW_KEY_D)) dir.x += 1.0f;
    }

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
    // Ability J — Binky Dash
    if (Input::IsKeyPressed(GLFW_KEY_J) &&
        m_ActiveBunnyID == m_TofuID &&
        !m_TofuDashing &&
        AbilitySystem::GetCooldown(BunnyType::Tofu, AbilitySlot::J).IsReady())
    {
        // Start dash in current facing direction
        m_TofuDashing = true;
        m_TofuDashTimer = m_TofuDashDuration;

        // Get dash direction from facing
        FacingDir facing = m_TofuAnimator.GetFacing();
        switch (facing) {
        case FacingDir::Right: m_TofuDashDir = { 1.0f,  0.0f }; break;
        case FacingDir::Left:  m_TofuDashDir = { -1.0f,  0.0f }; break;
        case FacingDir::Up:    m_TofuDashDir = { 0.0f,  1.0f }; break;
        case FacingDir::Down:  m_TofuDashDir = { 0.0f, -1.0f }; break;
        }

        m_TofuAnimator.Play(
            facing == FacingDir::Left ? "dash_left" : "dash_right", true);

        AbilitySystem::TriggerAbility(BunnyType::Tofu, AbilitySlot::J,
            GetEntity(m_TofuID));
        AudioManager::Get().PlaySFX("dash");
        std::cout << "[Tofu] Binky Dash!\n";
    }

    // Apply dash movement
    if (m_TofuDashing)
    {
        m_TofuDashTimer -= deltaTime;

        Entity* tofu = GetEntity(m_TofuID);
        if (tofu)
        {
            glm::vec2 dashMove = m_TofuDashDir * m_TofuDashSpeed * deltaTime;
            glm::vec2 newPos = tofu->transform.position + dashMove;

            // Still check collision during dash
            float halfSize = 0.4f;
            bool collX = false;
            bool collY = false;

            glm::vec2 testX = { newPos.x, tofu->transform.position.y };
            if (m_Tilemap.IsCollision(testX.x + halfSize, testX.y, m_TileScale) ||
                m_Tilemap.IsCollision(testX.x - halfSize, testX.y, m_TileScale))
                collX = true;

            glm::vec2 testY = { tofu->transform.position.x, newPos.y };
            if (m_Tilemap.IsCollision(testY.x, testY.y + halfSize, m_TileScale) ||
                m_Tilemap.IsCollision(testY.x, testY.y - halfSize, m_TileScale))
                collY = true;

            if (!collX) tofu->transform.position.x = newPos.x;
            if (!collY) tofu->transform.position.y = newPos.y;

            // Stop dash if hit wall or timer ran out
            if ((collX || collY) || m_TofuDashTimer <= 0.0f)
            {
                m_TofuDashing = false;
                std::cout << "[Tofu] Dash ended\n";
            }
        }
    }
    if (Input::IsKeyPressed(GLFW_KEY_K))
        AbilitySystem::TriggerAbility(m_ActiveBunnyType, AbilitySlot::K, bunny);
    if (Input::IsKeyPressed(GLFW_KEY_L))
        AbilitySystem::TriggerAbility(m_ActiveBunnyType, AbilitySlot::L, bunny);

    // 9. Camera follow active bunny
    glm::vec2 target = bunny->transform.position;
    glm::vec2 current = m_Camera.GetPosition();
    glm::vec2 newCamPos = current + (target - current) * 5.0f * deltaTime;
    m_Camera.SetPosition(newCamPos);

    // --- Hide System ---
    Entity* activeBunny = GetEntity(m_ActiveBunnyID);
    if (activeBunny)
    {

        bool& isHiding = m_BunnyHiding[m_ActiveBunnyID];
        int& currentSpot = m_BunnyHideSpot[m_ActiveBunnyID];
        // Check if active bunny is inside any hidespot
        int insideSpot = -1;
        for (int i = 0; i < (int)m_HideSpots.size(); i++)
        {
            float hx = m_HideSpots[i].x;
            float hy = m_HideSpots[i].y - m_HideSpots[i].h;
            float hw = m_HideSpots[i].w;
            float hh = m_HideSpots[i].h;

            glm::vec2 pos = activeBunny->transform.position;
            if (pos.x >= hx && pos.x <= hx + hw &&
                pos.y >= hy && pos.y <= hy + hh)
            {
                insideSpot = i;
                break;
            }
        }

        // Press H to toggle hide
        if (Input::IsKeyPressed(GLFW_KEY_H))
        {
            if (insideSpot != -1)
            {
                isHiding = !isHiding;
                currentSpot = isHiding ? insideSpot : -1;
                std::cout << "[GameScene] "
                    << (isHiding ? "Hiding!" : "Stopped hiding!") << "\n";
            }
            else
            {
                isHiding = false;
                currentSpot = -1;
                std::cout << "[GameScene] Not inside a hidespot!\n";
            }
        }

        // Auto-unhide if bunny leaves the hidespot
        if (isHiding && insideSpot == -1)
        {
            isHiding = false;
            currentSpot = -1;
            std::cout << "[GameScene] Auto-unhidden\n";
        }
    }


    // update guard ai
    if (!m_GameOver && !m_GameWon)
    {
        // Build list of visible bunny IDs (hidden bunnies are invisible to guards)
        std::vector<int> visibleBunnies;
        for (int id : m_BunnyIDs)
        {
            if (m_BunnyHiding.count(id) && m_BunnyHiding[id]) continue;
            visibleBunnies.push_back(id);
        }

        GuardAISystem::Update(deltaTime, m_Scene->GetEntities(), visibleBunnies);

        // Check lose condition
        if (GuardAISystem::IsBunnyCaught())
        {
            m_GameOver = true;
            AudioManager::Get().StopBGM();
            std::cout << "[GameScene] GAME OVER!\n";
        }

        // Check win condition — all carrots collected + reach exit
        // Win — reach exit after quest is complete
        // Win — both bunnies must reach exit after quest complete
        if (m_QuestComplete && !m_GameWon)
        {
            auto exitZones = m_Tilemap.GetObjectsByType("Exit");
            for (const auto& exit : exitZones)
            {
                const TilemapData& data = m_Tilemap.GetData();
                float exitX = exit.x * m_TileScale;
                float exitY = (data.mapHeight * data.tileHeight - exit.y) * m_TileScale;
                float exitW = exit.width * m_TileScale;
                float exitH = exit.height * m_TileScale;

                // Check each bunny
                for (int bunnyID : m_BunnyIDs)
                {
                    // Skip already escaped bunnies
                    if (std::find(m_BunniesEscaped.begin(),
                        m_BunniesEscaped.end(),
                        bunnyID) != m_BunniesEscaped.end()) continue;

                    Entity* bunny = GetEntity(bunnyID);
                    if (!bunny) continue;

                    if (bunny->transform.position.x >= exitX &&
                        bunny->transform.position.x <= exitX + exitW &&
                        bunny->transform.position.y >= exitY - exitH &&
                        bunny->transform.position.y <= exitY)
                    {
                        m_BunniesEscaped.push_back(bunnyID);
                        std::cout << "[GameScene] " << bunny->name
                            << " escaped!\n";

                        // Show message
                        AudioManager::Get().PlaySFX("collect");
                    }
                }

                // Win when ALL bunnies escaped
                if ((int)m_BunniesEscaped.size() >= (int)m_BunnyIDs.size())
                {
                    m_GameWon = true;
                    AudioManager::Get().StopBGM();
                    std::cout << "[GameScene] ALL BUNNIES ESCAPED! YOU WIN!\n";
                }
            }
        }
    }

    // npc dialogue 
    // --- NPC Dialogue ---
    // --- NPC Dialogue ---
    if (!DialogueSystem::Get().IsActive() && !m_GameOver && !m_GameWon)
    {
        m_NearNPC = false;
        m_NearNPCIdx = -1;

        Entity* bunny = GetEntity(m_ActiveBunnyID);
        if (bunny)
        {
            for (int i = 0; i < (int)m_NPCs.size(); i++)
            {
                float dist = glm::length(
                    bunny->transform.position - m_NPCs[i].position);

                if (dist < m_NPCs[i].triggerRadius)
                {
                    m_NearNPC = true;
                    m_NearNPCIdx = i;
                    break;
                }
            }
        }

        // Press E to talk
        if (m_NearNPC && Input::IsKeyPressed(GLFW_KEY_E))
        {
            NPCData& npc = m_NPCs[m_NearNPCIdx];
            std::vector<DialogueLine> lines;

            if (!npc.talked)
            {
                lines.push_back({ "Master Whiskers",
                    "Recruits. Welcome to your first trial." });
                lines.push_back({ "Master Whiskers",
                    "Grandma's garden. Easy target. 8 carrots hidden inside." });
                lines.push_back({ "Master Whiskers",
                    "Steal them all without getting caught." });
                lines.push_back({ "Master Whiskers",
                    "If grandma spots you, hide in the bushes. Press H." });
                lines.push_back({ "Master Whiskers",
                    "Switch between recruits with 1 and 2." });
                lines.push_back({ "Master Whiskers",
                    "Don't embarrass me. Now go." });
                npc.talked = true;
            }
            else if (m_CarrotsCollected >= m_CarrotsTotal && m_CarrotsTotal > 0)
            {
                lines.push_back({ "Master Whiskers",
                    "Not bad for beginners." });
                lines.push_back({ "Master Whiskers",
                    "Now get to the burrow. Both of you." });
                lines.push_back({ "Master Whiskers",
                    "Leave no bunny behind. That's the code." });

                DialogueSystem::Get().StartDialogue(lines, [this]() {
                    m_QuestComplete = true;
                    });
                return;
            }
            else
            {
                int remaining = m_CarrotsTotal - m_CarrotsCollected;
                lines.push_back({ "Master Whiskers",
                    std::to_string(remaining) + " carrot(s) still out there." });
                lines.push_back({ "Master Whiskers",
                    "A true thief leaves nothing behind. Keep looking." });
            }

            DialogueSystem::Get().StartDialogue(lines);
        }
    }

    // Update dialogue
    DialogueSystem::Get().Update(deltaTime);

    // Advance dialogue with E
    if (DialogueSystem::Get().IsActive() &&
        Input::IsKeyPressed(GLFW_KEY_E))
        DialogueSystem::Get().Advance();


    // Track and update Tofu animation
    if (m_ActiveBunnyID == m_TofuID)
    {
        m_TofuMoveDir = dir;
        m_TofuAbilityJ = Input::IsKeyPressed(GLFW_KEY_J);
        m_TofuAbilityK = Input::IsKeyPressed(GLFW_KEY_K);
        m_TofuAbilityL = Input::IsKeyPressed(GLFW_KEY_L);
    }
    else
    {
        m_TofuMoveDir = { 0.0f, 0.0f };
        m_TofuAbilityJ = false;
        m_TofuAbilityK = false;
        m_TofuAbilityL = false;
    }

    // Update facing
    m_TofuAnimator.UpdateFacing(m_TofuMoveDir);

    // Determine clip
    bool isMoving = glm::length(m_TofuMoveDir) > 0.01f;
    FacingDir facing = m_TofuAnimator.GetFacing();

    if (!m_TofuDashing) {
        if (m_TofuAbilityK) {
            m_TofuAnimator.Play(
                facing == FacingDir::Left ? "knock_left" : "knock_right", true);
        }
        else if (m_TofuAbilityL) {
            m_TofuAnimator.Play(
                facing == FacingDir::Up ? "squeeze_up" : "squeeze_down", true);
        }
        else if (isMoving) {
            switch (facing) {
            case FacingDir::Right: m_TofuAnimator.Play("walk_right"); break;
            case FacingDir::Left:  m_TofuAnimator.Play("walk_left");  break;
            case FacingDir::Up:    m_TofuAnimator.Play("walk_up");    break;
            case FacingDir::Down:  m_TofuAnimator.Play("walk_down");  break;
            }
        }
        else {
            switch (facing) {
            case FacingDir::Right: m_TofuAnimator.Play("idle_right"); break;
            case FacingDir::Left:  m_TofuAnimator.Play("idle_left");  break;
            case FacingDir::Up:    m_TofuAnimator.Play("idle_up");    break;
            case FacingDir::Down:  m_TofuAnimator.Play("idle_down");  break;
            }
        }
    }
    m_TofuAnimator.Update(deltaTime);

    // Track Toast movement
    if (m_ActiveBunnyID == m_ToastID)
    {
        m_ToastMoveDir = dir;
        m_ToastAbilityJ = Input::IsKeyPressed(GLFW_KEY_J);
        m_ToastAbilityK = Input::IsKeyPressed(GLFW_KEY_K);
        m_ToastAbilityL = Input::IsKeyPressed(GLFW_KEY_L);
    }
    else
    {
        m_ToastMoveDir = { 0.0f, 0.0f };
        m_ToastAbilityJ = false;
        m_ToastAbilityK = false;
        m_ToastAbilityL = false;
    }

    // Update Toast facing
    m_ToastAnimator.UpdateFacing(m_ToastMoveDir);

    bool toastMoving = glm::length(m_ToastMoveDir) > 0.01f;
    FacingDir toastFacing = m_ToastAnimator.GetFacing();

    if (m_ToastAbilityJ) {
        m_ToastAnimator.Play(
            toastFacing == FacingDir::Left ? "thump_left" : "thump_right", true);
    }
    else if (m_ToastAbilityK) {
        m_ToastAnimator.Play(
            toastFacing == FacingDir::Left ? "sniff_left" : "sniff_right", true);
    }
    else if (m_ToastAbilityL) {
        m_ToastAnimator.Play(
            toastFacing == FacingDir::Left ? "dig_left" : "dig_right", true);
    }
    else if (toastMoving) {
        switch (toastFacing) {
        case FacingDir::Right: m_ToastAnimator.Play("walk_right"); break;
        case FacingDir::Left:  m_ToastAnimator.Play("walk_left");  break;
        case FacingDir::Up:    m_ToastAnimator.Play("walk_up");    break;
        case FacingDir::Down:  m_ToastAnimator.Play("walk_down");  break;
        }
    }
    else {
        switch (toastFacing) {
        case FacingDir::Right: m_ToastAnimator.Play("idle_right"); break;
        case FacingDir::Left:  m_ToastAnimator.Play("idle_left");  break;
        case FacingDir::Up:    m_ToastAnimator.Play("idle_up");    break;
        case FacingDir::Down:  m_ToastAnimator.Play("idle_down");  break;
        }
    }

    m_ToastAnimator.Update(deltaTime);

    // Update Grandma animation based on guard state
    // Update Grandma animation based on guard state
    if (!GuardAISystem::GetGuards().empty())
    {
        const GuardData& guard = GuardAISystem::GetGuards()[0];
        Entity* grandma = GetEntity(guard.entityID);

        if (grandma)
        {
            bool grandmaMoving = (guard.state == GuardState::Patrol ||
                guard.state == GuardState::Alerted ||
                guard.state == GuardState::Searching);

            // Use guard's facing angle to determine direction
            // guard.facing is in degrees, 0 = right, 90 = up, -90 = down, 180 = left
            float angle = guard.facing;

            // Determine facing direction from angle
            FacingDir gFacing = FacingDir::Down; // default

            if (angle > -45.0f && angle <= 45.0f)   gFacing = FacingDir::Right;
            else if (angle > 45.0f && angle <= 135.0f)  gFacing = FacingDir::Up;
            else if (angle > 135.0f || angle <= -135.0f) gFacing = FacingDir::Left;
            else if (angle > -135.0f && angle <= -45.0f)  gFacing = FacingDir::Down;

            // Debug
            static FacingDir lastFacing = FacingDir::Down;
            if (gFacing != lastFacing) {
                std::cout << "[Grandma] Facing changed: " << (int)gFacing
                    << " angle: " << angle << "\n";
                lastFacing = gFacing;
            }

            if (grandmaMoving)
            {
                switch (gFacing) {
                case FacingDir::Down:  m_GrandmaAnimator.Play("walk_down");  break;
                case FacingDir::Up:    m_GrandmaAnimator.Play("walk_up");    break;
                case FacingDir::Left:  m_GrandmaAnimator.Play("walk_left");  break;
                case FacingDir::Right: m_GrandmaAnimator.Play("walk_right"); break;
                }
            }
            else
            {
                switch (gFacing) {
                case FacingDir::Down:  m_GrandmaAnimator.Play("idle_down");  break;
                case FacingDir::Up:    m_GrandmaAnimator.Play("idle_up");    break;
                case FacingDir::Left:  m_GrandmaAnimator.Play("idle_left");  break;
                case FacingDir::Right: m_GrandmaAnimator.Play("idle_right"); break;
                }
            }

            // Set flipX for right facing
            // We handle this in render
        }
    }

    m_GrandmaAnimator.Update(deltaTime);
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

        // Tofu — use animated sprite
        if (entity.id == m_TofuID)
        {
            AnimationFrame frame = m_TofuAnimator.GetCurrentFrame();

            // Highlight if selected or active
            glm::vec4 tint = glm::vec4(1.0f);
            if (entity.id == m_ActiveBunnyID)
                tint = glm::vec4(1.2f, 1.2f, 1.2f, 1.0f);
            if (m_BunnyHiding.count(entity.id) && m_BunnyHiding[entity.id])
                tint = glm::vec4(0.3f, 0.5f, 1.0f, 0.6f);

            // Draw sprite — scale up from 50px to world units
            // 50px / 16px per unit = ~3 units wide
            glm::vec2 spriteSize = { 3.0f, 3.0f };


            m_Renderer->DrawFrame(
                entity.transform.position,
                spriteSize,
                0.0f,
                m_TofuAnimator.GetTextureID(),
                frame.uvMin,
                frame.uvMax,
                tint
            );
            continue;
        }

        // Toast — animated sprite
        if (entity.id == m_ToastID)
        {
            AnimationFrame frame = m_ToastAnimator.GetCurrentFrame();

            glm::vec4 tint = glm::vec4(1.0f);
            if (entity.id == m_ActiveBunnyID)
                tint = glm::vec4(1.2f, 1.2f, 1.2f, 1.0f);
            if (m_BunnyHiding.count(entity.id) && m_BunnyHiding[entity.id])
                tint = glm::vec4(0.3f, 0.5f, 1.0f, 0.6f);

            m_Renderer->DrawFrame(
                entity.transform.position,
                m_ToastAnimator.GetWorldSize(),
                0.0f,
                m_ToastAnimator.GetTextureID(),
                frame.uvMin,
                frame.uvMax,
                tint
            );
            continue;
        }

        // Grandma — animated sprite
        // Grandma — animated sprite
        if (entity.name.find("Grandma") != std::string::npos)
        {
            AnimationFrame frame = m_GrandmaAnimator.GetCurrentFrame();

            bool      flipX = false;
            glm::vec4 tint = glm::vec4(1.0f);

            if (!GuardAISystem::GetGuards().empty())
            {
                const GuardData& guard = GuardAISystem::GetGuards()[0];
                float angle = guard.facing;

                flipX = (angle > -45.0f && angle <= 45.0f);

                // Debug
                static float lastAngle = -999.0f;
                if (std::abs(angle - lastAngle) > 1.0f) {
                    std::cout << "[Grandma] angle=" << angle
                        << " flipX=" << flipX << "\n";
                    lastAngle = angle;
                }

                if (guard.state == GuardState::Alerted)
                    tint = glm::vec4(1.5f, 0.5f, 0.5f, 1.0f);
                else if (guard.state == GuardState::Suspicious)
                    tint = glm::vec4(1.5f, 1.2f, 0.5f, 1.0f);
            }

            m_Renderer->DrawFrame(
                entity.transform.position,
                m_GrandmaAnimator.GetWorldSize(),
                0.0f,
                m_GrandmaAnimator.GetTextureID(),
                frame.uvMin,
                frame.uvMax,
                tint,
                flipX
            );
            continue;
        }
        if (!GuardAISystem::GetGuards().empty())
        {
            const GuardData& guard = GuardAISystem::GetGuards()[0];
            static float lastAngle = -999.0f;
            if (std::abs(guard.facing - lastAngle) > 1.0f)
            {
                std::cout << "[Grandma Render] Facing angle: " << guard.facing
                    << " flipX: " << (guard.facing > -45.0f && guard.facing <= 45.0f)
                    << "\n";
                lastAngle = guard.facing;
            }
        }

        // All other entities — colored quads as before
        glm::vec4 color = { 0.8f, 0.8f, 0.8f, 1.0f };
        if (entity.name.find("Guard") != std::string::npos) color = { 0.80f, 0.20f, 0.20f, 1.0f };
        if (entity.name.find("Carrot") != std::string::npos) color = { 1.00f, 0.55f, 0.10f, 1.0f };

        if (entity.id == m_ActiveBunnyID)
            color = glm::vec4(glm::vec3(color) * 1.5f, 1.0f);
        if (m_BunnyHiding.count(entity.id) && m_BunnyHiding[entity.id])
            color = glm::vec4(0.3f, 0.5f, 1.0f, 0.6f);

        m_Renderer->DrawQuad(
            entity.transform.position,
            entity.transform.scale,
            entity.transform.rotation,
            color
        );
    }

    // Draw hidespot outlines so player can see them
    for (auto& spot : m_HideSpots)
    {
        float hx = spot.x + spot.w * 0.5f;
        float hy = spot.y - spot.h * 0.5f;
        float hw = spot.w;
        float hh = spot.h;

        // Green outline when not hiding, blue when hiding
        bool activeIsHiding = m_BunnyHiding.count(m_ActiveBunnyID)
            && m_BunnyHiding[m_ActiveBunnyID];

        glm::vec4 spotColor = activeIsHiding
            ? glm::vec4(0.3f, 0.5f, 1.0f, 0.2f)
            : glm::vec4(0.2f, 0.8f, 0.2f, 0.15f);

        m_Renderer->DrawQuad({ hx, hy }, { hw, hh }, 0.0f, spotColor);
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

    // Hide status
    ImGui::SetCursorScreenPos(ImVec2(vpPos.x + 10, vpPos.y + 50));
    bool activeIsHiding = m_BunnyHiding.count(m_ActiveBunnyID)
        && m_BunnyHiding[m_ActiveBunnyID];

    if (activeIsHiding)
        ImGui::TextColored(ImVec4(0.3f, 0.5f, 1.0f, 1.0f), "HIDING [H to unhide]");
    else
    {
        Entity* b = GetEntity(m_ActiveBunnyID);
        bool nearSpot = false;
        if (b)
        {
            for (auto& spot : m_HideSpots)
            {
                float hx = spot.x;
                float hy = spot.y - spot.h;
                if (b->transform.position.x >= hx &&
                    b->transform.position.x <= hx + spot.w &&
                    b->transform.position.y >= hy &&
                    b->transform.position.y <= hy + spot.h)
                {
                    nearSpot = true;
                    break;
                }
            }
        }
        if (nearSpot)
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Press H to hide");
    }

    // NPC interaction prompt
    if (m_NearNPC && !DialogueSystem::Get().IsActive()
        && !m_GameOver && !m_GameWon)
    {
        ImGui::SetCursorScreenPos(
            ImVec2(vpPos.x + vpSize.x * 0.5f - 60,
                vpPos.y + vpSize.y * 0.5f - 60));
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f),
            "[ E ] Talk");
    }

    // Dialogue box
    DialogueSystem::Get().RenderImGui(
        vpPos.x, vpPos.y, vpSize.x, vpSize.y);



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

    // Escape progress when quest complete
    if (m_QuestComplete && !m_GameWon)
    {
        ImGui::SetCursorScreenPos(
            ImVec2(vpPos.x + vpSize.x * 0.5f - 100,
                vpPos.y + 10));
        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f),
            "Get both bunnies to the burrow! (%d/%d escaped)",
            (int)m_BunniesEscaped.size(),
            (int)m_BunnyIDs.size());
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

    // Pause menu renders on top of everything
    PauseMenu::Get().RenderImGui(
        vpPos.x, vpPos.y, vpSize.x, vpSize.y);
}

void GameScene::OnExit()
{
    std::cout << "[GameScene] Exited!\n";

    AudioManager::Get().StopBGM();
    m_Tilemap.Unload();
    m_TofuAnimator.Shutdown();
    m_ToastAnimator.Shutdown();
    m_GrandmaAnimator.Shutdown();
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