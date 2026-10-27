#include "mission.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static const char *missionNames[] = {"ELIMINATION", "SURVIVAL", "TIME ATTACK"};
static const char *difficultyNames[] = {"EASY", "NORMAL", "HARD"};

// 32-bit の決定的な疑似乱数: 同じseedから同じ結果を得る
static uint32_t NextRandom(uint32_t *state)
{
    uint32_t x = *state;
    if (x == 0) x = 0x6d2b79f5u;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

static int RandomRange(uint32_t *state, int lower, int upper)
{
    return lower + (int)(NextRandom(state) % (uint32_t)(upper - lower + 1));
}

static float DistanceSquared(Vector2 a, Vector2 b)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return dx * dx + dy * dy;
}

static float TimeRemaining(const MissionSystem *mission)
{
    float remaining = mission->current.timeLimit - mission->elapsed;
    return remaining > 0.0f ? remaining : 0.0f;
}

static Rectangle MissionButton(int i)
{
    return (Rectangle){170.0f, 218.0f + i * 125.0f, 940.0f, 108.0f};
}

void MissionInit(MissionSystem *mission)
{
    memset(mission, 0, sizeof(*mission));
    mission->state = MISSION_READY;
}

void MissionGenerateOffers(MissionSystem *mission, uint32_t seed)
{
    uint32_t random = seed ? seed : 1u;
    int typeOffset = RandomRange(&random, 0, MISSION_COUNT - 1);
    mission->selected = 0;
    mission->state = MISSION_READY;

    for (int i = 0; i < MISSION_COUNT; i++) {
        MissionOffer *offer = &mission->offers[i];
        offer->type = (MissionType)((i + typeOffset) % MISSION_COUNT);
        offer->difficulty = (MissionDifficulty)RandomRange(&random, 0, 2);
        int d = (int)offer->difficulty;
        offer->enemyCount = RandomRange(&random, 3 + 2 * d, 4 + 3 * d);
        if (offer->enemyCount > MAX_ENEMIES) offer->enemyCount = MAX_ENEMIES;
        offer->shooterCount = RandomRange(&random, 0, 1 + d);
        if (offer->shooterCount > offer->enemyCount / 2) {
            offer->shooterCount = offer->enemyCount / 2;
        }
        offer->hpBonus = d;  // EASY=0, NORMAL=1, HARD=2
        if (offer->type == MISSION_SURVIVAL) {
            offer->timeLimit = d == 0 ? 35.0f : (d == 1 ? 45.0f : 60.0f);
        } else if (offer->type == MISSION_TIME_ATTACK) {
            offer->timeLimit = d == 0 ? 90.0f : (d == 1 ? 75.0f : 60.0f);
        } else {
            offer->timeLimit = 0.0f;
        }
        offer->reward = 100 + (d + 1) * 160 + offer->enemyCount * 40;
        if (offer->type == MISSION_TIME_ATTACK) offer->reward += 80;
        // 各候補に固有seedを割り当てる。再挑戦時はこのseedを再利用する。
        offer->seed = NextRandom(&random);
        if (offer->seed == 0) offer->seed = 1;
    }
}

bool MissionSelectionUpdate(MissionSystem *mission)
{
    if (IsKeyPressed(KEY_R)) {
        MissionGenerateOffers(mission, (uint32_t)GetRandomValue(1, 2147483647));
        return false;
    }
    if (IsKeyPressed(KEY_DOWN)) mission->selected = (mission->selected + 1) % MISSION_COUNT;
    if (IsKeyPressed(KEY_UP)) {
        mission->selected = (mission->selected + MISSION_COUNT - 1) % MISSION_COUNT;
    }
    if (IsKeyPressed(KEY_ENTER)) return true;

    Vector2 mouse = GetMousePosition();
    Vector2 mouseDelta = GetMouseDelta();
    for (int i = 0; i < MISSION_COUNT; i++) {
        if (!CheckCollisionPointRec(mouse, MissionButton(i))) continue;
        if (mouseDelta.x != 0 || mouseDelta.y != 0) mission->selected = i;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            mission->selected = i;
            return true;
        }
    }
    return false;
}

static bool CandidateAvailable(const EnemySystem *enemies, const TileMap *map,
                               Vector2 candidate, Vector2 playerSpawn)
{
    if (DistanceSquared(candidate, playerSpawn) < 200.0f * 200.0f) return false;
    if (!MapCanOccupy(map, (Rectangle){candidate.x - 18, candidate.y - 18, 36, 36})) {
        return false;
    }
    for (int i = 0; i < enemies->count; i++) {
        if (DistanceSquared(candidate, enemies->enemies[i].position) < 90.0f * 90.0f) {
            return false;
        }
    }
    return true;
}

