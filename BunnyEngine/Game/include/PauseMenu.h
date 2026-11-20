#pragma once
#include <functional>

class Texture;

class PauseMenu
{
public:
    static PauseMenu& Get() {
        static PauseMenu instance;
        return instance;
    }

    void Toggle() { m_Active = !m_Active; }
    void Open() { m_Active = true; }
    void Close() { m_Active = false; }

    bool IsActive() const { return m_Active; }

    // Callbacks — set by each scene
    void SetOnResume(std::function<void()> fn) { m_OnResume = fn; }
    void SetOnMainMenu(std::function<void()> fn) { m_OnMainMenu = fn; }
    void SetOnGuide(std::function<void()> fn) { m_OnGuide = fn; }

    // Call every frame from scene's OnRenderImGui
    void RenderImGui(float vpX, float vpY, float vpW, float vpH);

private:
    PauseMenu() = default;

    bool m_Active = false;

    // Pixel-art pause menu textures, loaded on first draw (needs the GL context)
    void LoadTextures();
    bool     m_TexturesLoaded = false;
    Texture* m_Background = nullptr;
    Texture* m_BtnResume = nullptr;
    Texture* m_BtnGuide = nullptr;
    Texture* m_BtnMainMenu = nullptr;

    std::function<void()> m_OnResume;
    std::function<void()> m_OnMainMenu;
    std::function<void()> m_OnGuide;
};