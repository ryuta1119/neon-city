#include "raylib.h"
#include "neon_menu.h"
#include "player.h"
#include "map.h"
#include "hub_terminal.h"
#include "combat.h"
#include "mission.h"
#include "economy.h"
#include "shop.h"
#include "savegame.h"
#include "city_visuals.h"
#include "city_hud.h"
#include <stdint.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define SAVE_PATH "save.dat"
#define HUB_MAP_PATH "data/central_hub.map"
#define ARENA_MAP_PATH "data/training_arena.map"

typedef enum { SCENE_MENU, SCENE_HUB, SCENE_ARENA } Scene;
static const Vector2 ARENA_SPAWN = {1024.0f, 672.0f};

static float ClampPosition(float value, float minimum, float maximum)
{
    if (value < minimum) return minimum;
    if (value > maximum) return maximum;
    return value;
}

static void UpdateWorldCamera(Camera2D *camera, Vector2 playerPosition)
{
    float halfWidth = SCREEN_WIDTH / (2.0f * camera->zoom);
    float halfHeight = SCREEN_HEIGHT / (2.0f * camera->zoom);
    camera->target.x = ClampPosition(playerPosition.x, halfWidth,
                                     MAP_WORLD_WIDTH - halfWidth);
    camera->target.y = ClampPosition(playerPosition.y, halfHeight,
                                     MAP_WORLD_HEIGHT - halfHeight);
}

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "NEON CITY");
    SetExitKey(KEY_NULL);
#ifdef __EMSCRIPTEN__
    SetTargetFPS(0); // Yield explicitly below; do not block the browser event loop.
    EM_ASM({
        try {
            var saved = localStorage.getItem('neon-city-save-v1');
            if (saved && saved.length < 512) FS.writeFile('save.dat', saved);
        } catch (error) { console.warn('Save restore unavailable', error); }
    });
#else
    SetTargetFPS(60);
