#ifndef NEON_MAP_H
#define NEON_MAP_H

#include "raylib.h"
#include <stdbool.h>

#define TILE_SIZE 64
#define MAP_COLS 32
#define MAP_ROWS 20

#define MAP_WORLD_WIDTH  (MAP_COLS * TILE_SIZE)
#define MAP_WORLD_HEIGHT (MAP_ROWS * TILE_SIZE)

typedef struct {
    char tiles[MAP_ROWS][MAP_COLS];
} TileMap;

// マップの読み込み
bool MapLoad(TileMap *map, const char *path);

// マップの描画
void MapDraw(const TileMap *map);

// 指定範囲が通行可能か判定
bool MapCanOccupy(const TileMap *map, Rectangle hitbox);

#endif