#include "shop.h"
#include "raylib.h"
#include <stdio.h>
#include <string.h>

typedef enum {
    SHOP_PULSE,
    SHOP_ION,
    SHOP_GUN_POWER,
    SHOP_FIRE_RATE,
    SHOP_BLADE_POWER,
    SHOP_ARMOR
} ShopItem;

static const char *itemNames[SHOP_ITEM_COUNT] = {
    "PULSE RIFLE", "ION BLADE", "GUN DAMAGE", "FIRE RATE", "BLADE DAMAGE", "MAX HP"
};
static const char *descriptions[SHOP_ITEM_COUNT] = {
    "Primary weapon: 2 base damage", "Melee weapon: 4 base damage",
    "+1 damage per upgrade", "Faster firing per upgrade",
    "+1 melee damage per upgrade", "+1 maximum HP per upgrade"
};

static Rectangle ShopRow(int i)
{
    return (Rectangle){180, 177 + i * 72.0f, 920, 64};
}

static int ShopItemCost(const Economy *e, int item)
{
    switch (item) {
        case SHOP_PULSE: return e->ownsPulseRifle ? 0 : 450;
        case SHOP_ION: return e->ownsIonBlade ? 0 : 350;
        case SHOP_GUN_POWER: return e->gunPowerLevel >= ECONOMY_MAX_LEVEL
                                     ? 0 : 200 + 120 * e->gunPowerLevel;
        case SHOP_FIRE_RATE: return e->fireRateLevel >= ECONOMY_MAX_LEVEL
                                     ? 0 : 200 + 100 * e->fireRateLevel;
        case SHOP_BLADE_POWER: return e->bladePowerLevel >= ECONOMY_MAX_LEVEL
                                     ? 0 : 180 + 100 * e->bladePowerLevel;
        case SHOP_ARMOR: return e->armorLevel >= ECONOMY_MAX_LEVEL
                                     ? 0 : 240 + 130 * e->armorLevel;
        default: return 0;
    }
}

static bool ShopIsMaxed(const Economy *e, int item)
{
    switch (item) {
        case SHOP_GUN_POWER: return e->gunPowerLevel >= ECONOMY_MAX_LEVEL;
        case SHOP_FIRE_RATE: return e->fireRateLevel >= ECONOMY_MAX_LEVEL;
        case SHOP_BLADE_POWER: return e->bladePowerLevel >= ECONOMY_MAX_LEVEL;
        case SHOP_ARMOR: return e->armorLevel >= ECONOMY_MAX_LEVEL;
        default: return false;
    }
}

void ShopInit(Shop *shop)
{
    *shop = (Shop){0};
}

static void ShopMessage(Shop *shop, const char *message)
{
    snprintf(shop->message, sizeof(shop->message), "%s", message);
    shop->messageUntil = GetTime() + 2.2;
}

static bool ShopActivate(Shop *shop, Economy *e)
{
    int item = shop->selected;

    // 購入済み装備は無料で切り替え可能
    if (item == SHOP_PULSE && e->ownsPulseRifle) {
        if (e->equippedGun == GUN_PULSE_RIFLE) {
            e->equippedGun = GUN_SIDEARM;
            ShopMessage(shop, "SIDEARM EQUIPPED");
        } else {
            e->equippedGun = GUN_PULSE_RIFLE;
            ShopMessage(shop, "PULSE RIFLE EQUIPPED");
        }
        return true;
    }
    if (item == SHOP_ION && e->ownsIonBlade) {
        if (e->equippedBlade == BLADE_ION) {
            e->equippedBlade = BLADE_BASIC;
            ShopMessage(shop, "BASIC BLADE EQUIPPED");
        } else {
            e->equippedBlade = BLADE_ION;
            ShopMessage(shop, "ION BLADE EQUIPPED");
        }
        return true;
    }
    if (ShopIsMaxed(e, item)) {
        ShopMessage(shop, "MAXIMUM LEVEL REACHED");
        return false;
    }

    int cost = ShopItemCost(e, item);
    if (cost > e->credits) {
        ShopMessage(shop, "NOT ENOUGH CREDITS");
        return false;
    }
    if (cost <= 0) return false;

    e->credits -= cost;
    switch (item) {
        case SHOP_PULSE: e->ownsPulseRifle = 1; e->equippedGun = GUN_PULSE_RIFLE; break;
        case SHOP_ION: e->ownsIonBlade = 1; e->equippedBlade = BLADE_ION; break;
        case SHOP_GUN_POWER: e->gunPowerLevel++; break;
        case SHOP_FIRE_RATE: e->fireRateLevel++; break;
        case SHOP_BLADE_POWER: e->bladePowerLevel++; break;
        case SHOP_ARMOR: e->armorLevel++; break;
    }
    ShopMessage(shop, "PURCHASE COMPLETE (AUTO-SAVED)");
    return true;
}

