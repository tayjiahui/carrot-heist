#include "Renderer/TilemapRenderer.h"
#include <json.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

bool TilemapRenderer::Load(const std::string& filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[TilemapRenderer] Failed to open: " << filepath << "\n";
        return false;
    }

    json j;
    try { file >> j; }
    catch (const json::parse_error& e) {
        std::cerr << "[TilemapRenderer] Parse error: " << e.what() << "\n";
        return false;
    }

    // Map dimensions
    m_Data.mapWidth = j["width"].get<int>();
    m_Data.mapHeight = j["height"].get<int>();
    m_Data.tileWidth = j["tilewidth"].get<int>();
    m_Data.tileHeight = j["tileheight"].get<int>();

    std::cout << "[TilemapRenderer] Map: " << m_Data.mapWidth
        << "x" << m_Data.mapHeight << " tiles, "
        << m_Data.tileWidth << "x" << m_Data.tileHeight
        << "px each\n";

    // Load tilesets
    for (auto& ts : j["tilesets"])
    {
        TilesetData tileset;
        tileset.firstGID = ts["firstgid"].get<int>();

        // Handle external tileset reference
        std::string tsSource = "";
        if (ts.contains("source")) {
            std::cerr << "[TilemapRenderer] External tileset not supported yet: "
                << ts["source"] << "\n";
            continue;
        }

        tileset.tileWidth = ts.value("tilewidth", m_Data.tileWidth);
        tileset.tileHeight = ts.value("tileheight", m_Data.tileHeight);
        tileset.columns = ts.value("columns", 0);
        tileset.tileCount = ts.value("tilecount", 0);

        // Build image path relative to Assets/
        if (ts.contains("image")) {
            std::string rawPath = ts["image"].get<std::string>();

            // Extract just the filename
            size_t lastSlash = rawPath.find_last_of("/\\");
            if (lastSlash != std::string::npos)
                rawPath = rawPath.substr(lastSlash + 1);

            // Always look in Assets/Textures/
            tileset.imagePath = "Assets/Textures/" + rawPath;
            tileset.texture = new Texture(tileset.imagePath);
        }


        m_Data.tilesets.push_back(tileset);
        std::cout << "[TilemapRenderer] Loaded tileset: "
            << tileset.imagePath << "\n";
    }

    // Load layers
    for (auto& layer : j["layers"])
    {
        std::string layerType = layer["type"].get<std::string>();

        if (layerType == "tilelayer")
        {
            TileLayerData tileLayer;
            tileLayer.name = layer["name"].get<std::string>();
            tileLayer.width = layer["width"].get<int>();
            tileLayer.height = layer["height"].get<int>();
            tileLayer.visible = layer.value("visible", true);

            // Read tile data
            for (auto& gid : layer["data"])
                tileLayer.tiles.push_back(gid.get<int>());

            m_Data.tileLayers.push_back(tileLayer);
            std::cout << "[TilemapRenderer] Loaded tile layer: "
                << tileLayer.name << "\n";
        }
        else if (layerType == "objectgroup")
        {
            // Load objects
            for (auto& obj : layer["objects"])
            {
                MapObject mapObj;
                mapObj.id = obj["id"].get<int>();
                mapObj.name = obj.value("name", "");
                mapObj.type = obj.value("class", obj.value("type", ""));
                mapObj.x = obj["x"].get<float>();
                mapObj.y = obj["y"].get<float>();
                mapObj.width = obj.value("width", 0.0f);
                mapObj.height = obj.value("height", 0.0f);
                mapObj.isPoint = obj.value("point", false);

                // Read custom properties
                if (obj.contains("properties")) {
                    for (auto& prop : obj["properties"]) {
                        std::string propName = prop["name"].get<std::string>();
                        // Convert value to string regardless of type
                        if (prop["type"] == "string")
                            mapObj.properties[propName] = prop["value"].get<std::string>();
                        else if (prop["type"] == "float")
                            mapObj.properties[propName] = std::to_string(prop["value"].get<float>());
                        else if (prop["type"] == "bool")
                            mapObj.properties[propName] = prop["value"].get<bool>() ? "true" : "false";
                        else if (prop["type"] == "int")
                            mapObj.properties[propName] = std::to_string(prop["value"].get<int>());
                    }
                }

                if (obj.contains("polyline"))
                {
                    int wpIndex = 0;
                    for (auto& point : obj["polyline"])
                    {
                        float px = point["x"].get<float>();
                        float py = point["y"].get<float>();
                        std::string key = "wp_" + std::to_string(wpIndex++);
                        mapObj.properties[key] = std::to_string(px) + "," + std::to_string(py);
                    }
                }

                m_Data.objects.push_back(mapObj);
            }
            std::cout << "[TilemapRenderer] Loaded object layer: "
                << layer["name"].get<std::string>() << "\n";
        }
    }

    m_Loaded = true;
    std::cout << "[TilemapRenderer] Loaded: " << filepath << "\n";
    return true;
}

