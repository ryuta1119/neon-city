#include "map.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

// ===============================
// マップ読み込み
// ===============================

bool MapLoad(TileMap *map, const char *path)
{
    FILE *file = fopen(path, "r");

    if (file == NULL)
    {
        TraceLog(LOG_ERROR, "Cannot open map: %s", path);
        return false;
    }

    char line[MAP_COLS + 8];

    for (int y = 0; y < MAP_ROWS; y++)
    {
        if (fgets(line, sizeof(line), file) == NULL)
        {
            TraceLog(LOG_ERROR, "Map has too few rows");
            fclose(file);
            return false;
        }

        size_t length = strcspn(line, "\r\n");

        if (length != MAP_COLS)
        {
            TraceLog(
                LOG_ERROR,
                "Invalid map width at row %d",
                y + 1
            );

            fclose(file);
            return false;
        }

        for (int x = 0; x < MAP_COLS; x++)
        {
            char tile = line[x];

            if (tile != '#' &&
                tile != '.' &&
                tile != '=' &&
                tile != 'B')
            {
                TraceLog(
                    LOG_ERROR,
                    "Invalid tile at (%d, %d)",
                    x, y
                );

                fclose(file);
                return false;
            }

            map->tiles[y][x] = tile;
        }
    }

    fclose(file);

    return true;
}

// ===============================
// マップ描画
// ===============================

void MapDraw(const TileMap *map)
{
    for (int y = 0; y < MAP_ROWS; y++)
    {
        for (int x = 0; x < MAP_COLS; x++)
        {
            char tile = map->tiles[y][x];

            int px = x * TILE_SIZE;
            int py = y * TILE_SIZE;

            Color color;

            switch (tile)
            {
                case '#':
                    color = (Color){10, 15, 28, 255};
                    break;

                case 'B':
                    color = (Color){38, 48, 75, 255};
                    break;

                case '=':
                    color = (Color){30, 34, 48, 255};
                    break;

                default:
                    color = (Color){65, 72, 90, 255};
                    break;
            }

            DrawRectangle(
                px,
                py,
                TILE_SIZE,
                TILE_SIZE,
                color
            );

            // タイルの境界線
            DrawRectangleLines(
                px,
                py,
                TILE_SIZE,
                TILE_SIZE,
                Fade(BLACK, 0.18f)
            );

            // 建物の装飾
            if (tile == 'B')
            {
                DrawRectangle(
                    px + 8,
                    py + 8,
                    TILE_SIZE - 16,
                    TILE_SIZE - 16,
                    (Color){22, 29, 52, 255}
                );

                DrawRectangle(
                    px + 16,
                    py + 16,
                    12,
                    12,
                    (Color){40, 170, 210, 255}
                );

                DrawRectangle(
                    px + 36,
                    py + 16,
                    12,
                    12,
                    (Color){40, 170, 210, 255}
                );
            }

            // 道路の装飾
            if (tile == '=')
            {
                DrawCircle(
                    px + TILE_SIZE / 2,
                    py + TILE_SIZE / 2,
                    2,
                    (Color){100, 110, 130, 255}
                );
            }
        }
    }
}

// ===============================
// 通行可能か判定
// ===============================

bool MapCanOccupy(
    const TileMap *map,
    Rectangle hitbox
)
{
    // マップ外は通行禁止
    if (hitbox.x < 0 ||
        hitbox.y < 0 ||
        hitbox.x + hitbox.width > MAP_WORLD_WIDTH ||
        hitbox.y + hitbox.height > MAP_WORLD_HEIGHT)
    {
        return false;
    }

    int left = (int)floorf(
        hitbox.x / TILE_SIZE
    );

    int right = (int)floorf(
        (hitbox.x + hitbox.width - 0.01f) / TILE_SIZE
    );

    int top = (int)floorf(
        hitbox.y / TILE_SIZE
    );

    int bottom = (int)floorf(
        (hitbox.y + hitbox.height - 0.01f) / TILE_SIZE
    );

    // プレイヤーが重なる全タイルを確認
    for (int y = top; y <= bottom; y++)
    {
        for (int x = left; x <= right; x++)
        {
            char tile = map->tiles[y][x];

            if (tile == '#' || tile == 'B')
            {
                return false;
            }
        }
    }

    return true;
}