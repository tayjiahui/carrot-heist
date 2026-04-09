#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera2D
{
public:
    Camera2D() = default;

    // Call every frame with viewport size
    void Update(float viewportWidth, float viewportHeight);

    // Call with mouse input
    void OnMousePan(float deltaX, float deltaY);
    void OnMouseZoom(float scrollDelta);

    const glm::mat4& GetViewProjection() const { return m_ViewProjection; }

    // Getters for debug
    const glm::vec2& GetPosition() const { return m_Position; }
    float            GetZoom()     const { return m_Zoom; }

    // Reset
    void Reset() { m_Position = { 0,0 }; m_Zoom = 1.0f; }

private:
    void RecalculateMatrix(float viewportWidth, float viewportHeight);

    glm::vec2 m_Position = { 0.0f, 0.0f };
    float     m_Zoom = 1.0f;

    float     m_ZoomMin = 0.1f;
    float     m_ZoomMax = 10.0f;
    float     m_PanSpeed = 1.0f; // units per pixel

    glm::mat4 m_ViewProjection = glm::mat4(1.0f);
};