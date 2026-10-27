#ifndef NEON_MISSION_H
#define NEON_MISSION_H

#include "raylib.h"
#include "combat.h"
#include <stdbool.h>
#include <stdint.h>

#define MISSION_COUNT 3

typedef enum {
    MISSION_ELIMINATION,
    MISSION_SURVIVAL,
    MISSION_TIME_ATTACK
} MissionType;

typedef enum {
    MISSION_READY,
    MISSION_RUNNING,
    MISSION_CLEARED,
    MISSION_FAILED
} MissionState;

typedef enum {
    MISSION_EASY,
    MISSION_NORMAL,
    MISSION_HARD
} MissionDifficulty;

typedef struct {
    MissionType type;
    MissionDifficulty difficulty;
    int enemyCount;
    int shooterCount;
    int hpBonus;
    float timeLimit;
    int reward;
    uint32_t seed;
} MissionOffer;

typedef struct {
    int selected;
    MissionState state;
    float elapsed;
    MissionOffer offers[MISSION_COUNT];
    MissionOffer current;
} MissionSystem;

void MissionInit(MissionSystem *mission);
void MissionGenerateOffers(MissionSystem *mission, uint32_t seed);
bool MissionSelectionUpdate(MissionSystem *mission);
void MissionStart(MissionSystem *mission, CombatSystem *combat,
                  const TileMap *map, Vector2 spawn);
void MissionRetry(MissionSystem *mission, CombatSystem *combat,
                  const TileMap *map, Vector2 spawn);
void MissionUpdate(MissionSystem *mission, const CombatSystem *combat, float dt);
void MissionDrawSelection(const MissionSystem *mission);
void MissionDrawHUD(const MissionSystem *mission, const CombatSystem *combat);
void MissionDrawResult(const MissionSystem *mission, const CombatSystem *combat);

#endif
