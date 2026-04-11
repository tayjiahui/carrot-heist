#pragma once
#include "ECS/Entity.h"
#include "Renderer/TilemapRenderer.h"
#include <glm/glm.hpp>
#include <vector>
#include <string>

enum class GuardState
{
    Patrol,      // walking patrol route normally
    Suspicious,  // saw something, slowing down, looking
    Alerted,     // spotted a bunny! chasing
    Searching    // lost the bunny, searching area
};

struct PatrolRoute
{
    std::string            name;
    std::vector<glm::vec2> waypoints;
};

struct GuardData
{
    int         entityID = -1;
    std::string guardName;
    GuardState  state = GuardState::Patrol;
    float       speed = 1.5f;
    float       visionRange = 3.0f;
    float       visionAngle = 90.0f; // degrees
    float       alertTimer = 0.0f;
    float       suspiciousTimer = 0.0f;
    float       facing = 0.0f;  // angle in degrees

    // Patrol
    std::string patrolRouteName;
    int         currentWaypoint = 0;
    bool        patrolForward = true;

    // Last known bunny position
    glm::vec2   lastKnownPos = { 0.0f, 0.0f };
    int         targetBunnyID = -1;
};

class GuardAISystem
{
public:
    // Initialize guards from tilemap objects
    static void Init(
        const std::vector<MapObject>& objects,
        float                         tileScale,
        int                           mapHeight,
        int                           tileHeight
    );

    // Update all guards
    static void Update(
        float                    deltaTime,
        std::vector<Entity>& entities,
        const std::vector<int>& bunnyIDs
    );

    // Render vision cones (debug)
    static void RenderVisionCones(
        class Renderer2D* renderer,
        std::vector<Entity>& entities
    );

    // Check if any bunny was caught
    static bool IsBunnyCaught() { return s_BunnyCaught; }
    static int  GetCaughtBunnyID() { return s_CaughtBunnyID; }
    static void ResetCaught() { s_BunnyCaught = false; s_CaughtBunnyID = -1; }

    static const std::vector<GuardData>& GetGuards() { return s_Guards; }

private:
    static void UpdateGuard(
        GuardData& guard,
        float                deltaTime,
        std::vector<Entity>& entities,
        const std::vector<int>& bunnyIDs
    );

    static bool CanSeeEntity(
        const GuardData& guard,
        const Entity& guardEntity,
        const Entity& target
    );

    static glm::vec2 GetWaypoint(const GuardData& guard);
    static Entity* FindEntity(std::vector<Entity>& entities, int id);

    static std::vector<GuardData>   s_Guards;
    static std::vector<PatrolRoute> s_PatrolRoutes;

    static bool s_BunnyCaught;
    static int  s_CaughtBunnyID;
};