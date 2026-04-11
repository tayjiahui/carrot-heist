#include "Systems/GuardAI.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cmath>

// Static definitions
std::vector<GuardData>   GuardAISystem::s_Guards;
std::vector<PatrolRoute> GuardAISystem::s_PatrolRoutes;
bool GuardAISystem::s_BunnyCaught = false;
int  GuardAISystem::s_CaughtBunnyID = -1;

static const float PI = 3.14159265f;

void GuardAISystem::Init(
    const std::vector<MapObject>& objects,
    float                         tileScale,
    int                           mapHeight,
    int                           tileHeight)
{
    s_Guards.clear();
    s_PatrolRoutes.clear();

    // First pass — load patrol routes from polylines
    for (const auto& obj : objects)
    {
        if (obj.type != "PatrolRoute") continue;

        PatrolRoute route;
        route.name = obj.name;

        // Read waypoints stored as wp_0, wp_1 etc
        int i = 0;
        while (true)
        {
            std::string key = "wp_" + std::to_string(i);
            if (!obj.properties.count(key)) break;

            std::string val = obj.properties.at(key);
            size_t comma = val.find(',');
            if (comma == std::string::npos) break;

            float px = std::stof(val.substr(0, comma));
            float py = std::stof(val.substr(comma + 1));

            // Waypoints are relative to polyline origin in Tiled
            float wx = (obj.x + px) * tileScale;
            float wy = (mapHeight * tileHeight - (obj.y + py)) * tileScale;
            route.waypoints.push_back({ wx, wy });
            i++;
        }

        // Fallback — use object position as single waypoint
        if (route.waypoints.empty())
        {
            float wx = obj.x * tileScale;
            float wy = (mapHeight * tileHeight - obj.y) * tileScale;
            route.waypoints.push_back({ wx, wy });
        }

        s_PatrolRoutes.push_back(route);
        std::cout << "[GuardAI] Route: " << route.name
            << " with " << route.waypoints.size() << " waypoints\n";
    }

    // Second pass — load guards
    for (const auto& obj : objects)
    {
        if (obj.type != "Guard") continue;

        GuardData guard;
        guard.entityID = -1;
        guard.guardName = obj.name; // store name for entity matching

        // Read custom properties
        if (obj.properties.count("vision_range"))
            guard.visionRange = std::stof(obj.properties.at("vision_range"));
        if (obj.properties.count("vision_angle"))
            guard.visionAngle = std::stof(obj.properties.at("vision_angle"));
        if (obj.properties.count("speed"))
            guard.speed = std::stof(obj.properties.at("speed"));
        if (obj.properties.count("patrol_route"))
            guard.patrolRouteName = obj.properties.at("patrol_route");

        s_Guards.push_back(guard);
        std::cout << "[GuardAI] Guard: " << guard.guardName
            << " patrol: " << guard.patrolRouteName << "\n";
    }
}

void GuardAISystem::Update(
    float                    deltaTime,
    std::vector<Entity>& entities,
    const std::vector<int>& bunnyIDs)
{
    for (auto& guard : s_Guards)
    {
        // Match guard to entity by name
        if (guard.entityID == -1)
        {
            for (auto& e : entities)
            {
                if (e.name == guard.guardName)
                {
                    guard.entityID = e.id;
                    std::cout << "[GuardAI] Matched guard "
                        << guard.guardName << " to entity ID "
                        << guard.entityID << "\n";
                    break;
                }
            }
        }

        if (guard.entityID != -1)
            UpdateGuard(guard, deltaTime, entities, bunnyIDs);
    }
}

