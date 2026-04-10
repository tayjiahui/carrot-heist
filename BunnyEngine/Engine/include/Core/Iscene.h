#pragma once
#include "ECS/Entity.h"
#include <vector>
#include <string>

class Scene;

class IScene
{
public:
    virtual ~IScene() = default;

    virtual void OnEnter() = 0;
    virtual void OnUpdate(float deltaTime) = 0;
    virtual void OnRenderGL() = 0;
    virtual void OnRenderImGui() = 0;
    virtual void OnExit() = 0;

    virtual std::vector<Entity>* GetEntities() { return nullptr; }
    virtual const std::string& GetSceneName() { static std::string s = "Unknown"; return s; }
    virtual Scene* GetScene() { return nullptr; }
};