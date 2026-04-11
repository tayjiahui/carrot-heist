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
#include "MainMenuScene.h"
#include "Scene/Scene.h"
#include "ECS/Entity.h"
#include "Core/Input.h"
#include "GameScene.h"
#include "Scene/Camera2D.h"
#include "Core/EditorInteraction.h"
#include "Core/SceneSerializer.h"
#include "Core/AudioManager.h"
#include "GuideBook.h"

// -------------------------------------------------------
// Globals
// -------------------------------------------------------
static int  g_WindowWidth = 1280;
static int  g_WindowHeight = 720;
static bool g_ViewportHovered = false;

static Framebuffer* g_Framebuffer = nullptr;
static Renderer2D* g_Renderer = nullptr;
static int g_EditorSelectedEntity = -1;

// Forward declarations
void RenderDockspace();
void RenderMenuBar();
void RenderViewportPanel();
void SetupDefaultLayout(ImGuiID dockspace_id);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

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
    Input::OnScroll((float)yoffset);
}

void SetupDefaultLayout(ImGuiID dockspace_id)
{
    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->WorkSize);

    ImGuiID dock_main = dockspace_id;
    ImGuiID dock_left = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Left, 0.20f, nullptr, &dock_main);
    ImGuiID dock_right = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Right, 0.25f, nullptr, &dock_main);
    ImGuiID dock_bottom = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Down, 0.25f, nullptr, &dock_main);

    ImGui::DockBuilderDockWindow("Viewport", dock_main);
    ImGui::DockBuilderDockWindow("Scene", dock_left);
    ImGui::DockBuilderDockWindow("Inspector", dock_right);
    ImGui::DockBuilderDockWindow("Console", dock_bottom);

    ImGui::DockBuilderFinish(dockspace_id);
}

void RenderMenuBar()
{
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene", "Ctrl+N")) {}
            if (ImGui::MenuItem("Open Scene", "Ctrl+O")) {
                IScene* s = SceneManager::Get().GetCurrentScene();
                if (s && s->GetScene())
                    SceneSerializer::Load(*s->GetScene(), "Assets/Scenes/level01.json");
            }
            if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
                IScene* s = SceneManager::Get().GetCurrentScene();
                if (s && s->GetScene())
                    SceneSerializer::Save(*s->GetScene(), "Assets/Scenes/level01.json");
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) { exit(0); }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Scene")) {
            if (ImGui::MenuItem("Main Menu"))
                SceneManager::Get().SwitchTo("MainMenu");
            if (ImGui::MenuItem("Game"))
                SceneManager::Get().SwitchTo("Game");
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void RenderViewportPanel()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport");

    g_ViewportHovered = ImGui::IsWindowHovered();

    ImVec2 viewportSize = ImGui::GetContentRegionAvail(); // declared here
    ImVec2 viewportPos = ImGui::GetCursorScreenPos();    // declared here

    if (viewportSize.x > 0 && viewportSize.y > 0)
        g_Framebuffer->Resize((int)viewportSize.x, (int)viewportSize.y);

    ImGui::Image(
        (ImTextureID)(uintptr_t)g_Framebuffer->GetTextureID(),
        viewportSize,
        ImVec2(0, 1),
        ImVec2(1, 0)
    );

    // Scene info + controls
    ImGui::SetCursorScreenPos(ImVec2(viewportPos.x + 8, viewportPos.y + 8));
    ImGui::TextColored(ImVec4(1, 1, 1, 0.5f),
        "Scene: %s  |  MMB: Pan  |  Scroll: Zoom  |  R: Reset",
        SceneManager::Get().GetCurrentSceneName().c_str());

    // Drag mode indicator
    if (EditorInteraction::IsDragModeActive()) {
        ImGui::SetCursorScreenPos(
            ImVec2(viewportPos.x + viewportSize.x - 130, viewportPos.y + 8));
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "[DRAG MODE ON]");
    }

    // Reset cursor and render scene ImGui
    ImGui::SetCursorScreenPos(viewportPos);
    SceneManager::Get().RenderImGui();

    ImGui::End();
    ImGui::PopStyleVar();
}

