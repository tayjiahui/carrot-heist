#pragma once
#include <fmod.hpp>
#include <string>
#include <unordered_map>

class AudioManager
{
public:
    // Singleton
    static AudioManager& Get() {
        static AudioManager instance;
        return instance;
    }

    bool Init();
    void Update();
    void Shutdown();

    // Load a sound file
    bool LoadSound(const std::string& name,
        const std::string& filepath,
        bool               loop = false);

    // Play a loaded sound
    void Play(const std::string& name, float volume = 1.0f);
    void Stop(const std::string& name);
    void Pause(const std::string& name);
    void Resume(const std::string& name);

    // Global volume
    void SetMasterVolume(float volume);
    void SetBGMVolume(float volume);
    void SetSFXVolume(float volume);

    // Play as BGM (looping background music)
    void PlayBGM(const std::string& name, float volume = 1.0f);
    void StopBGM();

    // Play as SFX (one shot)
    void PlaySFX(const std::string& name, float volume = 1.0f);

    float GetMasterVolume() const { return m_MasterVolume; }
    float GetBGMVolume()    const { return m_BGMVolume; }
    float GetSFXVolume()    const { return m_SFXVolume; }

private:
    AudioManager() = default;

    FMOD::System* m_System = nullptr;

    // Sound storage
    std::unordered_map<std::string, FMOD::Sound*>   m_Sounds;
    std::unordered_map<std::string, FMOD::Channel*> m_Channels;

    // BGM channel
    FMOD::Channel* m_BGMChannel = nullptr;

    // Channel groups
    FMOD::ChannelGroup* m_BGMGroup = nullptr;
    FMOD::ChannelGroup* m_SFXGroup = nullptr;

    float m_MasterVolume = 1.0f;
    float m_BGMVolume = 1.0f;
    float m_SFXVolume = 1.0f;
};