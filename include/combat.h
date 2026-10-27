#ifndef NEON_COMBAT_H
#define NEON_COMBAT_H

#include "raylib.h"
#include "player.h"
#include "map.h"
#include "enemy.h"

#include <stdbool.h>

#define MAX_BULLETS 80
#define TARGET_COUNT 3

// 弾丸
typedef struct {
    Vector2 position;
    Vector2 velocity;

    float lifetime;
    bool active;
} CombatBullet;

// 訓練用ターゲット
typedef struct {
    Vector2 position;
    int hp;
} TrainingTarget;

// 戦闘システム
typedef struct {
    CombatBullet bullets[MAX_BULLETS];
    TrainingTarget targets[TARGET_COUNT];

    // 照準方向
    Vector2 facing;

    // 攻撃
    float shootCooldown;
    float meleeCooldown;
    float meleeEffectTime;

    // ダッシュ
    float dashTime;
    float dashCooldown;
    Vector2 dashDirection;

        // 敵AI
    EnemySystem enemies;

    // プレイヤーHP
    int playerHP;
    int playerMaxHP;

    // 被弾後の無敵時間
    float invulnerability;
    int gunDamage;
    int meleeDamage;
    float fireInterval;
} CombatSystem;

// 初期化
void CombatInit(CombatSystem *combat);

// 更新
void CombatUpdate(
    CombatSystem *combat,
    Player *player,
    const TileMap *map,
    Camera2D camera,
    float dt
);

// ワールド描画
void CombatDrawWorld(
    const CombatSystem *combat,
    const Player *player
);

// HUD描画
void CombatDrawHUD(const CombatSystem *combat);

#endif