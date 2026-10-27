#ifndef NEON_SAVEGAME_H
#define NEON_SAVEGAME_H

#include "player.h"
#include "economy.h"

// 旧NC1形式を読み込み、初期装備/300CRを補完する。
// NC2形式は座標と経済・装備データをすべて保存する。
bool SaveGameLoad(Player *player, Economy *economy,
                  const TileMap *map, const char *path);
bool SaveGameExists(const TileMap *map, const char *path);
bool SaveGameWrite(const Player *player, const Economy *economy,
                   const char *path);

#endif
