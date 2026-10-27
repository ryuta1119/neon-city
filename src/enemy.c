#include "enemy.h"
#include <math.h>
#include <string.h>

#define ENEMY_RADIUS 17.0f
#define ENEMY_BULLET_RADIUS 6.0f
#define ENEMY_BULLET_SPEED 420.0f

static Vector2 Subtract(Vector2 a, Vector2 b)
{
    return (Vector2){a.x - b.x, a.y - b.y};
}

static float Length(Vector2 v)
{
    return sqrtf(v.x * v.x + v.y * v.y);
}

static Vector2 NormalizeEnemy(Vector2 v)
{
    float length = Length(v);
    if (length < 0.0001f) return (Vector2){0, 0};
    return (Vector2){v.x / length, v.y / length};
}

static float TickEnemyTimer(float timer, float dt)
{
    return timer > dt ? timer - dt : 0.0f;
}

static Rectangle EnemyHitbox(Vector2 position)
{
    return (Rectangle){position.x - 15, position.y - 15, 30, 30};
}

static void MoveEnemy(Enemy *enemy, Vector2 delta, const TileMap *map)
{
    Vector2 next = enemy->position;
    next.x += delta.x;
    if (MapCanOccupy(map, EnemyHitbox(next))) enemy->position.x = next.x;
    next = enemy->position;
    next.y += delta.y;
    if (MapCanOccupy(map, EnemyHitbox(next))) enemy->position.y = next.y;
}

static bool HasLineOfSight(const TileMap *map, Vector2 from, Vector2 to)
{
    Vector2 delta = Subtract(to, from);
    float distance = Length(delta);
    int steps = (int)(distance / 12.0f);
    if (steps < 1) steps = 1;
    for (int i = 1; i < steps; i++) {
        float t = (float)i / (float)steps;
        Vector2 p = {from.x + delta.x * t, from.y + delta.y * t};
        if (!MapCanOccupy(map, (Rectangle){p.x - 3, p.y - 3, 6, 6})) {
            return false;
        }
    }
    return true;
}

static void FireEnemyBullet(EnemySystem *system, Vector2 position,
                            Vector2 direction, const TileMap *map)
{
    for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
        EnemyProjectile *bullet = &system->projectiles[i];
        if (bullet->active) continue;
        Vector2 spawn = {position.x + direction.x * 27.0f,
                         position.y + direction.y * 27.0f};
        Rectangle box = {spawn.x - ENEMY_BULLET_RADIUS,
                         spawn.y - ENEMY_BULLET_RADIUS,
                         ENEMY_BULLET_RADIUS * 2, ENEMY_BULLET_RADIUS * 2};
        if (!MapCanOccupy(map, box)) return;
        bullet->position = spawn;
        bullet->velocity = (Vector2){direction.x * ENEMY_BULLET_SPEED,
                                     direction.y * ENEMY_BULLET_SPEED};
        bullet->lifetime = 2.5f;
        bullet->active = true;
        return;
    }
}

void EnemiesInit(EnemySystem *system)
{
    memset(system, 0, sizeof(*system));
    // 旧版と同じ初期状態。ミッション開始時には自動生成した配置で上書きする。
    system->count = 3;
    system->enemies[0] = (Enemy){.type = ENEMY_STRIKER,
        .state = ENEMY_IDLE, .position = {780, 670}, .hp = 4, .maxHp = 4};
    system->enemies[1] = (Enemy){.type = ENEMY_STRIKER,
        .state = ENEMY_IDLE, .position = {1100, 930}, .hp = 4, .maxHp = 4};
    system->enemies[2] = (Enemy){.type = ENEMY_SHOOTER,
        .state = ENEMY_IDLE, .position = {1460, 680}, .hp = 3, .maxHp = 3};
}

