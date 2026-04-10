#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Input
{
public:
    // Call once per frame at start of game loop
    static void Update(GLFWwindow* window);

    // Keyboard
    static bool IsKeyDown(int key); // held down
    static bool IsKeyPressed(int key); // just pressed this frame
    static bool IsKeyReleased(int key); // just released this frame

    // Mouse buttons
    static bool IsMouseButtonDown(int button);
    static bool IsMouseButtonPressed(int button);
    static bool IsMouseButtonReleased(int button);

    // Mouse position
    static glm::vec2 GetMousePos();
    static glm::vec2 GetMouseDelta();
    static float     GetScrollDelta();

    // Call from scroll callback
    static void OnScroll(float delta);

private:
    static GLFWwindow* s_Window;

    // Current and previous frame key states
    static bool s_KeysCurrent[GLFW_KEY_LAST + 1];
    static bool s_KeysPrevious[GLFW_KEY_LAST + 1];

    // Current and previous mouse button states
    static bool s_MouseCurrent[GLFW_MOUSE_BUTTON_LAST + 1];
    static bool s_MousePrevious[GLFW_MOUSE_BUTTON_LAST + 1];

    static glm::vec2 s_MousePos;
    static glm::vec2 s_MousePosPrev;
    static float     s_ScrollDelta;
    static float     s_ScrollDeltaThisFrame;
};