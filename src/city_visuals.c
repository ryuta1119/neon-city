#include "city_visuals.h"

#include "raylib.h"
#include <math.h>
#include <stdint.h>

static uint32_t TileHash(int x, int y, uint32_t salt)
{
    uint32_t h = (uint32_t)x * 73856093u ^ (uint32_t)y * 19349663u ^ salt;
    h ^= h >> 13;
    h *= 1274126177u;
    return h ^ (h >> 16);
}

static Color RGB(int r, int g, int b)
{
    return (Color){(unsigned char)r, (unsigned char)g, (unsigned char)b, 255};
}

static void DrawSidewalk(int x, int y, uint32_t h, bool arena)
{
    Color base = arena ? RGB(27, 32, 44) : RGB(27, 37, 55);
    if ((h & 3u) == 0u) base = arena ? RGB(31, 38, 48) : RGB(30, 42, 62);
    DrawRectangle(x, y, TILE_SIZE, TILE_SIZE, base);
    DrawRectangleLines(x, y, TILE_SIZE, TILE_SIZE,
                       arena ? RGB(46, 51, 62) : RGB(45, 58, 77));
    DrawLine(x + 3, y + 3, x + 18, y + 3, Fade(WHITE, 0.04f));
    if (h % 13u == 0u) {
        DrawRectangle(x + 22, y + 20, 18, 12, RGB(16, 24, 34));
        DrawRectangleLines(x + 22, y + 20, 18, 12, RGB(65, 80, 95));
        DrawLine(x + 26, y + 24, x + 38, y + 24, RGB(65, 80, 95));
    }
    if (h % 11u == 0u) {
        DrawRectangle(x + 12, y + 46, 27, 2,
                      arena ? Fade(ORANGE, 0.18f) : Fade(SKYBLUE, 0.15f));
    }
}

static void DrawRoad(int x, int y, int cx, int cy, const TileMap *map, bool arena)
{
    DrawRectangle(x, y, TILE_SIZE, TILE_SIZE,
                  arena ? RGB(17, 23, 33) : RGB(16, 26, 41));
    Color seam = arena ? RGB(39, 45, 51) : RGB(33, 49, 67);
    DrawLine(x, y + TILE_SIZE - 1, x + TILE_SIZE, y + TILE_SIZE - 1, seam);
    bool horizontal = (cx > 0 && map->tiles[cy][cx - 1] == '=') &&
                      (cx + 1 < MAP_COLS && map->tiles[cy][cx + 1] == '=');
    bool vertical = (cy > 0 && map->tiles[cy - 1][cx] == '=') &&
                    (cy + 1 < MAP_ROWS && map->tiles[cy + 1][cx] == '=');
    // Keep markings visual-only; the actual driveable tiles never change.
    Color line = arena ? Fade(ORANGE, 0.45f) : Fade(RGB(235, 201, 116), 0.53f);
    if (horizontal && !vertical && cx % 2 == 0) {
        DrawRectangle(x + 17, y + 29, 30, 3, line);
    } else if (vertical && !horizontal && cy % 2 == 0) {
        DrawRectangle(x + 30, y + 15, 3, 34, line);
    } else if (horizontal && vertical) {
        DrawRectangleLines(x + 5, y + 5, 54, 54,
                           arena ? Fade(ORANGE, 0.10f) : Fade(SKYBLUE, 0.09f));
    }
    if ((TileHash(cx, cy, 9u) % 17u) == 0u)
        DrawLine(x + 4, y + 53, x + 34, y + 47, Fade(WHITE, 0.08f));
}