void GuardAISystem::UpdateGuard(
    GuardData& guard,
    float                   deltaTime,
    std::vector<Entity>& entities,
    const std::vector<int>& bunnyIDs)
{
    Entity* guardEntity = FindEntity(entities, guard.entityID);
    if (!guardEntity) return;

    // Check vision for all bunnies
    int spottedBunnyID = -1;
    for (int bunnyID : bunnyIDs)
    {
        Entity* bunny = FindEntity(entities, bunnyID);
        if (!bunny || !bunny->active) continue;

        if (CanSeeEntity(guard, *guardEntity, *bunny))
        {
            spottedBunnyID = bunnyID;
            guard.lastKnownPos = bunny->transform.position;
            break;
        }
    }

    switch (guard.state)
    {
    case GuardState::Patrol:
    {
        glm::vec2 waypoint = GetWaypoint(guard);
        glm::vec2 dir = waypoint - guardEntity->transform.position;
        float     dist = glm::length(dir);

        if (dist > 0.15f)
        {
            dir = glm::normalize(dir);
            guardEntity->transform.position += dir * guard.speed * deltaTime;
            guard.facing = glm::degrees(std::atan2(dir.y, dir.x));
        }
        else
        {
            // Reached waypoint — advance to next
            for (auto& route : s_PatrolRoutes)
            {
                if (route.name == guard.patrolRouteName)
                {
                    int total = (int)route.waypoints.size();

                    if (guard.patrolForward)
                    {
                        guard.currentWaypoint++;
                        if (guard.currentWaypoint >= total)
                        {
                            guard.currentWaypoint = total - 2; // step back
                            guard.patrolForward = false;
                        }
                    }
                    else
                    {
                        guard.currentWaypoint--;
                        if (guard.currentWaypoint < 0)
                        {
                            guard.currentWaypoint = 1; // step forward
                            guard.patrolForward = true;
                        }
                    }

                    std::cout << "[GuardAI] Waypoint -> "
                        << guard.currentWaypoint
                        << (guard.patrolForward ? " (fwd)" : " (bwd)") << "\n";
                    break;
                }
            }
        }

        // Transition to suspicious
        if (spottedBunnyID != -1)
        {
            guard.state = GuardState::Suspicious;
            guard.suspiciousTimer = 1.5f;
            guard.targetBunnyID = spottedBunnyID;
            std::cout << "[GuardAI] Guard suspicious!\n";
        }
        break;
    }

    case GuardState::Suspicious:
    {
        guard.suspiciousTimer -= deltaTime;

        // If bunny hid — go back to patrol immediately
        bool targetVisible = false;
        for (int id : bunnyIDs)
            if (id == guard.targetBunnyID) { targetVisible = true; break; }

        if (!targetVisible)
        {
            guard.state = GuardState::Patrol;
            std::cout << "[GuardAI] Bunny hid! Guard back to patrol.\n";
            break;
        }

        glm::vec2 dir = guard.lastKnownPos - guardEntity->transform.position;
        if (glm::length(dir) > 0.01f)
            guard.facing = glm::degrees(std::atan2(dir.y, dir.x));

        if (spottedBunnyID != -1)
        {
            guard.state = GuardState::Alerted;
            guard.alertTimer = 5.0f;
            guard.targetBunnyID = spottedBunnyID;
            std::cout << "[GuardAI] Guard ALERTED!\n";
        }
        else if (guard.suspiciousTimer <= 0.0f)
        {
            guard.state = GuardState::Patrol;
            std::cout << "[GuardAI] Guard back to patrol\n";
        }
        break;
    }

    case GuardState::Alerted:
    {
        Entity* target = FindEntity(entities, guard.targetBunnyID);

        // Check if target bunny is still visible (not hidden)
        bool targetVisible = false;
        for (int id : bunnyIDs)
            if (id == guard.targetBunnyID) { targetVisible = true; break; }

        // If bunny hid — guard immediately loses them, no matter how close
        if (!targetVisible)
        {
            guard.state = GuardState::Searching;
            guard.alertTimer = 3.0f;
            guard.targetBunnyID = -1;
            std::cout << "[GuardAI] Bunny hid! Guard lost target.\n";
            break;
        }

        // Update last known position
        if (target && target->active)
            guard.lastKnownPos = target->transform.position;

        // Chase
        glm::vec2 dir = guard.lastKnownPos - guardEntity->transform.position;
        float     dist = glm::length(dir);

        if (dist > 0.01f)
        {
            dir = glm::normalize(dir);
            guardEntity->transform.position += dir * guard.speed * 1.8f * deltaTime;
            guard.facing = glm::degrees(std::atan2(dir.y, dir.x));
        }

        // Catch — only when visible
        if (dist < 0.5f)
        {
            s_BunnyCaught = true;
            s_CaughtBunnyID = guard.targetBunnyID;
            std::cout << "[GuardAI] Bunny caught!\n";
        }

        // Lost sight timer
        if (spottedBunnyID == -1)
        {
            guard.alertTimer -= deltaTime;
            if (guard.alertTimer <= 0.0f)
            {
                guard.state = GuardState::Searching;
                std::cout << "[GuardAI] Guard searching...\n";
            }
        }
        else
            guard.alertTimer = 5.0f;

        break;
    }

    case GuardState::Searching:
    {
        // Move to last known position
        glm::vec2 dir = guard.lastKnownPos - guardEntity->transform.position;
        float dist = glm::length(dir);

        if (dist > 0.1f)
        {
            dir = glm::normalize(dir);
            guardEntity->transform.position += dir * guard.speed * deltaTime;
            guard.facing = glm::degrees(std::atan2(dir.y, dir.x));
        }
        else
        {
            // Reached last known pos, go back to patrol
            guard.state = GuardState::Patrol;
            std::cout << "[GuardAI] Guard resuming patrol\n";
        }

        if (spottedBunnyID != -1)
        {
            guard.state = GuardState::Alerted;
            guard.alertTimer = 5.0f;
        }
        break;
    }
    }
}