#endif

    TileMap hubMap, arenaMap;
    if (!MapLoad(&hubMap, HUB_MAP_PATH) ||
        !MapLoad(&arenaMap, ARENA_MAP_PATH)) {
        TraceLog(LOG_ERROR, "Map loading failed");
        CloseWindow();
        return 1;
    }

    InitAudioDevice();
    Music bgm = LoadMusicStream("assets/audio/bass_over.wav");
    if (!IsMusicValid(bgm)) {
        TraceLog(LOG_ERROR, "BGM loading failed");
        CloseAudioDevice();
        CloseWindow();
        return 1;
    }
    bgm.looping = true;

    Player player;
    PlayerInit(&player);
    Vector2 hubReturnPosition = player.position;

    Economy economy;
    EconomyInit(&economy);
    Shop shop;
    ShopInit(&shop);
    bool lastAutosaveOk = true;

    HubTerminals hub;
    HubTerminalsInit(&hub);
    CombatSystem combat;
    CombatInit(&combat);
    MissionSystem mission;
    MissionInit(&mission);

    Menu menu;
    MenuInit(&menu, SaveGameExists(&hubMap, SAVE_PATH));
    SetMusicVolume(bgm, menu.musicVolume);
    PlayMusicStream(bgm);

    Camera2D camera = {
        .offset = {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f},
        .target = player.position,
        .rotation = 0.0f, .zoom = 1.0f
    };

    Scene scene = SCENE_MENU;
    bool saveSuccess = false;
    double saveMessageUntil = 0.0;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        if (dt > 0.05f) dt = 0.05f;

        if (scene == SCENE_MENU) {
            UpdateMusicStream(bgm);
            MenuAction action = MenuUpdate(&menu);
            SetMusicVolume(bgm, menu.musicVolume);

            if (action == MENU_NEW_GAME) {
                PlayerInit(&player);
                EconomyInit(&economy);
                ShopInit(&shop);
                HubTerminalsInit(&hub);
                MissionInit(&mission);
                saveMessageUntil = 0.0;
                lastAutosaveOk = true;
                StopMusicStream(bgm);
                scene = SCENE_HUB;
            } else if (action == MENU_CONTINUE) {
                if (SaveGameLoad(&player, &economy, &hubMap, SAVE_PATH)) {
                    ShopInit(&shop);
                    HubTerminalsInit(&hub);
                    MissionInit(&mission);
                    saveMessageUntil = 0.0;
                    lastAutosaveOk = true;
                    StopMusicStream(bgm);
                    scene = SCENE_HUB;
                } else {
                    menu.hasSave = false;
                }
            } else if (action == MENU_EXIT) {
                break;
            }
        } else if (scene == SCENE_HUB) {
            if (HubTerminalsIsOpen(&hub)) {
                if (hub.active == HUB_TERMINAL_MISSION) {
                    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_E)) {
                        HubTerminalsClose(&hub);
                    } else if (MissionSelectionUpdate(&mission)) {
                        hubReturnPosition = player.position;
                        player.position = ARENA_SPAWN;
                        CombatInit(&combat);
                        EconomyApplyToCombat(&economy, &combat);
                        MissionStart(&mission, &combat, &arenaMap, ARENA_SPAWN);
                        HubTerminalsClose(&hub);
                        lastAutosaveOk = true;
                        scene = SCENE_ARENA;
                    }
                } else if (hub.active == HUB_TERMINAL_SHOP) {
                    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_E)) {
                        HubTerminalsClose(&hub);
                    } else if (ShopUpdate(&shop, &economy)) {
                        lastAutosaveOk = SaveGameWrite(&player, &economy, SAVE_PATH);
                    }
                }
            } else if (IsKeyPressed(KEY_ESCAPE)) {
                scene = SCENE_MENU;
                menu.hasSave = SaveGameExists(&hubMap, SAVE_PATH);
                PlayMusicStream(bgm);
            } else {
                PlayerUpdate(&player, dt, &hubMap);
                if (IsKeyPressed(KEY_E)) {
                    HubTerminalsInteract(&hub, player.position);
                    if (HubTerminalsIsOpen(&hub)) {
                        if (hub.active == HUB_TERMINAL_MISSION) {
                            MissionGenerateOffers(&mission,
                                (uint32_t)GetRandomValue(1, 2147483647));
                        } else if (hub.active == HUB_TERMINAL_SHOP) {
                            ShopInit(&shop);
                        }
                    }
                }
                if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_F5)) {
                    saveSuccess = SaveGameWrite(&player, &economy, SAVE_PATH);
                    lastAutosaveOk = saveSuccess;
                    saveMessageUntil = GetTime() + 2.0;
                }
            }
        } else if (scene == SCENE_ARENA) {
            if (IsKeyPressed(KEY_ESCAPE)) {
                player.position = hubReturnPosition;
                scene = SCENE_HUB;
            } else if (mission.state == MISSION_CLEARED ||
                       mission.state == MISSION_FAILED) {
                if (IsKeyPressed(KEY_R)) {
                    player.position = ARENA_SPAWN;
                    CombatInit(&combat);
                    EconomyApplyToCombat(&economy, &combat);
                    MissionRetry(&mission, &combat, &arenaMap, ARENA_SPAWN);
                    lastAutosaveOk = true;
                }
            } else if (mission.state == MISSION_RUNNING) {
                UpdateWorldCamera(&camera, player.position);
                CombatUpdate(&combat, &player, &arenaMap, camera, dt);
                MissionUpdate(&mission, &combat, dt);

                // RUNNING -> CLEAREDは一度きり。繰り返し描画しても二重加算しない。
                if (mission.state == MISSION_CLEARED) {
                    EconomyEarn(&economy, mission.current.reward);
                    Player hubSnapshot = player;
                    hubSnapshot.position = hubReturnPosition;
                    // 戦闘座標ではなく拠点座標を保存し、CONTINUE可能にする。
                    lastAutosaveOk = SaveGameWrite(&hubSnapshot, &economy, SAVE_PATH);
                }
            }
        }

        if (scene != SCENE_MENU) UpdateWorldCamera(&camera, player.position);

        BeginDrawing();
        ClearBackground((Color){12, 15, 30, 255});
        if (scene == SCENE_MENU) {
            MenuDraw(&menu);
        } else {
            BeginMode2D(camera);
            if (scene == SCENE_HUB) {
                CityDrawMap(&hubMap, false, (float)GetTime());
                HubTerminalsDrawWorld();
            } else if (scene == SCENE_ARENA) {
                CityDrawMap(&arenaMap, true, (float)GetTime());
                CombatDrawWorld(&combat, &player);
            }
            PlayerDraw(&player);
            CityDrawRain((float)GetTime(), scene == SCENE_ARENA);
            EndMode2D();

            if (scene == SCENE_HUB) {
                CityHUDDrawHub(&economy, &player, &hubMap,
                               GetTime() < saveMessageUntil, saveSuccess);
                HubTerminalsDrawUI(&hub, player.position);
                if (HubTerminalsIsOpen(&hub)) {
                    if (hub.active == HUB_TERMINAL_MISSION)
                        MissionDrawSelection(&mission);
                    else if (hub.active == HUB_TERMINAL_SHOP) {
                        ShopDraw(&shop, &economy);
                        if (!lastAutosaveOk)
                            DrawText("WARNING: AUTO-SAVE FAILED! USE P IN HUB",
                                     180, 590, 20, RED);
                    }
                }
            } else if (scene == SCENE_ARENA) {
                if (mission.state == MISSION_RUNNING) {
                    CityHUDDrawArena(&combat, &mission, &economy,
                                     &player, &arenaMap);
                    CityHUDDrawCrosshair();
                } else if (mission.state == MISSION_CLEARED ||
                           mission.state == MISSION_FAILED) {
                    MissionDrawResult(&mission, &combat);
                    DrawText(TextFormat("TOTAL CREDITS: %d", economy.credits),
                             330, 477, 20, YELLOW);
                    if (!lastAutosaveOk)
                        DrawText("AUTO-SAVE FAILED: SAVE MANUALLY IN HUB",
                                 330, 603, 18, RED);
                }
            }
        }
        EndDrawing();
#ifdef __EMSCRIPTEN__
        emscripten_sleep(16); // Asyncify yields so input, drawing and audio can run.
#endif
    }

    UnloadMusicStream(bgm);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