static void DrawBuilding(int x, int y, int cx, int cy, const TileMap *map,
                         uint32_t h, bool arena, float t)
{
    DrawRectangle(x, y, TILE_SIZE, TILE_SIZE,
                  arena ? RGB(34, 30, 40) : RGB(37, 32, 59));
    DrawRectangle(x + 3, y + 4, 58, 57,
                  arena ? RGB(41, 41, 49) : RGB(43, 44, 69));
    DrawLine(x + 4, y + 4, x + 59, y + 4,
             arena ? RGB(78, 71, 73) : RGB(74, 70, 105));
    DrawLine(x + 4, y + 4, x + 4, y + 60,
             arena ? RGB(81, 68, 76) : RGB(73, 72, 114));
    Color emissive = arena ? RGB(247, 132, 65) : RGB(54, 210, 245);
    Color emissive2 = arena ? RGB(250, 187, 95) : RGB(241, 72, 171);
    if ((h % 7u) == 0u) emissive = emissive2;
    // Facade windows / skylights, one tile never changes its geometry.
    for (int r = 0; r < 2; r++) {
        for (int c = 0; c < 2; c++) {
            uint32_t w = TileHash(cx * 2 + c, cy * 2 + r, 77u);
            int wx = x + 12 + c * 27;
            int wy = y + 12 + r * 25;
            DrawRectangle(wx, wy, 17, 13, RGB(13, 22, 39));
            if (w % 5u != 0u) {
                float shimmer = 0.43f + 0.10f * sinf(t * 1.6f + (float)(w % 11u));
                DrawRectangle(wx + 2, wy + 2, 13, 9, Fade(emissive, shimmer));
            }
        }
    }
    if ((h % 13u) == 0u) {
        DrawRectangle(x + 8, y + 39, 48, 4,
                      Fade(emissive2, 0.70f));
    }
    // Top row of building block: neon fascia. Not a collision object.
    if (cy > 0 && map->tiles[cy-1][cx] != 'B' && h % 3u == 0u) {
        DrawRectangle(x + 4, y + 48, 56, 14, RGB(12, 13, 28));
        DrawRectangleLines(x + 4, y + 48, 56, 14, emissive2);
        DrawText((h & 1u) ? "NOVA" : "ZONE", x + 9, y + 50, 10, WHITE);
    }
}

void CityDrawMap(const TileMap *map, bool arena, float timeSeconds)
{
    if (!map) return;
    DrawRectangle(0, 0, MAP_WORLD_WIDTH, MAP_WORLD_HEIGHT,
                  arena ? RGB(13, 17, 25) : RGB(12, 20, 35));
    for (int cy = 0; cy < MAP_ROWS; cy++) {
        for (int cx = 0; cx < MAP_COLS; cx++) {
            int x = cx * TILE_SIZE;
            int y = cy * TILE_SIZE;
            uint32_t h = TileHash(cx, cy, arena ? 777u : 119u);
            char tile = map->tiles[cy][cx];
            if (tile == 'B') {
                DrawBuilding(x, y, cx, cy, map, h, arena, timeSeconds);
            } else if (tile == '#') {
                DrawRectangle(x, y, TILE_SIZE, TILE_SIZE,
                              arena ? RGB(39, 36, 45) : RGB(25, 33, 58));
                DrawRectangle(x + 3, y + 3, 58, 58,
                              arena ? RGB(49, 42, 48) : RGB(30, 42, 70));
                Color edge = arena ? RGB(205, 102, 60) : RGB(91, 146, 197);
                DrawRectangle(x, y, TILE_SIZE, 3, Fade(edge, 0.57f));
                if (h % 3u == 0u)
                    DrawRectangle(x + 14, y + 25, 34, 3, Fade(edge, 0.45f));
            } else if (tile == '=') {
                DrawRoad(x, y, cx, cy, map, arena);
            } else {
                DrawSidewalk(x, y, h, arena);
            }
        }
    }
    // Pavement lighting around hub terminals. No extra obstacles.
    if (!arena) {
        Vector2 terminals[2] = {{928.0f, 544.0f}, {1184.0f, 544.0f}};
        Color colors[2] = {RGB(49, 218, 240), RGB(242, 71, 175)};
        for (int i = 0; i < 2; i++) {
            DrawCircleV(terminals[i], 68, Fade(colors[i], 0.075f));
            DrawCircleLines((int)terminals[i].x, (int)terminals[i].y,
                            53, Fade(colors[i], 0.50f));
            DrawCircleLines((int)terminals[i].x, (int)terminals[i].y,
                            47, Fade(colors[i], 0.30f));
        }
    } else {
        // Warning-marked arena spawn pad; does not alter collision.
        DrawCircleLines(1024, 672, 56, Fade(ORANGE, 0.37f));
        DrawCircleLines(1024, 672, 44, Fade(ORANGE, 0.27f));
        DrawText("SPAWN", 1004, 718, 13, Fade(WHITE, 0.40f));
    }
}

void CityDrawRain(float timeSeconds, bool arena)
{
    // Atmosphere: deterministic lightweight rain, rendered in world-space.
    for (int i = 0; i < 82; i++) {
        uint32_t h = TileHash(i, 70 + i * 7, 900u);
        float x = (float)(h % MAP_WORLD_WIDTH);
        float y = fmodf((float)((h >> 12) % MAP_WORLD_HEIGHT) +
                        timeSeconds * (arena ? 265.0f : 205.0f) + i * 19.0f,
                        (float)MAP_WORLD_HEIGHT);
        Color color = arena ? Fade(ORANGE, 0.12f) : Fade(SKYBLUE, 0.15f);
        DrawLine((int)x, (int)y, (int)x - 5, (int)y + 13, color);
    }
}