int EnemiesUpdate(EnemySystem *system, const TileMap *map,
                  Vector2 playerPosition, float dt)
{
    int hits = 0;
    for (int i = 0; i < system->count; i++) {
        Enemy *enemy = &system->enemies[i];
        if (enemy->hp <= 0) continue;
        enemy->attackCooldown = TickEnemyTimer(enemy->attackCooldown, dt);
        Vector2 difference = Subtract(playerPosition, enemy->position);
        float distance = Length(difference);
        Vector2 direction = NormalizeEnemy(difference);
        if (distance > 620.0f) {
            enemy->state = ENEMY_IDLE;
            continue;
        }
        if (enemy->type == ENEMY_STRIKER) {
            if (distance > 43.0f) {
                enemy->state = ENEMY_CHASE;
                MoveEnemy(enemy, (Vector2){direction.x * 175.0f * dt,
                    direction.y * 175.0f * dt}, map);
            } else {
                enemy->state = ENEMY_ATTACK;
                if (enemy->attackCooldown <= 0) {
                    hits++;
                    enemy->attackCooldown = 0.95f;
                }
            }
        } else if (enemy->type == ENEMY_SHOOTER) {
            float moveDirection = 0.0f;
            if (distance > 350.0f) moveDirection = 1.0f;
            else if (distance < 190.0f) moveDirection = -1.0f;
            if (moveDirection != 0.0f) {
                enemy->state = ENEMY_CHASE;
                MoveEnemy(enemy, (Vector2){direction.x * moveDirection * 130.0f * dt,
                    direction.y * moveDirection * 130.0f * dt}, map);
            } else {
                enemy->state = ENEMY_ATTACK;
            }
            if (distance < 560.0f && enemy->attackCooldown <= 0 &&
                HasLineOfSight(map, enemy->position, playerPosition)) {
                FireEnemyBullet(system, enemy->position, direction, map);
                enemy->attackCooldown = 1.35f;
                enemy->state = ENEMY_ATTACK;
            }
        }
    }
    for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
        EnemyProjectile *bullet = &system->projectiles[i];
        if (!bullet->active) continue;
        bullet->lifetime -= dt;
        if (bullet->lifetime <= 0) {
            bullet->active = false;
            continue;
        }
        float step = dt / 4.0f;
        for (int s = 0; s < 4; s++) {
            Vector2 next = {bullet->position.x + bullet->velocity.x * step,
                            bullet->position.y + bullet->velocity.y * step};
            Rectangle box = {next.x - ENEMY_BULLET_RADIUS,
                             next.y - ENEMY_BULLET_RADIUS,
                             ENEMY_BULLET_RADIUS * 2, ENEMY_BULLET_RADIUS * 2};
            if (!MapCanOccupy(map, box)) {
                bullet->active = false;
                break;
            }
            bullet->position = next;
            if (CheckCollisionCircles(bullet->position, ENEMY_BULLET_RADIUS,
                                      playerPosition, 16.0f)) {
                hits++;
                bullet->active = false;
                break;
            }
        }
    }
    return hits;
}

bool EnemiesHitCircle(EnemySystem *system, Vector2 position, float radius, int damage)
{
    for (int i = 0; i < system->count; i++) {
        Enemy *enemy = &system->enemies[i];
        if (enemy->hp <= 0) continue;
        if (CheckCollisionCircles(position, radius, enemy->position, ENEMY_RADIUS)) {
            enemy->hp -= damage;
            if (enemy->hp < 0) enemy->hp = 0;
            return true;
        }
    }
    return false;
}

int EnemiesHitMelee(EnemySystem *system, const TileMap *map,
                    Vector2 origin, Vector2 facing, float range,
                    float minimumDot, int damage)
{
    int hits = 0;
    for (int i = 0; i < system->count; i++) {
        Enemy *enemy = &system->enemies[i];
        if (enemy->hp <= 0) continue;
        Vector2 difference = Subtract(enemy->position, origin);
        float distance = Length(difference);
        if (distance > range || distance < 0.001f) continue;
        Vector2 direction = NormalizeEnemy(difference);
        float dot = direction.x * facing.x + direction.y * facing.y;
        if (dot >= minimumDot && HasLineOfSight(map, origin, enemy->position)) {
            enemy->hp -= damage;
            if (enemy->hp < 0) enemy->hp = 0;
            hits++;
        }
    }
    return hits;
}

int EnemiesRemaining(const EnemySystem *system)
{
    int count = 0;
    for (int i = 0; i < system->count; i++) {
        if (system->enemies[i].hp > 0) count++;
    }
    return count;
}

void EnemiesDrawWorld(const EnemySystem *system)
{
    for (int i = 0; i < system->count; i++) {
        const Enemy *enemy = &system->enemies[i];
        if (enemy->hp <= 0) continue;
        Color color = enemy->type == ENEMY_STRIKER ? RED : PURPLE;
        DrawCircleV(enemy->position, ENEMY_RADIUS, color);
        DrawCircleLines((int)enemy->position.x, (int)enemy->position.y,
                        ENEMY_RADIUS, WHITE);
        int x = (int)enemy->position.x - 20;
        int y = (int)enemy->position.y - 39;
        DrawRectangle(x, y, 40, 6, DARKGRAY);
        DrawRectangle(x, y, 40 * enemy->hp / enemy->maxHp, 6, GREEN);
        DrawText(enemy->type == ENEMY_STRIKER ? "STR" : "SHT",
                 x, y - 19, 14, WHITE);
    }
    for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
        const EnemyProjectile *bullet = &system->projectiles[i];
        if (bullet->active) {
            DrawCircleV(bullet->position, ENEMY_BULLET_RADIUS, RED);
        }
    }
}
