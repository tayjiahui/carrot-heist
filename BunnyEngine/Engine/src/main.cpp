#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

#include "Renderer/Framebuffer.h"
#include "Renderer/Renderer2D.h"
#include "Scene/Camera2D.h"
#include "Core/SceneManager.h"
#include "Core/IScene.h"

// -------------------------------------------------------
// Globals
// -------------------------------------------------------
static int  g_WindowWidth = 1280;
static int  g_WindowHeight = 720;
static bool g_ViewportHovered = false;

static Framebuffer* g_Framebuffer = nullptr;
static Renderer2D* g_Renderer = nullptr;
static Camera2D     g_Camera;

// -------------------------------------------------------
// Temporary test scene
// -------------------------------------------------------
class TestScene : public IScene
{
public:
    TestScene(Renderer2D* renderer) : m_Renderer(renderer) {}

    void OnEnter() override {
        std::cout << "[TestScene] Entered!\n";
    }

    void OnUpdate(float deltaTime) override {
        m_Timer += deltaTime;
    }

    void OnRenderGL() override
    {
        // Only OpenGL draw calls here
        float aspect = 1280.0f / 720.0f;
        glm::mat4 proj = glm::ortho(
            -10.0f * aspect, 10.0f * aspect,
            -10.0f, 10.0f, -1.0f, 1.0f
        );
        m_Renderer->SetViewProjection(proj);

        float x = glm::sin(m_Timer) * 3.0f;
        m_Renderer->DrawQuad(
            { x, 0.0f },
            { 1.0f, 1.0f },
            m_Timer * 45.0f,
            { 1.0f, 0.55f, 0.10f, 1.0f }
        );
    }

    void OnRenderImGui() override
    {
        // Only ImGui calls here
        ImGui::SetNextWindowPos(ImVec2(10, 10));
        ImGui::SetNextWindowBgAlpha(0.5f);
        ImGui::Begin("##test", nullptr,
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "SceneManager Test");
        ImGui::Text("Current scene: %s",
            SceneManager::Get().GetCurrentSceneName().c_str());
        ImGui::Text("Timer: %.2f", m_Timer);
        ImGui::End();
    }

    void OnExit() override {
        std::cout << "[TestScene] Exited!\n";
    }

private:
    Renderer2D* m_Renderer = nullptr;
    float       m_Timer = 0.0f;
};

// -------------------------------------------------------
// GLFW callbacks
// -------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    g_WindowWidth = width;
    g_WindowHeight = height;
    glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (g_ViewportHovered)
        g_Camera.OnMouseZoom((float)yoffset);
}

// -------------------------------------------------------
// Main
// -------------------------------------------------------
int main()
{
    std::cout << "[Main] Starting...\n";

    if (!glfwInit()) { std::cerr << "Failed to init GLFW\n"; return -1; }
    std::cout << "[Main] GLFW init OK\n";

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(g_WindowWidth, g_WindowHeight,
        "Carrot Heist", nullptr, nullptr);
    if (!window) { std::cerr << "Failed to create window\n"; glfwTerminate(); return -1; }
    std::cout << "[Main] Window created OK\n";

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to init glad\n"; return -1;
    }
    std::cout << "[Main] Glad init OK\n";

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    std::cout << "[Main] Creating Framebuffer...\n";
    g_Framebuffer = new Framebuffer(1280, 720);
    std::cout << "[Main] Framebuffer OK\n";

    std::cout << "[Main] Creating Renderer...\n";
    g_Renderer = new Renderer2D();
    g_Renderer->Init();
    std::cout << "[Main] Renderer OK\n";

    // Init renderer and framebuffer
    g_Framebuffer = new Framebuffer(1280, 720);
    g_Renderer = new Renderer2D();
    g_Renderer->Init();

    // Init ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.TabRounding = 4.0f;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    // Register and start test scene
    SceneManager::Get().RegisterScene("Test", new TestScene(g_Renderer));
    SceneManager::Get().SwitchTo("Test");

    float lastTime = 0.0f;

    // Game loop
    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        float currentTime = (float)glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glfwPollEvents();

        // --- Render into framebuffer ---
        g_Framebuffer->Bind();
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClearColor(0.08f, 0.08f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Only update and render OpenGL here (no ImGui)
        SceneManager::Get().Update(deltaTime);
        SceneManager::Get().RenderGL();

        g_Framebuffer->Unbind();

        // --- ImGui ---
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();              // <-- ImGui frame starts HERE

        SceneManager::Get().RenderImGui();

        ImGui::Render();
        glClearColor(0.08f, 0.08f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    SceneManager::Get().Shutdown();
    g_Renderer->Shutdown();
    delete g_Renderer;
    delete g_Framebuffer;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}