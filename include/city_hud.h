#ifndef NEON_CITY_HUD_H
#define NEON_CITY_HUD_H

#include "raylib.h"
#include "player.h"
#include "map.h"
#include "combat.h"
#include "mission.h"
#include "economy.h"

void CityHUDDrawHub(const Economy *economy, const Player *player,
                    const TileMap *map, bool showSave, bool saveSuccess);
void CityHUDDrawArena(const CombatSystem *combat, const MissionSystem *mission,
                      const Economy *economy, const Player *player,
                      const TileMap *map);
void CityHUDDrawCrosshair(void);

#endif
