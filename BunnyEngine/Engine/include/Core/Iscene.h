#pragma once

class IScene
{
public:
    virtual ~IScene() = default;

    virtual void OnEnter() = 0;
    virtual void OnUpdate(float deltaTime) = 0;
    virtual void OnRenderGL() = 0; // OpenGL draw calls
    virtual void OnRenderImGui() = 0; // ImGui draw calls
    virtual void OnExit() = 0;
};