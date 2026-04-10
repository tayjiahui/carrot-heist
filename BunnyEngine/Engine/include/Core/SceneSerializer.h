#pragma once
#include "Scene/Scene.h"
#include <string>

class SceneSerializer
{
public:
    // Save scene to JSON file
    static bool Save(Scene& scene, const std::string& filepath);

    // Load scene from JSON file into existing scene
    static bool Load(Scene& scene, const std::string& filepath);
};