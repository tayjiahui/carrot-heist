#include "Core/AudioManager.h"
#include <fmod_errors.h>
#include <iostream>

// Helper macro for FMOD error checking
#define FMOD_CHECK(result) \
    if (result != FMOD_OK) { \
        std::cerr << "[AudioManager] FMOD Error: " \
                  << FMOD_ErrorString(result) << "\n"; \
    }

bool AudioManager::Init()
{
    FMOD_RESULT result;

    // Create FMOD system
    result = FMOD::System_Create(&m_System);
    FMOD_CHECK(result);
    if (result != FMOD_OK) return false;

    // Initialize with 512 channels
    result = m_System->init(512, FMOD_INIT_NORMAL, nullptr);
    FMOD_CHECK(result);
    if (result != FMOD_OK) return false;

    // Create channel groups
    m_System->createChannelGroup("BGM", &m_BGMGroup);
    m_System->createChannelGroup("SFX", &m_SFXGroup);

    // Set initial volumes
    m_BGMGroup->setVolume(m_BGMVolume);
    m_SFXGroup->setVolume(m_SFXVolume);

    std::cout << "[AudioManager] Initialized successfully\n";
    return true;
}

void AudioManager::Update()
{
    if (m_System)
        m_System->update();
}

void AudioManager::Shutdown()
{
    // Release all sounds
    for (auto& [name, sound] : m_Sounds)
        sound->release();
    m_Sounds.clear();

    if (m_BGMGroup) m_BGMGroup->release();
    if (m_SFXGroup) m_SFXGroup->release();
    if (m_System)   m_System->release();

    std::cout << "[AudioManager] Shutdown\n";
}

bool AudioManager::LoadSound(const std::string& name,
    const std::string& filepath,
    bool               loop)
{
    FMOD_MODE mode = FMOD_DEFAULT;
    if (loop) mode |= FMOD_LOOP_NORMAL;
    else      mode |= FMOD_LOOP_OFF;

    FMOD::Sound* sound = nullptr;
    FMOD_RESULT result = m_System->createSound(
        filepath.c_str(), mode, nullptr, &sound);

    if (result != FMOD_OK) {
        std::cerr << "[AudioManager] Failed to load: " << filepath
            << " — " << FMOD_ErrorString(result) << "\n";
        return false;
    }

    m_Sounds[name] = sound;
    std::cout << "[AudioManager] Loaded: " << name
        << " (" << filepath << ")\n";
    return true;
}

void AudioManager::Play(const std::string& name, float volume)
{
    auto it = m_Sounds.find(name);
    if (it == m_Sounds.end()) {
        std::cerr << "[AudioManager] Sound not found: " << name << "\n";
        return;
    }

    FMOD::Channel* channel = nullptr;
    m_System->playSound(it->second, nullptr, false, &channel);

    if (channel) {
        channel->setVolume(volume * m_MasterVolume);
        m_Channels[name] = channel;
    }
}

void AudioManager::Stop(const std::string& name)
{
    auto it = m_Channels.find(name);
    if (it != m_Channels.end())
        it->second->stop();
}

void AudioManager::Pause(const std::string& name)
{
    auto it = m_Channels.find(name);
    if (it != m_Channels.end())
        it->second->setPaused(true);
}

void AudioManager::Resume(const std::string& name)
{
    auto it = m_Channels.find(name);
    if (it != m_Channels.end())
        it->second->setPaused(false);
}

void AudioManager::PlayBGM(const std::string& name, float volume)
{
    // Stop current BGM first
    StopBGM();

    auto it = m_Sounds.find(name);
    if (it == m_Sounds.end()) {
        std::cerr << "[AudioManager] BGM not found: " << name << "\n";
        return;
    }

    m_System->playSound(it->second, m_BGMGroup, false, &m_BGMChannel);
    if (m_BGMChannel)
        m_BGMChannel->setVolume(volume * m_BGMVolume * m_MasterVolume);

    std::cout << "[AudioManager] Playing BGM: " << name << "\n";
}

void AudioManager::StopBGM()
{
    if (m_BGMChannel) {
        bool playing = false;
        m_BGMChannel->isPlaying(&playing);
        if (playing)
            m_BGMChannel->stop();
        m_BGMChannel = nullptr;
    }
}

void AudioManager::PlaySFX(const std::string& name, float volume)
{
    auto it = m_Sounds.find(name);
    if (it == m_Sounds.end()) {
        std::cerr << "[AudioManager] SFX not found: " << name << "\n";
        return;
    }

    FMOD::Channel* channel = nullptr;
    m_System->playSound(it->second, m_SFXGroup, false, &channel);
    if (channel)
        channel->setVolume(volume * m_SFXVolume * m_MasterVolume);
}

void AudioManager::SetMasterVolume(float volume)
{
    m_MasterVolume = volume;
    m_BGMGroup->setVolume(m_BGMVolume * m_MasterVolume);
    m_SFXGroup->setVolume(m_SFXVolume * m_MasterVolume);
}

void AudioManager::SetBGMVolume(float volume)
{
    m_BGMVolume = volume;
    m_BGMGroup->setVolume(m_BGMVolume * m_MasterVolume);
}

void AudioManager::SetSFXVolume(float volume)
{
    m_SFXVolume = volume;
    m_SFXGroup->setVolume(m_SFXVolume * m_MasterVolume);
}