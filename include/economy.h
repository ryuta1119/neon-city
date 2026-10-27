#ifndef NEON_ECONOMY_H
#define NEON_ECONOMY_H

#include <stdbool.h>
#include "combat.h"

#define ECONOMY_MAX_LEVEL 3
#define ECONOMY_MAX_CREDITS 9999999

typedef enum { GUN_SIDEARM = 0, GUN_PULSE_RIFLE = 1 } GunType;
typedef enum { BLADE_BASIC = 0, BLADE_ION = 1 } BladeType;

typedef struct {
    int credits;
    int completedMissions;
    int ownsPulseRifle;
    int ownsIonBlade;
    int equippedGun;
    int equippedBlade;
    int gunPowerLevel;
    int fireRateLevel;
    int bladePowerLevel;
    int armorLevel;
} Economy;

void EconomyInit(Economy *economy);
bool EconomyIsValid(const Economy *economy);
void EconomyEarn(Economy *economy, int reward);
void EconomyApplyToCombat(const Economy *economy, CombatSystem *combat);
const char *EconomyGunName(const Economy *economy);
const char *EconomyBladeName(const Economy *economy);

#endif
