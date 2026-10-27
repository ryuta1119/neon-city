#ifndef NEON_CITY_VISUALS_H
#define NEON_CITY_VISUALS_H

#include "map.h"

// Existing tile data and collision rules are unchanged.
void CityDrawMap(const TileMap *map, bool arena, float timeSeconds);
void CityDrawRain(float timeSeconds, bool arena);

#endif
