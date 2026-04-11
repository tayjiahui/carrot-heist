#pragma once
#include "Renderer/Texture.h"
#include <vector>
#include <string>

class GuideBook
{
public:
    static GuideBook& Get() {
        static GuideBook instance;
        return instance;
    }

    void Init(const std::string& closeTexture,
        const std::string& nextTexture,
        const std::string& prevTexture,
        const std::vector<std::string>& pageTextures);
    void Shutdown();

    void Open() { m_Active = true; m_CurrentPage = 0; }
    void Close() { m_Active = false; }
    bool IsActive() const { return m_Active; }

    void NextPage();
    void PrevPage();

    void RenderImGui(float vpX, float vpY, float vpW, float vpH);

private:
    GuideBook() = default;

    Texture* m_CloseTexture = nullptr;
    Texture* m_NextTexture = nullptr;
    Texture* m_PrevTexture = nullptr;
    std::vector<Texture*> m_PageTextures;

    int  m_CurrentPage = 0;
    bool m_Active = false;
};