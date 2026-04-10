#pragma once
#include "ECS/Entity.h"
#include <string>

enum class BunnyType
{
    Tofu,
    Toast
};

enum class AbilitySlot
{
    J = 0,
    K = 1,
    L = 2
};

struct AbilityCooldown
{
    float current = 0.0f;
    float max = 0.0f;
    bool  IsReady() const { return current <= 0.0f; }
};

class AbilitySystem
{
public:
    // Called every frame to tick cooldowns
    static void Update(float deltaTime);

    // Trigger an ability for a bunny
    static void TriggerAbility(BunnyType bunny, AbilitySlot slot, Entity* entity);

    // Get cooldown info for UI
    static AbilityCooldown GetCooldown(BunnyType bunny, AbilitySlot slot);

    // Get ability name for UI
    static std::string GetAbilityName(BunnyType bunny, AbilitySlot slot);

private:
    // Cooldowns[bunny][slot]
    static AbilityCooldown s_Cooldowns[2][3];

    // Tofu abilities
    static void Tofu_BinkyDash(Entity* entity);
    static void Tofu_KnockOver(Entity* entity);
    static void Tofu_Tumble(Entity* entity);

    // Toast abilities
    static void Toast_Thump(Entity* entity);
    static void Toast_Signal(Entity* entity);
    static void Toast_Dig(Entity* entity);
};