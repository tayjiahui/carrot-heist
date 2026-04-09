#include "Camera2D.h"
#include <algorithm>

void Camera2D::Update(float viewportWidth, float viewportHeight)
{
    RecalculateMatrix(viewportWidth, viewportHeight);
}

void Camera2D::OnMousePan(float deltaX, float deltaY)
{
    // Pan speed scales with zoom so panning feels consistent
    float speed = m_PanSpeed / m_Zoom;
    m_Position.x -= deltaX * speed * 0.01f;
    m_Position.y += deltaY * speed * 0.01f;
}

void Camera2D::OnMouseZoom(float scrollDelta)
{
    m_Zoom += scrollDelta * 0.1f * m_Zoom;
    m_Zoom = std::clamp(m_Zoom, m_ZoomMin, m_ZoomMax);
}

void Camera2D::RecalculateMatrix(float viewportWidth, float viewportHeight)
{
    if (viewportWidth <= 0 || viewportHeight <= 0) return;

    float aspect = viewportWidth / viewportHeight;
    float orthoSize = 10.0f / m_Zoom; // zoom in = smaller ortho size

    glm::mat4 proj = glm::ortho(
        -orthoSize * aspect, orthoSize * aspect,
        -orthoSize, orthoSize,
        -1.0f, 1.0f
    );

    glm::mat4 view = glm::translate(glm::mat4(1.0f),
        glm::vec3(-m_Position, 0.0f));

    m_ViewProjection = proj * view;
}