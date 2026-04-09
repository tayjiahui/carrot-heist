#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>
#include <iostream>
#include "Framebuffer.h"
#include "Scene.h"
#include "Renderer2D.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Camera2D.h"
#include "SpriteSheet.h"

static int g_WindowWidth = 1280;
static int g_WindowHeight = 720;

// Global framebuffer
static Framebuffer* g_Framebuffer = nullptr;
static Scene* g_Scene = nullptr;
static int    g_SelectedEntity = -1; // id of selected entity, -1 = none
static Renderer2D* g_Renderer = nullptr;
static Camera2D g_Camera;
static bool     g_ViewportHovered = false; // only pan/zoom when mouse is over viewport
static SpriteSheet* g_BunnySheet = nullptr;
static float        g_AnimTimer = 0.0f;
static int          g_AnimFrame = 0;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    g_WindowWidth = width;
    g_WindowHeight = height;
    glViewport(0, 0, width, height);
}

// Forward declarations
void RenderDockspace();
void RenderMenuBar();
void RenderHierarchyPanel();
void RenderInspectorPanel();
void RenderAssetBrowserPanel();
void RenderViewportPanel();
void RenderConsolePanel();
void SetupDefaultLayout(ImGuiID dockspace_id);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

int main()
{
    if (!glfwInit()) { std::cerr << "Failed to init GLFW\n"; return -1; }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(g_WindowWidth, g_WindowHeight,
        "BunnyEngine - Carrot Heist", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to init glad\n"; return -1;
    }

    // Create framebuffer at a default size
    g_Framebuffer = new Framebuffer(1280, 720);
    g_Renderer = new Renderer2D();
    g_Renderer->Init();
    g_BunnySheet = new SpriteSheet("Assets/Textures/bunny_idle.png", 64, 64);
    g_BunnySheet = new SpriteSheet("Assets/Textures/bunny_idle.png", 64, 64);
    glfwSetScrollCallback(window, scroll_callback);

    // Create scene with starter entities
    g_Scene = new Scene("Level_01");
    g_Scene->AddEntity("Toast (Player)").transform.position = { 0.0f,  0.0f };
    g_Scene->AddEntity("Tofu (Player)").transform.position = { 1.0f,  0.0f };
    g_Scene->AddEntity("Toffee (Player)").transform.position = { -1.0f,  0.0f };
    g_Scene->AddEntity("Garden_Tilemap").transform.position = { 0.0f,  0.0f };
    g_Scene->AddEntity("Guard_01").transform.position = { 3.0f,  2.0f };
    g_Scene->AddEntity("Guard_02").transform.position = { -3.0f,  2.0f };
    g_Scene->AddEntity("Carrot_01").transform.position = { 0.0f,  3.0f };

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

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // --- Camera input (only when viewport is hovered) ---
        if (g_ViewportHovered)
        {
            // Middle mouse pan
            static double lastMouseX = 0, lastMouseY = 0;
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);

            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
            {
                float dx = (float)(mouseX - lastMouseX);
                float dy = (float)(mouseY - lastMouseY);
                g_Camera.OnMousePan(dx, dy);
            }

            lastMouseX = mouseX;
            lastMouseY = mouseY;
        }

        // --- Render scene into framebuffer ---
        g_Framebuffer->Bind();
        glEnable(GL_BLEND);                               
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClearColor(0.18f, 0.22f, 0.18f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update camera with current viewport size
        g_Camera.Update(
            (float)g_Framebuffer->GetWidth(),
            (float)g_Framebuffer->GetHeight()
        );
        g_Renderer->SetViewProjection(g_Camera.GetViewProjection());

        // Animate — flip frame every 0.4 seconds
        g_AnimTimer += 0.016f; // approx 60fps deltatime
        if (g_AnimTimer >= 0.4f) {
            g_AnimTimer = 0.0f;
            g_AnimFrame = (g_AnimFrame + 1) % 2; // 2 idle frames
        }

        // Draw each entity as a colored quad
        for (auto& entity : g_Scene->GetEntities())
        {
            if (!entity.active) continue;

            glm::vec4 tint = glm::vec4(1.0f);

            // Highlight selected
            if (entity.id == g_SelectedEntity)
                tint = glm::vec4(1.4f, 1.4f, 0.8f, 1.0f);

            bool isBunny = entity.name.find("Toast") != std::string::npos ||
                entity.name.find("Tofu") != std::string::npos ||
                entity.name.find("Toffee") != std::string::npos;

            if (isBunny && g_BunnySheet)
            {
                // Draw animated bunny sprite
                SpriteFrame frame = g_BunnySheet->GetFrameByIndex(g_AnimFrame);
                g_Renderer->DrawFrame(
                    entity.transform.position,
                    entity.transform.scale * 2.0f, // scale up so 64px looks good
                    entity.transform.rotation,
                    g_BunnySheet->GetTexture().GetID(),
                    frame.uvMin,
                    frame.uvMax,
                    tint
                );
            }
            else
            {
                // Non-bunny entities stay as colored quads
                glm::vec4 color = { 0.8f, 0.8f, 0.8f, 1.0f };
                if (entity.name.find("Guard") != std::string::npos) color = { 0.80f, 0.20f, 0.20f, 1.0f };
                if (entity.name.find("Carrot") != std::string::npos) color = { 1.00f, 0.55f, 0.10f, 1.0f };
                if (entity.name.find("Tilemap") != std::string::npos) color = { 0.25f, 0.45f, 0.25f, 1.0f };

                if (entity.id == g_SelectedEntity)
                    color = glm::vec4(glm::vec3(color) * 1.4f, 1.0f);

                g_Renderer->DrawQuad(
                    entity.transform.position,
                    entity.transform.scale,
                    entity.transform.rotation,
                    color
                );
            }
        }

        g_Framebuffer->Unbind();

        // --- Render ImGui editor ---
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

    g_Renderer->Shutdown();
    delete g_Renderer;
    delete g_Scene;
    delete g_Framebuffer;
    delete g_BunnySheet;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
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

    ImGui::DockBuilderDockWindow("Hierarchy", dock_left);
    ImGui::DockBuilderDockWindow("Viewport", dock_main);
    ImGui::DockBuilderDockWindow("Inspector", dock_right);
    ImGui::DockBuilderDockWindow("Asset Browser", dock_bottom);
    ImGui::DockBuilderDockWindow("Console", dock_bottom);

    ImGui::DockBuilderFinish(dockspace_id);
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

    RenderHierarchyPanel();
    RenderViewportPanel();
    RenderInspectorPanel();
    RenderAssetBrowserPanel();
    RenderConsolePanel();
}

