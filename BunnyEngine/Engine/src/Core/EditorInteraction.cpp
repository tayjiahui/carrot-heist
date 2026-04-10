#include "Core/EditorInteraction.h"
#include "Core/Input.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <GLFW/glfw3.h>
#include <iostream>

bool      EditorInteraction::s_DragMode = false;
bool      EditorInteraction::s_Dragging = false;
glm::vec2 EditorInteraction::s_DragOffset = { 0.0f, 0.0f };
glm::vec2 EditorInteraction::s_LastMouseWorldPos = { 0.0f, 0.0f };

void EditorInteraction::Update(
    Scene* scene,
    Framebuffer* framebuffer,
    int& selectedEntityID,
    bool         viewportHovered)
{
    if (!scene || !framebuffer) return;

    // Toggle drag mode with Alt
    if (Input::IsKeyPressed(GLFW_KEY_LEFT_ALT) ||
        Input::IsKeyPressed(GLFW_KEY_RIGHT_ALT))
    {
        s_DragMode = !s_DragMode;
        s_Dragging = false;
        std::cout << "[Editor] Drag mode: " << (s_DragMode ? "ON" : "OFF") << "\n";
    }

    if (!viewportHovered) return;

    // Get viewport info from ImGui
    ImGuiWindow* vpWin = ImGui::FindWindowByName("Viewport");
    if (!vpWin) return;

    glm::vec2 vpPos = { vpWin->InnerRect.Min.x, vpWin->InnerRect.Min.y };
    glm::vec2 vpSize = { vpWin->InnerRect.GetSize().x, vpWin->InnerRect.GetSize().y };

    glm::vec2 mousePos = Input::GetMousePos();
    glm::vec2 worldPos = ScreenToWorld(mousePos, vpPos, vpSize, framebuffer);

    if (!s_DragMode)
    {
        // Normal mode — left click to select
        if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            selectedEntityID = -1;
            // Hit test in reverse order (top entity first)
            auto& entities = scene->GetEntities();
            for (int i = (int)entities.size() - 1; i >= 0; i--)
            {
                if (HitTest(entities[i], worldPos)) {
                    selectedEntityID = entities[i].id;
                    std::cout << "[Editor] Selected: " << entities[i].name << "\n";
                    break;
                }
            }
        }
        return;
    }

    // Drag mode
    if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
    {
        // Try to start drag on selected entity
        if (selectedEntityID != -1) {
            for (auto& e : scene->GetEntities()) {
                if (e.id == selectedEntityID && HitTest(e, worldPos)) {
                    s_Dragging = true;
                    s_DragOffset = e.transform.position - worldPos;
                    break;
                }
            }
        }
        // If not on selected, try to select a new one
        if (!s_Dragging) {
            for (int i = (int)scene->GetEntities().size() - 1; i >= 0; i--) {
                auto& e = scene->GetEntities()[i];
                if (HitTest(e, worldPos)) {
                    selectedEntityID = e.id;
                    s_Dragging = true;
                    s_DragOffset = e.transform.position - worldPos;
                    break;
                }
            }
        }
    }

    // While dragging
    if (s_Dragging && Input::IsMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT))
    {
        for (auto& e : scene->GetEntities()) {
            if (e.id == selectedEntityID) {
                e.transform.position = worldPos + s_DragOffset;
                break;
            }
        }
    }

    // Release drag
    if (Input::IsMouseButtonReleased(GLFW_MOUSE_BUTTON_LEFT))
        s_Dragging = false;

    s_LastMouseWorldPos = worldPos;
}

glm::vec2 EditorInteraction::ScreenToWorld(
    glm::vec2    mousePos,
    glm::vec2    vpPos,
    glm::vec2    vpSize,
    Framebuffer* framebuffer)
{
    // Normalize mouse to 0-1 within viewport
    float nx = (mousePos.x - vpPos.x) / vpSize.x;
    float ny = (mousePos.y - vpPos.y) / vpSize.y;

    // Flip Y (screen Y goes down, world Y goes up)
    ny = 1.0f - ny;

    // Map to world space (-10 to 10 range with aspect)
    float aspect = vpSize.x / vpSize.y;
    float worldX = (nx * 2.0f - 1.0f) * 10.0f * aspect;
    float worldY = (ny * 2.0f - 1.0f) * 10.0f;

    return { worldX, worldY };
}

bool EditorInteraction::HitTest(const Entity& entity, glm::vec2 worldPos)
{
    // AABB hit test using entity scale as size
    glm::vec2 halfSize = entity.transform.scale * 0.5f;
    glm::vec2 pos = entity.transform.position;

    return worldPos.x >= pos.x - halfSize.x &&
        worldPos.x <= pos.x + halfSize.x &&
        worldPos.y >= pos.y - halfSize.y &&
        worldPos.y <= pos.y + halfSize.y;
}