static void BuildEnemyRoster(EnemySystem *system, const TileMap *map,
                             Vector2 spawn, const MissionOffer *offer)
{
    memset(system, 0, sizeof(*system));
    uint32_t random = offer->seed;
    int desired = offer->enemyCount;
    if (desired > MAX_ENEMIES) desired = MAX_ENEMIES;

    for (int i = 0; i < desired; i++) {
        bool placed = false;
        Vector2 position = {0};
        // 障害物やプレイヤー初期位置を避ける
        for (int attempt = 0; attempt < 500; attempt++) {
            int tileX = RandomRange(&random, 2, MAP_COLS - 3);
            int tileY = RandomRange(&random, 2, MAP_ROWS - 3);
            position = (Vector2){(tileX + 0.5f) * TILE_SIZE,
                                 (tileY + 0.5f) * TILE_SIZE};
            if (CandidateAvailable(system, map, position, spawn)) {
                placed = true;
                break;
            }
        }
        if (!placed) {
            // 通行可能な空きマスを順番に探索するフォールバック
            for (int y = 2; y < MAP_ROWS - 2 && !placed; y++) {
                for (int x = 2; x < MAP_COLS - 2 && !placed; x++) {
                    position = (Vector2){(x + 0.5f) * TILE_SIZE,
                                         (y + 0.5f) * TILE_SIZE};
                    placed = CandidateAvailable(system, map, position, spawn);
                }
            }
        }
        if (!placed) break;
        EnemyType type = (i < offer->shooterCount) ? ENEMY_SHOOTER : ENEMY_STRIKER;
        int baseHP = type == ENEMY_STRIKER ? 4 : 3;
        Enemy *enemy = &system->enemies[system->count++];
        *enemy = (Enemy){.type = type, .state = ENEMY_IDLE,
                         .position = position,
                         .hp = baseHP + offer->hpBonus,
                         .maxHp = baseHP + offer->hpBonus};
    }
}

void MissionStart(MissionSystem *mission, CombatSystem *combat,
                  const TileMap *map, Vector2 spawn)
{
    mission->current = mission->offers[mission->selected];
    mission->state = MISSION_RUNNING;
    mission->elapsed = 0.0f;
    BuildEnemyRoster(&combat->enemies, map, spawn, &mission->current);
}

void MissionRetry(MissionSystem *mission, CombatSystem *combat,
                  const TileMap *map, Vector2 spawn)
{
    // offers[]は変更しないので、同じ敵・配置・条件が再現される
    mission->state = MISSION_RUNNING;
    mission->elapsed = 0.0f;
    BuildEnemyRoster(&combat->enemies, map, spawn, &mission->current);
}

void MissionUpdate(MissionSystem *mission, const CombatSystem *combat, float dt)
{
    if (mission->state != MISSION_RUNNING) return;
    mission->elapsed += dt;
    int remaining = EnemiesRemaining(&combat->enemies);
    if (combat->playerHP <= 0) {
        mission->state = MISSION_FAILED;
        return;
    }
    switch (mission->current.type) {
        case MISSION_ELIMINATION:
            if (remaining == 0) mission->state = MISSION_CLEARED;
            break;
        case MISSION_SURVIVAL:
            if (mission->elapsed >= mission->current.timeLimit) {
                mission->state = MISSION_CLEARED;
            }
            break;
        case MISSION_TIME_ATTACK:
            if (remaining == 0 && mission->elapsed <= mission->current.timeLimit) {
                mission->state = MISSION_CLEARED;
            } else if (mission->elapsed >= mission->current.timeLimit) {
                mission->state = MISSION_FAILED;
            }
            break;
    }
}