void RenderMenuBar()
{
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene", "Ctrl+N")) {}
            if (ImGui::MenuItem("Open Scene", "Ctrl+O")) {}
            if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) { exit(0); }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Hierarchy")) {}
            if (ImGui::MenuItem("Inspector")) {}
            if (ImGui::MenuItem("Asset Browser")) {}
            if (ImGui::MenuItem("Console")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About BunnyEngine")) {}
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void RenderHierarchyPanel()
{
    ImGui::Begin("Hierarchy");

    ImGui::Text("Scene: %s", g_Scene->GetName().c_str());
    ImGui::Separator();

    // List all entities
    for (auto& entity : g_Scene->GetEntities())
    {
        bool isSelected = (g_SelectedEntity == entity.id);

        // Highlight selected entity
        if (isSelected)
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.50f, 0.25f, 1.0f));

        std::string label = (entity.active ? "  " : "  [hidden] ") + entity.name;

        if (ImGui::Selectable(label.c_str(), isSelected))
            g_SelectedEntity = entity.id;

        if (isSelected)
            ImGui::PopStyleColor();

        // Right-click context menu per entity
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Duplicate")) {
                std::string newName = entity.name + "_copy";
                g_Scene->AddEntity(newName).transform = entity.transform;
            }
            if (ImGui::MenuItem("Delete")) {
                if (g_SelectedEntity == entity.id)
                    g_SelectedEntity = -1;
                g_Scene->DeleteEntity(entity.id);
                ImGui::EndPopup();
                break; // stop iterating — vector changed
            }
            ImGui::EndPopup();
        }
    }

    ImGui::Separator();

    // Add new entity button
    if (ImGui::Button("+ Add Entity", ImVec2(-1, 0)))
    {
        auto& e = g_Scene->AddEntity("New Entity");
        g_SelectedEntity = e.id;
    }

    ImGui::End();
}