void TilemapRenderer::Render(Renderer2D* renderer, float tileScale)
{
    if (!m_Loaded) return;

    for (auto& layer : m_Data.tileLayers)
    {
        if (!layer.visible) continue;
        if (layer.name == "Collision") continue; // skip collision layer

        RenderLayer(renderer, layer.name, tileScale);
    }
}

void TilemapRenderer::RenderLayer(Renderer2D* renderer,
    const std::string& layerName,
    float              tileScale)
{
    if (!m_Loaded) return;

    // Find the layer
    TileLayerData* layer = nullptr;
    for (auto& l : m_Data.tileLayers)
        if (l.name == layerName) { layer = &l; break; }

    if (!layer || !layer->visible) return;

    float tw = m_Data.tileWidth * tileScale;
    float th = m_Data.tileHeight * tileScale;

    for (int row = 0; row < layer->height; row++)
    {
        for (int col = 0; col < layer->width; col++)
        {
            int gid = layer->tiles[row * layer->width + col];
            if (gid == 0) continue; // empty tile

            const TilesetData* tileset = FindTileset(gid);
            if (!tileset || !tileset->texture) continue;

            glm::vec2 uvMin, uvMax;
            GetTileUV(gid, *tileset, uvMin, uvMax);

            // World position — centered on tile
            // Tiled Y is top-down, OpenGL Y is bottom-up so flip Y
            float worldX = col * tw + tw * 0.5f;
            float worldY = (m_Data.mapHeight - row - 1) * th + th * 0.5f;

            renderer->DrawFrame(
                { worldX, worldY },
                { tw, th },
                0.0f,
                tileset->texture->GetID(),
                uvMin,
                uvMax
            );
        }
    }
}

std::vector<MapObject> TilemapRenderer::GetObjectsByType(const std::string& type) const
{
    std::vector<MapObject> result;
    for (const auto& obj : m_Data.objects)
        if (obj.type == type)
            result.push_back(obj);
    return result;
}

bool TilemapRenderer::IsCollision(float worldX, float worldY, float tileScale) const
{
    if (!m_Loaded) return false;

    float tw = m_Data.tileWidth * tileScale;
    float th = m_Data.tileHeight * tileScale;

    int col = (int)(worldX / tw);
    int row = m_Data.mapHeight - 1 - (int)(worldY / th);

    // Check ALL collision layers
    for (const auto& layer : m_Data.tileLayers)
    {
        // Check any layer with "Collision" in the name
        if (layer.name.find("Collision") == std::string::npos &&
            layer.name.find("ollision") == std::string::npos) continue;

        if (col < 0 || col >= layer.width ||
            row < 0 || row >= layer.height)
            return true;

        int gid = layer.tiles[row * layer.width + col];
        if (gid != 0) return true;
    }

    return false;
}

void TilemapRenderer::GetTileUV(int gid, const TilesetData& tileset,
    glm::vec2& uvMin, glm::vec2& uvMax) const
{
    int localID = gid - tileset.firstGID;
    int col = localID % tileset.columns;
    int row = localID / tileset.columns;

    float texW = (float)(tileset.columns * tileset.tileWidth);
    float texH = (float)((tileset.tileCount / tileset.columns) * tileset.tileHeight);

    float uMin = (col * tileset.tileWidth) / texW;
    float vMin = (row * tileset.tileHeight) / texH;
    float uMax = uMin + tileset.tileWidth / texW;
    float vMax = vMin + tileset.tileHeight / texH;

    // Flip V for OpenGL
    uvMin = { uMin, 1.0f - vMax };
    uvMax = { uMax, 1.0f - vMin };
}

void TilemapRenderer::Unload()
{
    for (auto& ts : m_Data.tilesets)
        delete ts.texture;

    m_Data.tilesets.clear();
    m_Data.tileLayers.clear();
    m_Data.objects.clear();
    m_Loaded = false;
}

const TilesetData* TilemapRenderer::FindTileset(int gid) const
{
    const TilesetData* result = nullptr;
    for (const auto& ts : m_Data.tilesets)
        if (gid >= ts.firstGID)
            result = &ts;
    return result;
}