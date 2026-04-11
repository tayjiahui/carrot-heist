#pragma once
#include "Renderer/Renderer2D.h"
#include "Renderer/Texture.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <unordered_map>

// Represents one tileset
struct TilesetData
{
    int         firstGID = 0;    // first global tile ID
    int         tileWidth = 16;
    int         tileHeight = 16;
    int         columns = 0;    // tiles per row
    int         tileCount = 0;
    std::string imagePath;
    Texture* texture = nullptr;
};

// Represents one tile layer
struct TileLayerData
{
    std::string        name;
    int                width = 0;
    int                height = 0;
    std::vector<int>   tiles; // tile GIDs, 0 = empty
    bool               visible = true;
};

// Represents one object
struct MapObject
{
    int         id = 0;
    std::string name;
    std::string type;    // "Player", "Guard", "Carrot" etc
    float       x = 0.0f;
    float       y = 0.0f;
    float       width = 0.0f;
    float       height = 0.0f;
    bool        isPoint = false;

    // Custom properties
    std::unordered_map<std::string, std::string> properties;
};

// Represents the whole map
struct TilemapData
{
    int                      mapWidth = 0;
    int                      mapHeight = 0;
    int                      tileWidth = 0;
    int                      tileHeight = 0;
    std::vector<TilesetData> tilesets;
    std::vector<TileLayerData> tileLayers;
    std::vector<MapObject>   objects;
};

class TilemapRenderer
{
public:
    // Load a .tmj file
    bool Load(const std::string& filepath);

    // Render all visible tile layers
    void Render(Renderer2D* renderer, float tileScale = 1.0f);

    // Render a specific layer by name
    void RenderLayer(Renderer2D* renderer,
        const std::string& layerName,
        float              tileScale = 1.0f);

    // Get objects for game logic
    const std::vector<MapObject>& GetObjects() const { return m_Data.objects; }

    // Get objects by type
    std::vector<MapObject> GetObjectsByType(const std::string& type) const;

    // Get map data
    const TilemapData& GetData() const { return m_Data; }

    // Check if a world position is on a collision tile
    bool IsCollision(float worldX, float worldY, float tileScale = 1.0f) const;

    void Unload();

private:
    // Get UV coords for a tile GID
    void GetTileUV(int gid, const TilesetData& tileset,
        glm::vec2& uvMin, glm::vec2& uvMax) const;

    // Find tileset for a given GID
    const TilesetData* FindTileset(int gid) const;

    TilemapData m_Data;
    bool        m_Loaded = false;
};