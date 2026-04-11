#pragma once
#include "Core/IScene.h"
#include "Renderer/Renderer2D.h"
#include "Renderer/Texture.h"
#include "Renderer/Framebuffer.h"
#include <glm/glm.hpp>

class MainMenuScene : public IScene
{
public:
    MainMenuScene(Renderer2D* renderer);
    ~MainMenuScene();

    void OnEnter()                 override;
    void OnUpdate(float deltaTime) override;
    void OnRenderGL()              override;
    void OnRenderImGui()           override;
    void OnExit()                  override;
    MainMenuScene(Renderer2D* renderer, Framebuffer* framebuffer);


    // Screen dimensions updated each frame
    float m_ScreenW = 1920.0f;
    float m_ScreenH = 1080.0f;

    std::vector<Entity>* GetEntities() override { return nullptr; }
    const std::string& GetSceneName() override {
        static std::string name = "MainMenu";
        return name;
    }

private:
    Renderer2D* m_Renderer = nullptr;
    Framebuffer* m_Framebuffer = nullptr;

    // Textures
    Texture* m_Background = nullptr;
    Texture* m_BtnPlay = nullptr;
    Texture* m_BtnGuide = nullptr;
    Texture* m_BtnOptions = nullptr;
    Texture* m_BtnQuit = nullptr;

};