void MissionDrawSelection(const MissionSystem *mission)
{
    Color cyan = (Color){72, 222, 236, 255};
    Color gold = (Color){239, 198, 114, 255};
    Color muted = (Color){164, 182, 201, 255};
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                  (Color){8, 13, 25, 253});
    DrawRectangle(0, 0, GetScreenWidth(), 5, cyan);
    DrawText("/ CONTRACT BOARD", 168, 43, 46, WHITE);
    DrawText("PROCEDURAL OPERATIONS  /  CENTRAL HUB", 173, 111, 18, cyan);
    DrawRectangle(170, 170, 940, 2, Fade(cyan, 0.34f));
    DrawText("03 AVAILABLE TRANSMISSIONS", 173, 181, 18, muted);

    for (int i = 0; i < MISSION_COUNT; i++) {
        const MissionOffer *offer = &mission->offers[i];
        Rectangle rect = MissionButton(i);
        bool selected = mission->selected == i;
        DrawRectangleRec(rect, selected ? (Color){25, 44, 63, 255}
                                         : (Color){19, 29, 48, 255});
        DrawRectangleLinesEx(rect, selected ? 2.5f : 1.0f,
                             selected ? cyan : (Color){61, 83, 105, 255});
        if (selected) DrawRectangle((int)rect.x, (int)rect.y,
                                    5, (int)rect.height, cyan);
        DrawText(TextFormat("%02d", i + 1), (int)rect.x + 21,
                 (int)rect.y + 28, 28, selected ? cyan : muted);
        DrawText(missionNames[offer->type], (int)rect.x + 83,
                 (int)rect.y + 15, 29, WHITE);
        DrawText(difficultyNames[offer->difficulty], (int)rect.x + 746,
                 (int)rect.y + 17, 22,
                 offer->difficulty == MISSION_HARD ? ORANGE : cyan);
        char timeLabel[32];
        if (offer->timeLimit > 0.0f)
            snprintf(timeLabel, sizeof(timeLabel), "%.0f SEC", offer->timeLimit);
        else snprintf(timeLabel, sizeof(timeLabel), "NO LIMIT");
        DrawText(TextFormat("%d HOSTILES  /  %d SHOOTERS  /  %s",
                 offer->enemyCount, offer->shooterCount, timeLabel),
                 (int)rect.x + 84, (int)rect.y + 57, 18, muted);
        DrawText(TextFormat("%d CR", offer->reward),
                 (int)rect.x + 748, (int)rect.y + 61, 19, gold);
        DrawText(TextFormat("SEED %08X", (unsigned int)offer->seed),
                 (int)rect.x + 747, (int)rect.y + 87, 12, muted);
    }
    DrawRectangle(0, 612, GetScreenWidth(), 1, Fade(cyan, 0.35f));
    DrawText("UP / DOWN  SELECT        ENTER  DEPLOY        R  REROLL        ESC  EXIT",
             181, 648, 19, WHITE);
}

void MissionDrawHUD(const MissionSystem *mission, const CombatSystem *combat)
{
    const MissionOffer *offer = &mission->current;
    DrawRectangle(825, 12, 440, 165, Fade(BLACK, 0.85f));
    DrawText(missionNames[offer->type], 842, 25, 26, SKYBLUE);
    DrawText(TextFormat("%s  |  SEED %08X", difficultyNames[offer->difficulty],
             (unsigned int)offer->seed), 842, 62, 18, LIGHTGRAY);
    DrawText(TextFormat("ENEMIES: %d / %d", EnemiesRemaining(&combat->enemies),
             combat->enemies.count), 842, 95, 21, WHITE);
    if (offer->timeLimit > 0) {
        DrawText(TextFormat("TIME LEFT: %.1f", TimeRemaining(mission)),
                 842, 129, 23, YELLOW);
    } else {
        DrawText(TextFormat("TIME: %.1f", mission->elapsed), 842, 129, 23, YELLOW);
    }
}

void MissionDrawResult(const MissionSystem *mission, const CombatSystem *combat)
{
    bool cleared = mission->state == MISSION_CLEARED;
    const MissionOffer *offer = &mission->current;
    Color cyan = (Color){70, 218, 231, 255};
    Color gold = (Color){244, 204, 117, 255};
    Color accent = cleared ? cyan : RED;
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                  (Color){6, 11, 21, 246});
    Rectangle panel = {248, 118, 785, 482};
    DrawRectangleRec(panel, (Color){16, 28, 46, 255});
    DrawRectangleLinesEx(panel, 2, accent);
    DrawRectangle((int)panel.x, (int)panel.y, (int)panel.width, 6, accent);
    DrawText("/ AFTER-ACTION REPORT", 330, 143, 17, accent);
    const char *title = cleared ? "MISSION CLEAR" : "MISSION FAILED";
    DrawText(title, 328, 183, 47, WHITE);
    DrawRectangle(328, 254, 622, 1, Fade(accent, 0.42f));
    DrawText(TextFormat("%s   /   %s", missionNames[offer->type],
             difficultyNames[offer->difficulty]), 330, 274, 24, accent);
    DrawText(TextFormat("TIME  %.1f SEC", mission->elapsed),
             330, 326, 22, WHITE);
    DrawText(TextFormat("HOSTILES NEUTRALIZED  %d / %d",
             combat->enemies.count - EnemiesRemaining(&combat->enemies),
             combat->enemies.count), 330, 367, 22, WHITE);
    DrawText(TextFormat("MISSION PAYOUT  +%d CR", cleared ? offer->reward : 0),
             330, 411, 23, gold);
    DrawText(TextFormat("SEED  %08X", (unsigned int)offer->seed),
             330, 448, 16, (Color){164, 179, 197, 255});
    DrawText("R  REDEPLOY       ESC  CENTRAL HUB", 330, 535, 21, WHITE);
}