void RenderDockspace()
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags dockspace_flags =
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_MenuBar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("##DockspaceRoot", nullptr, dockspace_flags);
    ImGui::PopStyleVar(3);

    RenderMenuBar();

    ImGuiID dockspace_id = ImGui::GetID("MainDockspace");

    static bool layout_initialized = false;
    if (!layout_initialized) {
        SetupDefaultLayout(dockspace_id);
        layout_initialized = true;
    }

    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    ImGui::End();

    // Panels
    RenderViewportPanel();

    // Simple console
    ImGui::Begin("Console");
    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "[INFO]");
    ImGui::SameLine(); ImGui::Text("BunnyEngine running.");
    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "[INFO]");
    ImGui::SameLine(); ImGui::Text("Current scene: %s", SceneManager::Get().GetCurrentSceneName().c_str());

    ImGui::Separator();
    ImGui::Text("Audio");

    float masterVol = AudioManager::Get().GetMasterVolume();
    float bgmVol = AudioManager::Get().GetBGMVolume();
    float sfxVol = AudioManager::Get().GetSFXVolume();

    if (ImGui::SliderFloat("Master", &masterVol, 0.0f, 1.0f))
        AudioManager::Get().SetMasterVolume(masterVol);
    if (ImGui::SliderFloat("BGM", &bgmVol, 0.0f, 1.0f))
        AudioManager::Get().SetBGMVolume(bgmVol);
    if (ImGui::SliderFloat("SFX", &sfxVol, 0.0f, 1.0f))
        AudioManager::Get().SetSFXVolume(sfxVol);

    ImGui::End();

    // Scene panel
    ImGui::Begin("Scene");

    IScene* currentScene = SceneManager::Get().GetCurrentScene();
    if (currentScene) {
        ImGui::Text("Scene: %s", currentScene->GetSceneName().c_str());
        ImGui::Separator();

        auto* entities = currentScene->GetEntities();
        if (entities && !entities->empty()) {
            for (auto& entity : *entities) {
                bool isSelected = (g_EditorSelectedEntity == entity.id);

                if (isSelected)
                    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.50f, 0.25f, 1.0f));

                std::string label = (entity.active ? "  " : "  [hidden] ") + entity.name;
                if (ImGui::Selectable(label.c_str(), isSelected))
                    g_EditorSelectedEntity = entity.id;

                if (isSelected)
                    ImGui::PopStyleColor();
            }
        }
        else {
            ImGui::TextDisabled("No entities in this scene.");
        }
    }
    else {
        ImGui::TextDisabled("No active scene.");
    }

    ImGui::End();

    // Inspector panel
    ImGui::Begin("Inspector");

    IScene* inspScene = SceneManager::Get().GetCurrentScene();
    if (inspScene) {
        auto* entities = inspScene->GetEntities();
        if (entities) {
            Entity* selected = nullptr;
            for (auto& e : *entities) {
                if (e.id == g_EditorSelectedEntity) {
                    selected = &e;
                    break;
                }
            }

            if (selected) {
                char nameBuf[128];
                strncpy_s(nameBuf, selected->name.c_str(), sizeof(nameBuf));
                if (ImGui::InputText("##name", nameBuf, sizeof(nameBuf)))
                    selected->name = nameBuf;

                ImGui::SameLine();
                ImGui::Checkbox("Active", &selected->active);
                ImGui::Separator();

                if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::DragFloat2("Position", &selected->transform.position.x, 0.1f);
                    ImGui::DragFloat("Rotation", &selected->transform.rotation, 0.5f);
                    ImGui::DragFloat2("Scale", &selected->transform.scale.x, 0.1f);
                }
            }
            else {
                ImGui::TextDisabled("No entity selected.");
            }
        }
    }

    IScene* activeScene = SceneManager::Get().GetCurrentScene();
    if (activeScene) {
        Scene* scene = activeScene->GetScene();
        if (scene) {
            EditorInteraction::Update(
                scene,
                g_Framebuffer,
                g_EditorSelectedEntity,
                g_ViewportHovered
            );
        }
    }

    ImGui::End();
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
    GuideBook::Get().Init(
        "Assets/Textures/Close_button.png",
        "Assets/Textures/Carrot_Next.png",
        "Assets/Textures/Carrot_Prev.png",
        {
            "Assets/Textures/Bookpg1-2.png",
            "Assets/Textures/Bookpg3-4.png",
            "Assets/Textures/Bookpg5-6.png"
        }
    );
    std::cout << "[Main] Renderer OK\n";
    AudioManager::Get().Init();

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

    // Register scenes
    SceneManager::Get().RegisterScene("MainMenu", new MainMenuScene(g_Renderer, g_Framebuffer));
    SceneManager::Get().RegisterScene("Game", new GameScene(g_Renderer, g_Framebuffer));
    SceneManager::Get().SwitchTo("MainMenu");

    float lastTime = 0.0f;

    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        float currentTime = (float)glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glfwPollEvents();
        Input::Update(window);
        AudioManager::Get().Update();
        // DO NOT resize framebuffer here - viewport panel handles it

        // --- Render GL into framebuffer ---
        g_Framebuffer->Bind();
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClearColor(0.08f, 0.08f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        SceneManager::Get().Update(deltaTime);
        SceneManager::Get().RenderGL();

        g_Framebuffer->Unbind();

        // --- ImGui ---
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        RenderDockspace();

        ImGui::Render();
        glClearColor(0.08f, 0.08f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    AudioManager::Get().Shutdown();
    SceneManager::Get().Shutdown();
    GuideBook::Get().Shutdown();
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