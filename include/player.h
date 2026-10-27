#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include "map.h"

#include <stdbool.h>

#define PLAYER_SIZE 32

typedef struct {
    Vector2 position;
    float speed;
} Player;

void PlayerInit(Player *player);

void PlayerUpdate(
    Player *player,
    float dt,
    const TileMap *map
);

void PlayerDraw(const Player *player);

bool PlayerSave(
    const Player *player,
    const char *path
);

bool PlayerLoad(
    Player *player,
    const char *path,
    const TileMap *map
);

bool PlayerHasSave(
    const char *path,
    const TileMap *map
);

// 指定した移動量だけプレイヤーを動かす
// 壁・障害物との当たり判定も行う
void PlayerMove(
    Player *player,
    Vector2 delta,
    const TileMap *map
);

#endif