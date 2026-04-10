#include "Core/SceneSerializer.h"
#include <json.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

bool SceneSerializer::Save(Scene& scene, const std::string& filepath)
{
    json j;
    j["scene"] = scene.GetName();
    j["entities"] = json::array();

    for (const auto& entity : scene.GetEntities())
    {
        json e;
        e["id"] = entity.id;
        e["name"] = entity.name;
        e["active"] = entity.active;
        e["position"] = { entity.transform.position.x, entity.transform.position.y };
        e["rotation"] = entity.transform.rotation;
        e["scale"] = { entity.transform.scale.x, entity.transform.scale.y };
        j["entities"].push_back(e);
    }

    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[SceneSerializer] Failed to open file for writing: " << filepath << "\n";
        return false;
    }

    file << j.dump(4); // pretty print with 4 space indent
    file.close();

    std::cout << "[SceneSerializer] Saved scene to: " << filepath << "\n";
    return true;
}

bool SceneSerializer::Load(Scene& scene, const std::string& filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[SceneSerializer] Failed to open file: " << filepath << "\n";
        return false;
    }

    json j;
    try {
        file >> j;
    }
    catch (const json::parse_error& e) {
        std::cerr << "[SceneSerializer] Parse error: " << e.what() << "\n";
        return false;
    }

    // Clear existing entities
    scene.GetEntities().clear();

    for (auto& e : j["entities"])
    {
        auto& entity = scene.AddEntity(e["name"].get<std::string>());
        entity.active = e["active"].get<bool>();
        entity.transform.position.x = e["position"][0].get<float>();
        entity.transform.position.y = e["position"][1].get<float>();
        entity.transform.rotation = e["rotation"].get<float>();
        entity.transform.scale.x = e["scale"][0].get<float>();
        entity.transform.scale.y = e["scale"][1].get<float>();
    }

    std::cout << "[SceneSerializer] Loaded scene from: " << filepath << "\n";
    return true;
}