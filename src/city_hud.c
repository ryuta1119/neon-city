#include "city_hud.h"
#include "raylib.h"
#include <stdio.h>

static const Color HUD_BG = {9, 16, 29, 225};
static const Color HUD_INNER = {21, 32, 50, 255};
static const Color HUD_BORDER = {59, 94, 119, 255};
static const Color HUD_TEAL = {64, 222, 226, 255};
static const Color HUD_PINK = {240, 98, 174, 255};
static const Color HUD_GOLD = {241, 206, 127, 255};
static const Color HUD_SOFT = {163, 184, 204, 255};

static void Panel(int x, int y, int w, int h, Color accent)
{
    DrawRectangle(x, y, w, h, HUD_BG);
    DrawRectangleLinesEx((Rectangle){(float)x, (float)y, (float)w, (float)h},
                         1.0f, HUD_BORDER);
    DrawRectangle(x, y, 4, h, accent);
    DrawRectangle(x + 13, y + 10, 24, 2, Fade(accent, 0.75f));
}

static void StatusBar(int x, int y, int w, int h,
                      float proportion, Color color)
{
    if (proportion < 0.0f) proportion = 0.0f;
    if (proportion > 1.0f) proportion = 1.0f;
    DrawRectangle(x, y, w, h, HUD_INNER);
    DrawRectangle(x, y, (int)(w * proportion), h, color);
    DrawRectangleLines(x, y, w, h, Fade(WHITE, 0.22f));
}

static void DrawMiniMap(const TileMap *map, Vector2 player,
                        int x, int y, bool arena)
{
    Panel(x, y, 177, 147, arena ? HUD_GOLD : HUD_TEAL);
    DrawText(arena ? "ARENA / RADAR" : "CENTRAL / RADAR",
             x + 18, y + 10, 14, HUD_SOFT);
    const int tile = 4;
    const int left = x + 24;
    const int top = y + 36;
    for (int row = 0; row < MAP_ROWS; row++) {
        for (int col = 0; col < MAP_COLS; col++) {
            char kind = map->tiles[row][col];
            Color c = (Color){27, 43, 63, 255};
            if (kind == '#') c = (Color){65, 79, 100, 255};
            if (kind == 'B') c = (Color){87, 67, 98, 255};
            if (kind == '=') c = (Color){36, 69, 82, 255};
            DrawRectangle(left + col * tile, top + row * tile,
                          tile - 1, tile - 1, c);
        }
    }
    if (!arena) {
        DrawCircle(left + (int)(928.0f / TILE_SIZE * tile),
                   top + (int)(544.0f / TILE_SIZE * tile), 2.6f, HUD_TEAL);
        DrawCircle(left + (int)(1184.0f / TILE_SIZE * tile),
                   top + (int)(544.0f / TILE_SIZE * tile), 2.6f, HUD_PINK);
    }
    DrawCircle(left + (int)(player.x / TILE_SIZE * tile),
               top + (int)(player.y / TILE_SIZE * tile), 3.4f, HUD_GOLD);
    DrawText("Uplink online", x + 25, y + 124, 12, HUD_SOFT);
}

void CityHUDDrawHub(const Economy *economy, const Player *player,
                    const TileMap *map, bool showSave, bool saveSuccess)
{
    Panel(16, 16, 337, 154, HUD_TEAL);
    DrawText("NEON CITY", 36, 27, 31, WHITE);
    DrawText("CENTRAL HUB  /  NIGHT DISTRICT", 36, 68, 16, HUD_TEAL);

    // Keep credits and contracts on separate rows; fit the full credit range.
    char creditsText[64];
    snprintf(creditsText, sizeof(creditsText), "CREDITS  %d CR", economy->credits);
    int creditsFontSize = 22;
    while (creditsFontSize > 14 && MeasureText(creditsText, creditsFontSize) > 297)
        creditsFontSize--;
    DrawText(creditsText, 36, 96, creditsFontSize, HUD_GOLD);
    DrawText(TextFormat("CONTRACTS %d", economy->completedMissions),
             36, 132, 16, HUD_SOFT);

    DrawMiniMap(map, player->position, 1085, 16, false);

    // Keep controls at bottom left; terminal prompts occupy bottom center.
    Panel(16, 643, 358, 59, HUD_TEAL);
    DrawText("WASD MOVE    E INTERACT", 33, 651, 17, WHITE);
    DrawText("P / F5 SAVE    ESC TITLE", 33, 677, 17, HUD_SOFT);
    if (showSave) {
        Panel(376, 17, 238, 44, saveSuccess ? GREEN : RED);
        DrawText(saveSuccess ? "SAVE COMPLETE" : "SAVE FAILED",
                 393, 29, 19, saveSuccess ? GREEN : RED);
    }
    DrawText("NEON CITY  //  BUILD 1.0", 939, 687, 12, HUD_SOFT);
}

