#pragma once
#include "Core/IScene.h"
#include "Renderer/Renderer2D.h"
#include "Renderer/Framebuffer.h"
#include "Scene/Scene.h"
#include "ECS/Entity.h"
#include "Core/Input.h"
#include "Systems/AbilitySystem.h"
#include <glm/glm.hpp>
#include <vector>
#include "Scene/Camera2D.h"
#include "Renderer/TilemapRenderer.h"
#include "Systems/GuardAI.h"
#include "DialogueSystem.h"
#include <unordered_map>
#include "Renderer/CharacterAnimator.h"
#include "Renderer/Texture.h"

class GameScene : public IScene
{
public:
    GameScene(Renderer2D* renderer, Framebuffer* framebuffer);
    ~GameScene();

    Scene* GetScene() override { return m_Scene; }
    void OnEnter()                 override;
    void OnUpdate(float deltaTime) override;
    void OnRenderGL()              override;
    void OnRenderImGui()           override;
    void OnExit()                  override;

    std::vector<Entity>* GetEntities()  override;
    const std::string& GetSceneName() override;

    int   m_CarrotsCollected = 0;
    int   m_CarrotsTotal = 0;
    float m_PickupRadius = 0.8f;
    bool m_QuestComplete = false;

    std::vector<int> m_BunniesEscaped;

    // tofu's binky dash
    // Binky dash state
    bool      m_TofuDashing = false;
    float     m_TofuDashTimer = 0.0f;
    float     m_TofuDashDuration = 0.5f;
    float     m_TofuDashSpeed = 12.0f;
    glm::vec2 m_TofuDashDir = { 0.0f, 0.0f };

    CharacterAnimator m_GrandmaAnimator;

    // Pixel-art carrot pickup sprite
    Texture* m_CarrotTex = nullptr;

    // Master Whiskers (NPC) sprite: tofu.png drawn at double bunny size
    Texture* m_WhiskersTex = nullptr;
    glm::vec2         m_GrandmaMoveDir = { 0.0f, 0.0f };

private:
    Entity* GetEntity(int id);
    void    CycleBunny(int dir);
    void    RenderHUD();

    Camera2D m_Camera;

    Renderer2D* m_Renderer = nullptr;
    Framebuffer* m_Framebuffer = nullptr;
    Scene* m_Scene = nullptr;

    // Bunny IDs
    int m_TofuID = -1;
    int m_ToastID = -1;
    int m_ActiveBunnyID = -1;

    // Active bunny type for ability system
    BunnyType m_ActiveBunnyType = BunnyType::Tofu;

    float m_Speed = 5.0f;

    // tile map
    TilemapRenderer m_Tilemap;
    float           m_TileScale = 1.0f;

    // guard ai 
    std::vector<int> m_BunnyIDs;
    bool             m_GameOver = false;
    bool             m_GameWon = false;

    // hiding spots
    std::unordered_map<int, bool> m_BunnyHiding;     // bunnyID -> isHiding
    std::unordered_map<int, int>  m_BunnyHideSpot; // bunnyID -> hideSpotIndex

    struct HideSpot {
        float x, y, w, h;
    };
    std::vector<HideSpot> m_HideSpots;

    struct NPCData {
        std::string name;
        glm::vec2   position;
        float       triggerRadius;
        std::string dialogue;
        bool        talked = false;
    };

    std::vector<NPCData> m_NPCs;
    bool                 m_NearNPC = false;
    int                  m_NearNPCIdx = -1;

    // animation spritesheet
    CharacterAnimator m_TofuAnimator;
    CharacterAnimator m_ToastAnimator;

    glm::vec2 m_TofuMoveDir = { 0.0f, 0.0f };
    bool      m_TofuAbilityJ = false;
    bool      m_TofuAbilityK = false;
    bool      m_TofuAbilityL = false;

    glm::vec2         m_ToastMoveDir = { 0.0f, 0.0f };
    bool              m_ToastAbilityJ = false;
    bool              m_ToastAbilityK = false;
    bool              m_ToastAbilityL = false;
};