bool GuardAISystem::CanSeeEntity(
    const GuardData& guard,
    const Entity& guardEntity,
    const Entity& target)
{
    glm::vec2 toTarget = target.transform.position
        - guardEntity.transform.position;
    float dist = glm::length(toTarget);

    // Range check
    if (dist > guard.visionRange) return false;

    // Angle check
    float angleToTarget = glm::degrees(std::atan2(toTarget.y, toTarget.x));
    float angleDiff = angleToTarget - guard.facing;

    // Normalize angle to -180 to 180
    while (angleDiff > 180.0f) angleDiff -= 360.0f;
    while (angleDiff < -180.0f) angleDiff += 360.0f;

    return std::abs(angleDiff) <= guard.visionAngle * 0.5f;
}

glm::vec2 GuardAISystem::GetWaypoint(const GuardData& guard)
{
    for (const auto& route : s_PatrolRoutes)
    {
        if (route.name == guard.patrolRouteName && !route.waypoints.empty())
        {
            int idx = guard.currentWaypoint % (int)route.waypoints.size();
            return route.waypoints[idx];
        }
    }

    // No route found — stay in place
    return { 0.0f, 0.0f };
}

Entity* GuardAISystem::FindEntity(std::vector<Entity>& entities, int id)
{
    for (auto& e : entities)
        if (e.id == id) return &e;
    return nullptr;
}

void GuardAISystem::RenderVisionCones(
    Renderer2D* renderer,
    std::vector<Entity>& entities)
{
    for (const auto& guard : s_Guards)
    {
        Entity* e = FindEntity(entities, guard.entityID);
        if (!e) continue;

        // Color based on state
        glm::vec4 color = { 1.0f, 1.0f, 0.0f, 0.3f }; // yellow = patrol
        if (guard.state == GuardState::Suspicious) color = { 1.0f, 0.5f, 0.0f, 0.4f }; // orange
        if (guard.state == GuardState::Alerted)    color = { 1.0f, 0.0f, 0.0f, 0.5f }; // red
        if (guard.state == GuardState::Searching)  color = { 0.5f, 0.5f, 1.0f, 0.4f }; // blue

        // Draw a simple quad as vision indicator for now
        renderer->DrawQuad(
            e->transform.position + glm::vec2(
                std::cos(glm::radians(guard.facing)) * guard.visionRange * 0.5f,
                std::sin(glm::radians(guard.facing)) * guard.visionRange * 0.5f),
            { guard.visionRange, guard.visionRange * 0.5f },
            guard.facing,
            color
        );
    }
}