#include "economy.h"

void EconomyInit(Economy *economy)
{
    *economy = (Economy){0};
    economy->credits = 300;
    economy->equippedGun = GUN_SIDEARM;
    economy->equippedBlade = BLADE_BASIC;
}

bool EconomyIsValid(const Economy *e)
{
    if (e->credits < 0 || e->credits > ECONOMY_MAX_CREDITS ||
        e->completedMissions < 0 || e->completedMissions > ECONOMY_MAX_CREDITS)
        return false;
    if (e->ownsPulseRifle < 0 || e->ownsPulseRifle > 1 ||
        e->ownsIonBlade < 0 || e->ownsIonBlade > 1)
        return false;
    if (e->equippedGun < GUN_SIDEARM || e->equippedGun > GUN_PULSE_RIFLE ||
        e->equippedBlade < BLADE_BASIC || e->equippedBlade > BLADE_ION)
        return false;
    if (e->equippedGun == GUN_PULSE_RIFLE && !e->ownsPulseRifle)
        return false;
    if (e->equippedBlade == BLADE_ION && !e->ownsIonBlade)
        return false;
    if (e->gunPowerLevel < 0 || e->gunPowerLevel > ECONOMY_MAX_LEVEL ||
        e->fireRateLevel < 0 || e->fireRateLevel > ECONOMY_MAX_LEVEL ||
        e->bladePowerLevel < 0 || e->bladePowerLevel > ECONOMY_MAX_LEVEL ||
        e->armorLevel < 0 || e->armorLevel > ECONOMY_MAX_LEVEL)
        return false;
    return true;
}

void EconomyEarn(Economy *e, int reward)
{
    if (reward > 0) {
        if (reward > ECONOMY_MAX_CREDITS - e->credits)
            e->credits = ECONOMY_MAX_CREDITS;
        else
            e->credits += reward;
    }
    if (e->completedMissions < ECONOMY_MAX_CREDITS)
        e->completedMissions++;
}

void EconomyApplyToCombat(const Economy *e, CombatSystem *combat)
{
    combat->gunDamage = (e->equippedGun == GUN_PULSE_RIFLE ? 2 : 1)
                        + e->gunPowerLevel;
    combat->meleeDamage = (e->equippedBlade == BLADE_ION ? 4 : 2)
                          + e->bladePowerLevel;
    combat->fireInterval = (e->equippedGun == GUN_PULSE_RIFLE ? 0.18f : 0.14f)
                           - 0.018f * e->fireRateLevel;
    combat->playerMaxHP = 5 + e->armorLevel;
    combat->playerHP = combat->playerMaxHP;
}

const char *EconomyGunName(const Economy *e)
{
    return e->equippedGun == GUN_PULSE_RIFLE ? "PULSE RIFLE" : "SIDEARM";
}

const char *EconomyBladeName(const Economy *e)
{
    return e->equippedBlade == BLADE_ION ? "ION BLADE" : "BASIC BLADE";
}