static const char *MissionName(MissionType type)
{
    switch (type) {
        case MISSION_ELIMINATION: return "ELIMINATION";
        case MISSION_SURVIVAL: return "SURVIVAL";
        case MISSION_TIME_ATTACK: return "TIME ATTACK";
        default: return "CONTRACT";
    }
}

static const char *DifficultyName(MissionDifficulty difficulty)
{
    switch (difficulty) {
        case MISSION_EASY: return "EASY";
        case MISSION_NORMAL: return "NORMAL";
        case MISSION_HARD: return "HARD";
        default: return "UNKNOWN";
    }
}

void CityHUDDrawArena(const CombatSystem *combat, const MissionSystem *mission,
                      const Economy *economy, const Player *player,
                      const TileMap *map)
{
    Panel(16, 16, 345, 132, HUD_PINK);
    DrawText("OPERATOR / VITALS", 36, 31, 20, WHITE);
    DrawText(TextFormat("HP %d / %d", combat->playerHP, combat->playerMaxHP),
             37, 68, 19, HUD_SOFT);
    float hp = combat->playerMaxHP > 0
             ? (float)combat->playerHP / combat->playerMaxHP : 0.0f;
    StatusBar(146, 70, 189, 16, hp, hp <= 0.25f ? RED : HUD_PINK);
    DrawText(TextFormat("HOSTILES %d / %d",
             EnemiesRemaining(&combat->enemies), combat->enemies.count),
             37, 107, 19, HUD_TEAL);

    Panel(867, 16, 395, 157, HUD_GOLD);
    DrawText("ACTIVE CONTRACT", 887, 30, 16, HUD_SOFT);
    DrawText(MissionName(mission->current.type), 887, 53, 29, WHITE);
    DrawText(TextFormat("%s  //  REWARD %d CR",
             DifficultyName(mission->current.difficulty),
             mission->current.reward), 888, 93, 16, HUD_GOLD);
    if (mission->current.timeLimit > 0.0f) {
        float remaining = mission->current.timeLimit - mission->elapsed;
        if (remaining < 0.0f) remaining = 0.0f;
        DrawText(TextFormat("TIME LEFT  %05.1f", remaining),
                 888, 128, 20, remaining < 10.0f ? RED : HUD_TEAL);
    } else {
        DrawText(TextFormat("ELAPSED  %05.1f", mission->elapsed),
                 888, 128, 20, HUD_TEAL);
    }

    Panel(16, 629, 750, 73, HUD_TEAL);
    DrawText(TextFormat("PRIMARY  %s   /  DMG %d   /  %.2f SEC",
             EconomyGunName(economy), combat->gunDamage, combat->fireInterval),
             34, 644, 18, WHITE);
    DrawText(TextFormat("MELEE    %s   /  DMG %d",
             EconomyBladeName(economy), combat->meleeDamage),
             34, 671, 18, HUD_SOFT);
    DrawMiniMap(map, player->position, 1085, 546, true);
    DrawText("LMB SHOOT   RMB MELEE   SHIFT DASH   ESC HUB",
             770, 699, 13, HUD_SOFT);

    // Damage feedback that respects the existing invulnerability clock.
    if (combat->invulnerability > 0.0f &&
        ((int)(GetTime() * 12.0) % 2 == 0)) {
        DrawRectangle(0, 0, GetScreenWidth(), 7, Fade(RED, 0.45f));
        DrawRectangle(0, GetScreenHeight() - 7,
                      GetScreenWidth(), 7, Fade(RED, 0.45f));
    }
}

void CityHUDDrawCrosshair(void)
{
    Vector2 mouse = GetMousePosition();
    int x = (int)mouse.x;
    int y = (int)mouse.y;
    DrawCircleLines(x, y, 9, HUD_TEAL);
    DrawLine(x - 16, y, x - 5, y, WHITE);
    DrawLine(x + 5, y, x + 16, y, WHITE);
    DrawLine(x, y - 16, x, y - 5, WHITE);
    DrawLine(x, y + 5, x, y + 16, WHITE);
    DrawCircle(x, y, 2, HUD_GOLD);
}
