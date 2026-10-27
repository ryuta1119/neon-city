#include "combat.h"
#include <math.h>
#include <string.h>
#define BULLET_SPEED 950.0f
#define BULLET_RADIUS 5.0f
#define DASH_SPEED 850.0f
#define DASH_DURATION 0.16f
#define DASH_COOLDOWN 0.80f
#define MELEE_RANGE 90.0f
// ===============================
// タイマー更新
// ===============================
static float TickTimer(float timer, float dt)
{
    if (timer <= dt)
        return 0.0f;
    return timer - dt;
}
// ===============================
// ベクトル正規化
// ===============================
static Vector2 Normalize(Vector2 v)
{
    float length = sqrtf(
        v.x * v.x + v.y * v.y
    );
    if (length <= 0.0001f)
        return (Vector2){0, 0};
    return (Vector2){
        v.x / length,
        v.y / length
    };
}
// ===============================
// 戦闘初期化
// ===============================
void CombatInit(CombatSystem *combat)
{
    memset(combat, 0, sizeof(*combat));
    // 敵AIを初期化
    EnemiesInit(&combat->enemies);
    // プレイヤーHP
    combat->playerMaxHP = 5;
    combat->playerHP = 5;
    combat->gunDamage = 1;
    combat->meleeDamage = 2;
    combat->fireInterval = 0.14f;
    // 無敵時間
    combat->invulnerability = 0.0f;
    combat->facing = (Vector2){1, 0};
    combat->targets[0] = (TrainingTarget){
        {500, 480}, 3
    };
    combat->targets[1] = (TrainingTarget){
        {1510, 440}, 3
    };
    combat->targets[2] = (TrainingTarget){
        {1550, 1010}, 3
    };
}
// ===============================
// 射撃
// ===============================
static void FireBullet(
    CombatSystem *combat,
    const Player *player,
    const TileMap *map
)
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        CombatBullet *bullet = &combat->bullets[i];
        if (bullet->active)
            continue;
        Vector2 spawn = {
            player->position.x
                + combat->facing.x * 28.0f,
            player->position.y
                + combat->facing.y * 28.0f
        };
        Rectangle hitbox = {
            spawn.x - BULLET_RADIUS,
            spawn.y - BULLET_RADIUS,
            BULLET_RADIUS * 2,
            BULLET_RADIUS * 2
        };
        if (!MapCanOccupy(map, hitbox))
            return;
        bullet->position = spawn;
        bullet->velocity = (Vector2){
            combat->facing.x * BULLET_SPEED,
            combat->facing.y * BULLET_SPEED
        };
        bullet->lifetime = 1.5f;
        bullet->active = true;
        return;
    }
}
// ===============================
// 近接攻撃
// ===============================
static void MeleeAttack(
    CombatSystem *combat,
    const Player *player,
    const TileMap *map
)
{
    combat->meleeEffectTime = 0.14f;
    for (int i = 0; i < TARGET_COUNT; i++)
    {
        TrainingTarget *target = &combat->targets[i];
        if (target->hp <= 0)
            continue;
        Vector2 difference = {
            target->position.x - player->position.x,
            target->position.y - player->position.y
        };
        float distance = sqrtf(
            difference.x * difference.x +
            difference.y * difference.y
        );
        if (distance > MELEE_RANGE ||
            distance <= 0.001f)
        {
            continue;
        }
        Vector2 direction = Normalize(difference);
        // 照準方向との内積
        float dot =
            direction.x * combat->facing.x +
            direction.y * combat->facing.y;
        // 前方の扇形範囲にいる場合
        if (dot >= 0.65f)
        {
            target->hp -= combat->meleeDamage;
            if (target->hp < 0)
                target->hp = 0;
        }
    }
    // 敵への近接攻撃判定
    EnemiesHitMelee(
        &combat->enemies,
        map,
        player->position,
        combat->facing,
        MELEE_RANGE,
        0.65f,
        combat->meleeDamage
    );
}
// ===============================
// 弾丸更新
// ===============================
static void UpdateBullets(
    CombatSystem *combat,
    const TileMap *map,
    float dt
)
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        CombatBullet *bullet = &combat->bullets[i];
        if (!bullet->active)
            continue;
        bullet->lifetime -= dt;
        if (bullet->lifetime <= 0)
        {
            bullet->active = false;
            continue;
        }
        // 高速移動による壁のすり抜けを防ぐ
        // 1フレームの移動を4回に分割
        float step = dt / 4.0f;
        for (int s = 0; s < 4; s++)
        {
            Vector2 next = {
                bullet->position.x
                    + bullet->velocity.x * step,
                bullet->position.y
                    + bullet->velocity.y * step
            };
            Rectangle hitbox = {
                next.x - BULLET_RADIUS,
                next.y - BULLET_RADIUS,
                BULLET_RADIUS * 2,
                BULLET_RADIUS * 2
            };
            // 壁との衝突
            if (!MapCanOccupy(map, hitbox))
            {
                bullet->active = false;
                break;
            }
            bullet->position = next;
            // ターゲットとの衝突
            for (int t = 0; t < TARGET_COUNT; t++)
            {
                TrainingTarget *target =
                    &combat->targets[t];
                if (target->hp <= 0)
                    continue;
                bool hit = CheckCollisionCircles(
                    bullet->position,
                    BULLET_RADIUS,
                    target->position,
                    24.0f
                );
                if (hit)
                {
                    target->hp--;
                    bullet->active = false;
                    break;
                }
            }
            // 敵との命中判定（壁や訓練ターゲットに当たった弾は除外）
            if (bullet->active &&
                EnemiesHitCircle(
                    &combat->enemies,
                    bullet->position,
                    BULLET_RADIUS,
                    combat->gunDamage))
            {
                bullet->active = false;
            }
            if (!bullet->active)
                break;
        }
    }
}
// ===============================
// 戦闘更新
// ===============================
void CombatUpdate(
    CombatSystem *combat,
    Player *player,
    const TileMap *map,
    Camera2D camera,
    float dt
)
{
    // HPが0になったらアクション更新を停止する
    if (combat->playerHP <= 0)
        return;

    // 被ダメージ後の無敵時間を更新
    combat->invulnerability = TickTimer(
        combat->invulnerability, dt);

    // クールダウン更新
    combat->shootCooldown =
        TickTimer(combat->shootCooldown, dt);
    combat->meleeCooldown =
        TickTimer(combat->meleeCooldown, dt);
    combat->meleeEffectTime =
        TickTimer(combat->meleeEffectTime, dt);
    combat->dashCooldown =
        TickTimer(combat->dashCooldown, dt);
    // ===========================
    // 移動方向
    // ===========================
    Vector2 movement = {0};
    if (IsKeyDown(KEY_W)) movement.y -= 1;
    if (IsKeyDown(KEY_S)) movement.y += 1;
    if (IsKeyDown(KEY_A)) movement.x -= 1;
    if (IsKeyDown(KEY_D)) movement.x += 1;
    movement = Normalize(movement);
    // ===========================
    // マウス照準
    // ===========================
    Vector2 mouseWorld = GetScreenToWorld2D(
        GetMousePosition(),
        camera
    );
    Vector2 aim = {
        mouseWorld.x - player->position.x,
        mouseWorld.y - player->position.y
    };
    Vector2 normalizedAim = Normalize(aim);
    if (normalizedAim.x != 0 ||
        normalizedAim.y != 0)
    {
        combat->facing = normalizedAim;
    }
    // ===========================
    // ダッシュ開始
    // ===========================
    bool shiftPressed =
        IsKeyPressed(KEY_LEFT_SHIFT) ||
        IsKeyPressed(KEY_RIGHT_SHIFT);
    if (shiftPressed &&
        combat->dashCooldown <= 0 &&
        combat->dashTime <= 0 &&
        (movement.x != 0 || movement.y != 0))
    {
        combat->dashTime = DASH_DURATION;
        combat->dashCooldown = DASH_COOLDOWN;
        combat->dashDirection = movement;
    }
    // ===========================
    // プレイヤー移動
    // ===========================
    Vector2 velocityDirection = movement;
    float speed = player->speed;
    if (combat->dashTime > 0)
    {
        velocityDirection = combat->dashDirection;
        speed = DASH_SPEED;
        combat->dashTime =
            TickTimer(combat->dashTime, dt);
    }
    PlayerMove(
        player,
        (Vector2){
            velocityDirection.x * speed * dt,
            velocityDirection.y * speed * dt
        },
        map
    );
    // ===========================
    // 射撃
    // ===========================
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
        combat->shootCooldown <= 0 &&
        combat->dashTime <= 0)
    {
        FireBullet(combat, player, map);
        combat->shootCooldown = combat->fireInterval;
    }
    // ===========================
    // 近接攻撃
    // ===========================
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) &&
        combat->meleeCooldown <= 0 &&
        combat->dashTime <= 0)
    {
        MeleeAttack(combat, player, map);
        combat->meleeCooldown = 0.38f;
    }
    // プレイヤーの弾丸の移動・命中判定
    UpdateBullets(combat, map, dt);

    // 敵の行動・敵弾の移動・プレイヤーへの命中判定
    int damageEvents = EnemiesUpdate(
        &combat->enemies,
        map,
        player->position,
        dt);

    // 複数ヒットが同時発生しても無敵時間中は1ダメージのみ
    if (damageEvents > 0 && combat->invulnerability <= 0.0f)
    {
        combat->playerHP--;
        if (combat->playerHP < 0)
            combat->playerHP = 0;
        combat->invulnerability = 0.9f;
    }
}
// ===============================
// 戦闘ワールド描画
// ===============================
void CombatDrawWorld(
    const CombatSystem *combat,
    const Player *player
)
{
    // ===========================
    // ターゲット
    // ===========================
    for (int i = 0; i < TARGET_COUNT; i++)
    {
        const TrainingTarget *target =
            &combat->targets[i];
        Color color = target->hp > 0
            ? ORANGE
            : DARKGRAY;
        DrawCircleV(
            target->position,
            24,
            color
        );
        DrawCircleLines(
            (int)target->position.x,
            (int)target->position.y,
            24,
            WHITE
        );
        DrawText(
            TextFormat("%d HP", target->hp),
            (int)target->position.x - 24,
            (int)target->position.y - 44,
            18,
            WHITE
        );
    }
    // ===========================
    // 弾丸
    // ===========================
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        const CombatBullet *bullet =
            &combat->bullets[i];
        if (!bullet->active)
            continue;
        DrawCircleV(
            bullet->position,
            BULLET_RADIUS,
            YELLOW
        );
    }

    // 敵キャラクターと敵の弾丸
    EnemiesDrawWorld(&combat->enemies);

    // ===========================
    // ダッシュエフェクト
    // ===========================
    if (combat->dashTime > 0)
    {
        DrawCircleV(
            player->position,
            32,
            Fade(SKYBLUE, 0.25f)
        );
    }
    // ===========================
    // 近接攻撃エフェクト
    // ===========================
    if (combat->meleeEffectTime > 0)
    {
        float angle = atan2f(
            combat->facing.y,
            combat->facing.x
        ) * 180.0f / 3.14159265f;
        DrawCircleSector(
            player->position,
            76,
            angle - 45,
            angle + 45,
            24,
            Fade(MAGENTA, 0.55f)
        );
    }
    // ===========================
    // 照準方向
    // ===========================
    Vector2 aimEnd = {
        player->position.x
            + combat->facing.x * 45,
        player->position.y
            + combat->facing.y * 45
    };
    DrawLineEx(
        player->position,
        aimEnd,
        4,
        YELLOW
    );
}
// ===============================
// 戦闘HUD
// ===============================
void CombatDrawHUD(const CombatSystem *combat)
{
    int cleared = 0;
    for (int i = 0; i < TARGET_COUNT; i++)
    {
        if (combat->targets[i].hp <= 0)
            cleared++;
    }
    DrawRectangle(
        12, 12, 430, 210,
        Fade(BLACK, 0.8f)
    );
    DrawText(
        "TRAINING ARENA",
        30, 25, 30, SKYBLUE
    );
    DrawText(
        "WASD : MOVE",
        30, 75, 20, WHITE
    );
    DrawText(
        "LEFT CLICK : SHOOT",
        30, 105, 20, WHITE
    );
    DrawText(
        "RIGHT CLICK : MELEE",
        30, 135, 20, WHITE
    );
    DrawText(
        "SHIFT + WASD : DASH",
        30, 165, 20, WHITE
    );
    DrawText(
        TextFormat(
            "TARGETS: %d / %d",
            cleared,
            TARGET_COUNT
        ),
        30, 235, 24, YELLOW
    );
    if (cleared == TARGET_COUNT)
    {
        const char *message = "ALL TARGETS CLEARED";
        int width = MeasureText(message, 40);
        DrawText(
            message,
            GetScreenWidth() / 2 - width / 2,
            280,
            32,
            GREEN
        );
    }
    DrawText(
        "ESC : RETURN TO HUB",
        30,
        GetScreenHeight() - 45,
        20,
        LIGHTGRAY
    );

    // HPと敵の状況を表示するパネル
    DrawRectangle(475, 12, 320, 130, Fade(BLACK, 0.8f));
    DrawText(
        TextFormat("HP: %d / %d", combat->playerHP, combat->playerMaxHP),
        495, 25, 26, WHITE);

    DrawRectangle(495, 65, 270, 20, DARKGRAY);
    if (combat->playerMaxHP > 0)
    {
        DrawRectangle(
            495, 65,
            270 * combat->playerHP / combat->playerMaxHP,
            20,
            (Color){55, 220, 130, 255});
    }

    DrawText(
        TextFormat("ENEMIES: %d", EnemiesRemaining(&combat->enemies)),
        495, 100, 24, WHITE);

    // ゲームオーバー表示を最後に描画する
    if (combat->playerHP <= 0)
    {
        DrawRectangle(
            0, 0,
            GetScreenWidth(), GetScreenHeight(),
            Fade(BLACK, 0.78f));

        const char *message = "GAME OVER";
        DrawText(
            message,
            GetScreenWidth()/2 - MeasureText(message, 60)/2,
            260, 60, RED);
        DrawText("R : RETRY", 545, 370, 28, WHITE);
        DrawText("ESC : RETURN TO HUB", 470, 420, 24, LIGHTGRAY);
    }
}
