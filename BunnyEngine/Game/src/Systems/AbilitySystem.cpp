#include "Systems/AbilitySystem.h"
#include <iostream>

// Static member definitions
AbilityCooldown AbilitySystem::s_Cooldowns[2][3] = {};

void AbilitySystem::Update(float deltaTime)
{
    for (int b = 0; b < 2; b++)
        for (int s = 0; s < 3; s++)
            if (s_Cooldowns[b][s].current > 0.0f)
                s_Cooldowns[b][s].current -= deltaTime;
}

void AbilitySystem::TriggerAbility(BunnyType bunny, AbilitySlot slot, Entity* entity)
{
    int b = (int)bunny;
    int s = (int)slot;

    if (!s_Cooldowns[b][s].IsReady()) {
        std::cout << "[AbilitySystem] " << GetAbilityName(bunny, slot)
            << " on cooldown! (" << s_Cooldowns[b][s].current << "s)\n";
        return;
    }

    if (bunny == BunnyType::Tofu) {
        switch (slot) {
        case AbilitySlot::J: Tofu_BinkyDash(entity); break;
        case AbilitySlot::K: Tofu_KnockOver(entity); break;
        case AbilitySlot::L: Tofu_Tumble(entity);    break;
        }
    }
    else if (bunny == BunnyType::Toast) {
        switch (slot) {
        case AbilitySlot::J: Toast_Thump(entity);  break;
        case AbilitySlot::K: Toast_Signal(entity); break;
        case AbilitySlot::L: Toast_Dig(entity);    break;
        }
    }
}

AbilityCooldown AbilitySystem::GetCooldown(BunnyType bunny, AbilitySlot slot)
{
    return s_Cooldowns[(int)bunny][(int)slot];
}

std::string AbilitySystem::GetAbilityName(BunnyType bunny, AbilitySlot slot)
{
    if (bunny == BunnyType::Tofu) {
        switch (slot) {
        case AbilitySlot::J: return "Binky Dash";
        case AbilitySlot::K: return "Knock Over";
        case AbilitySlot::L: return "Tumble";
        }
    }
    else {
        switch (slot) {
        case AbilitySlot::J: return "Thump";
        case AbilitySlot::K: return "Signal";
        case AbilitySlot::L: return "Dig";
        }
    }
    return "Unknown";
}

// -------------------------------------------------------
// Tofu abilities
// -------------------------------------------------------
void AbilitySystem::Tofu_BinkyDash(Entity* entity)
{
    std::cout << "[Tofu] Binky Dash!\n";
    // TODO: apply zigzag velocity burst
    s_Cooldowns[(int)BunnyType::Tofu][(int)AbilitySlot::J] = { 3.0f, 3.0f };
}

void AbilitySystem::Tofu_KnockOver(Entity* entity)
{
    std::cout << "[Tofu] Knock Over!\n";
    // TODO: knock over nearest object
    s_Cooldowns[(int)BunnyType::Tofu][(int)AbilitySlot::K] = { 5.0f, 5.0f };
}

void AbilitySystem::Tofu_Tumble(Entity* entity)
{
    std::cout << "[Tofu] Tumble!\n";
    // TODO: roll through small gap
    s_Cooldowns[(int)BunnyType::Tofu][(int)AbilitySlot::L] = { 4.0f, 4.0f };
}

// -------------------------------------------------------
// Toast abilities
// -------------------------------------------------------
void AbilitySystem::Toast_Thump(Entity* entity)
{
    std::cout << "[Toast] Thump!\n";
    // TODO: stun nearby guards
    s_Cooldowns[(int)BunnyType::Toast][(int)AbilitySlot::J] = { 4.0f, 4.0f };
}

void AbilitySystem::Toast_Signal(Entity* entity)
{
    std::cout << "[Toast] Signal!\n";
    // TODO: mark a location for Tofu
    s_Cooldowns[(int)BunnyType::Toast][(int)AbilitySlot::K] = { 6.0f, 6.0f };
}

void AbilitySystem::Toast_Dig(Entity* entity)
{
    std::cout << "[Toast] Dig!\n";
    // TODO: create slow trap for guards
    s_Cooldowns[(int)BunnyType::Toast][(int)AbilitySlot::L] = { 5.0f, 5.0f };
}