void RenderInspectorPanel()
{
    ImGui::Begin("Inspector");

    // Find selected entity
    Entity* selected = nullptr;
    for (auto& e : g_Scene->GetEntities()) {
        if (e.id == g_SelectedEntity) {
            selected = &e;
            break;
        }
    }

    if (!selected) {
        ImGui::TextDisabled("No entity selected.");
        ImGui::End();
        return;
    }

    // Entity header
    char nameBuf[128];
    strncpy_s(nameBuf, selected->name.c_str(), sizeof(nameBuf));
    if (ImGui::InputText("##name", nameBuf, sizeof(nameBuf)))
        selected->name = nameBuf;

    ImGui::SameLine();
    ImGui::Checkbox("Active", &selected->active);

    ImGui::Separator();
    ImGui::Spacing();

    // Transform component
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::DragFloat2("Position", &selected->transform.position.x, 0.1f);
        ImGui::DragFloat("Rotation", &selected->transform.rotation, 0.5f);
        ImGui::DragFloat2("Scale", &selected->transform.scale.x, 0.1f);
    }

    ImGui::Spacing();

    // Placeholder components
    if (ImGui::CollapsingHeader("Sprite Renderer"))
    {
        ImGui::Button("Select Sprite...", ImVec2(-1, 0));
        static float tint[4] = { 1.f, 1.f, 1.f, 1.f };
        ImGui::ColorEdit4("Tint", tint);
    }

    if (ImGui::CollapsingHeader("Collider"))
    {
        static float size[2] = { 1.f, 1.f };
        ImGui::DragFloat2("Box Size", size, 0.1f);
    }

    ImGui::End();
}

void RenderAssetBrowserPanel()
{
    ImGui::Begin("Asset Browser");
    ImGui::Text("Assets/");
    ImGui::Separator();

    const char* folders[] = { "Textures", "Scenes", "Audio", "Prefabs" };
    for (auto& f : folders) {
        if (ImGui::TreeNode(f)) {
            ImGui::Text("(empty)");
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void RenderViewportPanel()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport");

    // Track if mouse is over viewport
    g_ViewportHovered = ImGui::IsWindowHovered();

    ImVec2 viewportSize = ImGui::GetContentRegionAvail();

    if (viewportSize.x > 0 && viewportSize.y > 0)
        g_Framebuffer->Resize((int)viewportSize.x, (int)viewportSize.y);

    ImGui::Image(
        (ImTextureID)(uintptr_t)g_Framebuffer->GetTextureID(),
        viewportSize,
        ImVec2(0, 1),
        ImVec2(1, 0)
    );

    // Camera info overlay
    ImGui::SetCursorPos(ImVec2(8, 28));
    ImGui::TextColored(ImVec4(1, 1, 1, 0.5f),
        "Pos: (%.1f, %.1f)  Zoom: %.2fx  [MMB drag: pan | Scroll: zoom | R: reset]",
        g_Camera.GetPosition().x,
        g_Camera.GetPosition().y,
        g_Camera.GetZoom()
    );

    // Reset camera on R key when viewport hovered
    if (g_ViewportHovered && ImGui::IsKeyPressed(ImGuiKey_R))
        g_Camera.Reset();

    ImGui::End();
    ImGui::PopStyleVar();
}

void RenderConsolePanel()
{
    ImGui::Begin("Console");
    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "[INFO]");
    ImGui::SameLine(); ImGui::Text("BunnyEngine initialized.");
    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "[INFO]");
    ImGui::SameLine(); ImGui::Text("Framebuffer ready. Viewport is live!");
    ImGui::End();
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (g_ViewportHovered)
        g_Camera.OnMouseZoom((float)yoffset);
}