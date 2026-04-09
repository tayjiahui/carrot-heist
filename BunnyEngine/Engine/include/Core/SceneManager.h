#pragma once
#include "Core/IScene.h"
#include <unordered_map>
#include <string>

class SceneManager
{
public:
    // Singleton
    static SceneManager& Get() {
        static SceneManager instance;
        return instance;
    }

    void RenderGL();
    void RenderImGui();
    void RegisterScene(const std::string& name, IScene* scene);
    void SwitchTo(const std::string& name);
    void Update(float deltaTime);
    void Render();
    void Shutdown();

    const std::string& GetCurrentSceneName() const { return m_CurrentName; }

private:
    SceneManager() = default;

    std::unordered_map<std::string, IScene*> m_Scenes;
    IScene* m_CurrentScene = nullptr;
    std::string m_CurrentName = "";
    std::string m_PendingSwitch = "";
    bool        m_HasPending = false;
};