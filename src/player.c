#include "player.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

// ===============================
// プレイヤー初期化
// ===============================

void PlayerInit(Player *player)
{
    player->position = (Vector2){
        16.5f * TILE_SIZE,
        9.5f * TILE_SIZE
    };

    player->speed = 300.0f;
}

// ===============================
// 当たり判定用矩形
// ===============================

static Rectangle PlayerHitbox(Vector2 position)
{
    return (Rectangle){
        position.x - PLAYER_SIZE / 2.0f,
        position.y - PLAYER_SIZE / 2.0f,
        PLAYER_SIZE,
        PLAYER_SIZE
    };
}

// ===============================
// 指定移動量による移動
// ===============================

void PlayerMove(
    Player *player,
    Vector2 delta,
    const TileMap *map
)
{
    // X方向の移動
    Vector2 nextX = player->position;

    nextX.x += delta.x;

    if (MapCanOccupy(
            map,
            PlayerHitbox(nextX)))
    {
        player->position.x = nextX.x;
    }

    // Y方向の移動
    Vector2 nextY = player->position;

    nextY.y += delta.y;

    if (MapCanOccupy(
            map,
            PlayerHitbox(nextY)))
    {
        player->position.y = nextY.y;
    }
}

// ===============================
// 通常移動の更新
// ===============================

void PlayerUpdate(
    Player *player,
    float dt,
    const TileMap *map
)
{
    Vector2 direction = {0};

    // キーボード入力
    if (IsKeyDown(KEY_W)) direction.y -= 1;
    if (IsKeyDown(KEY_S)) direction.y += 1;
    if (IsKeyDown(KEY_A)) direction.x -= 1;
    if (IsKeyDown(KEY_D)) direction.x += 1;

    // 移動方向の正規化
    float length = sqrtf(
        direction.x * direction.x +
        direction.y * direction.y
    );

    if (length > 0.0001f)
    {
        direction.x /= length;
        direction.y /= length;
    }

    // 移動量を計算
    Vector2 delta = {
        direction.x * player->speed * dt,
        direction.y * player->speed * dt
    };

    // 共通移動処理
    PlayerMove(
        player,
        delta,
        map
    );
}

// ===============================
// プレイヤー描画
// ===============================

void PlayerDraw(const Player *player)
{
    Rectangle rect = PlayerHitbox(
        player->position
    );

    DrawRectangleRec(
        rect,
        SKYBLUE
    );

    DrawRectangleLinesEx(
        rect,
        2,
        WHITE
    );

    DrawCircleV(
        player->position,
        4,
        WHITE
    );
}

// ===============================
// セーブ
// ===============================

bool PlayerSave(
    const Player *player,
    const char *path
)
{
    FILE *file = fopen(path, "w");

    if (file == NULL)
        return false;

    int result = fprintf(
        file,
        "NC1 %.6f %.6f\n",
        player->position.x,
        player->position.y
    );

    int closeResult = fclose(file);

    return result > 0 && closeResult == 0;
}

// ===============================
// ロード
// ===============================

bool PlayerLoad(
    Player *player,
    const char *path,
    const TileMap *map
)
{
    FILE *file = fopen(path, "r");

    if (file == NULL)
        return false;

    char version[8];
    float x, y;

    int result = fscanf(
        file,
        "%7s %f %f",
        version,
        &x,
        &y
    );

    fclose(file);

    // データ形式の確認
    if (result != 3 ||
        strcmp(version, "NC1") != 0)
    {
        return false;
    }

    // 座標値の確認
    if (!isfinite(x) || !isfinite(y))
        return false;

    Vector2 loadedPosition = {x, y};

    // 保存位置の当たり判定
    if (!MapCanOccupy(
            map,
            PlayerHitbox(loadedPosition)))
    {
        return false;
    }

    PlayerInit(player);

    player->position = loadedPosition;

    return true;
}

// ===============================
// セーブデータの存在確認
// ===============================

bool PlayerHasSave(
    const char *path,
    const TileMap *map
)
{
    Player temporary;

    return PlayerLoad(
        &temporary,
        path,
        map
    );
}