bool ShopUpdate(Shop *shop, Economy *economy)
{
    if (IsKeyPressed(KEY_DOWN))
        shop->selected = (shop->selected + 1) % SHOP_ITEM_COUNT;
    if (IsKeyPressed(KEY_UP))
        shop->selected = (shop->selected + SHOP_ITEM_COUNT - 1) % SHOP_ITEM_COUNT;
    if (IsKeyPressed(KEY_ENTER)) return ShopActivate(shop, economy);

    Vector2 mouse = GetMousePosition();
    Vector2 delta = GetMouseDelta();
    for (int i = 0; i < SHOP_ITEM_COUNT; i++) {
        if (!CheckCollisionPointRec(mouse, ShopRow(i))) continue;
        if (delta.x != 0 || delta.y != 0) shop->selected = i;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            shop->selected = i;
            return ShopActivate(shop, economy);
        }
    }
    return false;
}

void ShopDraw(const Shop *shop, const Economy *e)
{
    Color pink = (Color){241, 85, 171, 255};
    Color cyan = (Color){67, 217, 224, 255};
    Color gold = (Color){241, 201, 112, 255};
    Color muted = (Color){158, 175, 197, 255};
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                  (Color){8, 13, 25, 253});
    DrawRectangle(0, 0, GetScreenWidth(), 5, pink);
    DrawText("/ NEON MARKET", 178, 35, 42, WHITE);
    DrawText("HARDWARE  /  LOADOUT  /  UPGRADES", 180, 90, 17, cyan);
    DrawRectangle(180, 137, 920, 2, Fade(pink, 0.35f));
    DrawText(TextFormat("WALLET    %d CR", e->credits), 183, 148, 20, gold);
    DrawText(TextFormat("%s  +  %s", EconomyGunName(e), EconomyBladeName(e)),
             601, 152, 16, muted);

    for (int i = 0; i < SHOP_ITEM_COUNT; i++) {
        Rectangle row = ShopRow(i);
        bool selected = shop->selected == i;
        Color frame = selected ? pink : (Color){60, 82, 105, 255};
        DrawRectangleRec(row, selected ? (Color){34, 31, 58, 255}
                                         : (Color){19, 28, 47, 255});
        DrawRectangleLinesEx(row, selected ? 2.0f : 1.0f, frame);
        if (selected) DrawRectangle((int)row.x, (int)row.y, 5, (int)row.height, pink);
        DrawText(TextFormat("%02d", i + 1), (int)row.x + 16,
                 (int)row.y + 17, 20, selected ? pink : muted);
        DrawText(itemNames[i], (int)row.x + 63, (int)row.y + 7,
                 23, selected ? WHITE : (Color){210, 224, 234, 255});
        DrawText(descriptions[i], (int)row.x + 63, (int)row.y + 37,
                 16, muted);
        char costLabel[44];
        if (i == SHOP_PULSE && e->ownsPulseRifle) {
            snprintf(costLabel, sizeof(costLabel), "%s",
                     e->equippedGun == GUN_PULSE_RIFLE ? "EQUIPPED" : "EQUIP FREE");
        } else if (i == SHOP_ION && e->ownsIonBlade) {
            snprintf(costLabel, sizeof(costLabel), "%s",
                     e->equippedBlade == BLADE_ION ? "EQUIPPED" : "EQUIP FREE");
        } else if (ShopIsMaxed(e, i)) {
            snprintf(costLabel, sizeof(costLabel), "MAX LV");
        } else {
            snprintf(costLabel, sizeof(costLabel), "%d CR", ShopItemCost(e, i));
        }
        DrawText(costLabel, (int)row.x + 732, (int)row.y + 10, 21,
                 selected ? gold : muted);
        if (i >= SHOP_GUN_POWER) {
            int lv = i == SHOP_GUN_POWER ? e->gunPowerLevel :
                     i == SHOP_FIRE_RATE ? e->fireRateLevel :
                     i == SHOP_BLADE_POWER ? e->bladePowerLevel : e->armorLevel;
            for (int j = 0; j < ECONOMY_MAX_LEVEL; j++) {
                DrawRectangle((int)row.x + 736 + j * 39, (int)row.y + 44,
                              31, 7, j < lv ? cyan : (Color){54, 70, 83, 255});
            }
        }
    }
    if (shop->message[0] && GetTime() < shop->messageUntil)
        DrawText(shop->message, 181, 619, 20, cyan);
    DrawRectangle(0, 653, GetScreenWidth(), 1, Fade(pink, 0.35f));
    DrawText("UP / DOWN  SELECT        ENTER / CLICK  BUY / EQUIP        ESC / E  EXIT",
             183, 671, 17, WHITE);
}
