#pragma once
#include "ECS/Entity.h"
#include "Renderer/Framebuffer.h"
#include "Scene/Scene.h"
#include <glm/glm.hpp>
#include <vector>

class EditorInteraction
{
public:
    // Call every frame from editor
    static void Update(
        Scene* scene,
        Framebuffer* framebuffer,
        int& selectedEntityID,
        bool         viewportHovered
    );

    static bool IsDragModeActive() { return s_DragMode; }

private:
    // Convert screen mouse pos to world space
    static glm::vec2 ScreenToWorld(
        glm::vec2    mousePos,
        glm::vec2    viewportPos,
        glm::vec2    viewportSize,
        Framebuffer* framebuffer
    );

    // Check if world pos is inside entity bounds
    static bool HitTest(const Entity& entity, glm::vec2 worldPos);

    static bool      s_DragMode;
    static bool      s_Dragging;
    static glm::vec2 s_DragOffset;
    static glm::vec2 s_LastMouseWorldPos;
};