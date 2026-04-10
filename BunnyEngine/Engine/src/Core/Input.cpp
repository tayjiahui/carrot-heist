#include "Core/Input.h"
#include <cstring>

// Static member definitions
GLFWwindow* Input::s_Window = nullptr;

bool Input::s_KeysCurrent[GLFW_KEY_LAST + 1] = {};
bool Input::s_KeysPrevious[GLFW_KEY_LAST + 1] = {};

bool Input::s_MouseCurrent[GLFW_MOUSE_BUTTON_LAST + 1] = {};
bool Input::s_MousePrevious[GLFW_MOUSE_BUTTON_LAST + 1] = {};

glm::vec2 Input::s_MousePos = { 0.0f, 0.0f };
glm::vec2 Input::s_MousePosPrev = { 0.0f, 0.0f };
float     Input::s_ScrollDelta = 0.0f;
float     Input::s_ScrollDeltaThisFrame = 0.0f;

void Input::Update(GLFWwindow* window)
{
    s_Window = window;

    // Shift current to previous
    memcpy(s_KeysPrevious, s_KeysCurrent, sizeof(s_KeysCurrent));
    memcpy(s_MousePrevious, s_MouseCurrent, sizeof(s_MouseCurrent));

    // Sample current keyboard state
    for (int i = 0; i <= GLFW_KEY_LAST; i++)
        s_KeysCurrent[i] = glfwGetKey(window, i) == GLFW_PRESS;

    // Sample current mouse button state
    for (int i = 0; i <= GLFW_MOUSE_BUTTON_LAST; i++)
        s_MouseCurrent[i] = glfwGetMouseButton(window, i) == GLFW_PRESS;

    // Mouse position
    s_MousePosPrev = s_MousePos;
    double mx, my;
    glfwGetCursorPos(window, &mx, &my);
    s_MousePos = { (float)mx, (float)my };

    // Scroll — consume this frame's scroll
    s_ScrollDelta = s_ScrollDeltaThisFrame;
    s_ScrollDeltaThisFrame = 0.0f;
}

void Input::OnScroll(float delta)
{
    s_ScrollDeltaThisFrame += delta;
}

bool Input::IsKeyDown(int key)
{
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    return s_KeysCurrent[key];
}

bool Input::IsKeyPressed(int key)
{
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    return s_KeysCurrent[key] && !s_KeysPrevious[key];
}

bool Input::IsKeyReleased(int key)
{
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    return !s_KeysCurrent[key] && s_KeysPrevious[key];
}

bool Input::IsMouseButtonDown(int button)
{
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
    return s_MouseCurrent[button];
}

bool Input::IsMouseButtonPressed(int button)
{
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
    return s_MouseCurrent[button] && !s_MousePrevious[button];
}

bool Input::IsMouseButtonReleased(int button)
{
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
    return !s_MouseCurrent[button] && s_MousePrevious[button];
}

glm::vec2 Input::GetMousePos()
{
    return s_MousePos;
}

glm::vec2 Input::GetMouseDelta()
{
    return s_MousePos - s_MousePosPrev;
}

float Input::GetScrollDelta()
{
    return s_ScrollDelta;
}