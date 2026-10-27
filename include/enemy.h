#ifndef NEON_ENEMY_H
#define NEON_ENEMY_H

#include "raylib.h"
#include "map.h"
#include <stdbool.h>

#define MAX_ENEMIES 12
#define MAX_ENEMY_PROJECTILES 32

typedef enum {
    ENEMY_STRIKER,
    ENEMY_SHOOTER
} EnemyType;

typedef enum {
    ENEMY_IDLE,
    ENEMY_CHASE,
    ENEMY_ATTACK
} EnemyState;

typedef struct {
    EnemyType type;
    EnemyState state;
    Vector2 position;
    int hp;
    int maxHp;
    float attackCooldown;
} Enemy;

typedef struct {
    Vector2 position;
    Vector2 velocity;
    float lifetime;
    bool active;
} EnemyProjectile;

typedef struct {
    Enemy enemies[MAX_ENEMIES];
    int count; // 今回のミッションで実際に登場する敵の数
    EnemyProjectile projectiles[MAX_ENEMY_PROJECTILES];
} EnemySystem;

void EnemiesInit(EnemySystem *system);
int EnemiesUpdate(EnemySystem *system, const TileMap *map, Vector2 playerPosition, float dt);
bool EnemiesHitCircle(EnemySystem *system, Vector2 position, float radius, int damage);
int EnemiesHitMelee(EnemySystem *system, const TileMap *map, Vector2 origin,
                    Vector2 facing, float range, float minimumDot, int damage);
void EnemiesDrawWorld(const EnemySystem *system);
int EnemiesRemaining(const EnemySystem *system);

#endif
