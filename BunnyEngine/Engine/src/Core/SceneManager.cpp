#include "Core/SceneManager.h"
#include <iostream>

void SceneManager::RegisterScene(const std::string& name, IScene* scene)
{
    m_Scenes[name] = scene;
    std::cout << "[SceneManager] Registered: " << name << "\n";
}

void SceneManager::SwitchTo(const std::string& name)
{
    m_PendingSwitch = name;
    m_HasPending = true;
}

void SceneManager::Update(float deltaTime)
{
    if (m_HasPending)
    {
        auto it = m_Scenes.find(m_PendingSwitch);
        if (it == m_Scenes.end()) {
            std::cerr << "[SceneManager] Scene not found: " << m_PendingSwitch << "\n";
            m_HasPending = false;
            return;
        }

        if (m_CurrentScene)
            m_CurrentScene->OnExit();

        m_CurrentScene = it->second;
        m_CurrentName = m_PendingSwitch;
        m_CurrentScene->OnEnter();
        m_HasPending = false;
    }

    if (m_CurrentScene)
        m_CurrentScene->OnUpdate(deltaTime);
}

void SceneManager::RenderGL()
{
    if (m_CurrentScene)
        m_CurrentScene->OnRenderGL();
}

void SceneManager::RenderImGui()
{
    if (m_CurrentScene)
        m_CurrentScene->OnRenderImGui();
}

void SceneManager::Shutdown()
{
    if (m_CurrentScene)
        m_CurrentScene->OnExit();

    for (auto& [name, scene] : m_Scenes)
        delete scene;

    m_Scenes.clear();
    m_CurrentScene = nullptr;
}