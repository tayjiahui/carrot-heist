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
};