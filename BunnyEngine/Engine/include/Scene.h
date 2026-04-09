#pragma once
#include "ECS/Entity.h"
#include <vector>
#include <string>
#include <algorithm>

class Scene
{
public:
    Scene(const std::string& name = "Untitled")
        : m_Name(name) {
    }

    // Add a new entity and return a reference to it
    Entity& AddEntity(const std::string& name)
    {
        m_Entities.emplace_back(m_NextID++, name);
        return m_Entities.back();
    }

    // Delete entity by id
    void DeleteEntity(int id)
    {
        m_Entities.erase(
            std::remove_if(m_Entities.begin(), m_Entities.end(),
                [id](const Entity& e) { return e.id == id; }),
            m_Entities.end());
    }

    std::vector<Entity>& GetEntities() { return m_Entities; }
    const std::string& GetName() { return m_Name; }

private:
    std::string         m_Name;
    std::vector<Entity> m_Entities;
    int                 m_NextID = 0;
};