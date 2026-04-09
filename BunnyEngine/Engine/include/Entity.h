#pragma once
#include <string>
#include <glm/glm.hpp>

// -------------------------------------------------------
// Transform component — position, rotation, scale
// -------------------------------------------------------
struct Transform
{
    glm::vec2 position = { 0.0f, 0.0f };
    float     rotation = 0.0f;
    glm::vec2 scale = { 1.0f, 1.0f };
};

// -------------------------------------------------------
// Entity — a game object in the scene
// -------------------------------------------------------
struct Entity
{
    int         id;
    std::string name;
    bool        active = true;
    Transform   transform;

    Entity(int id, const std::string& name)
        : id(id), name(name) {
    }
};