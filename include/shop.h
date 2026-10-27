#ifndef NEON_SHOP_H
#define NEON_SHOP_H

#include "economy.h"

#define SHOP_ITEM_COUNT 6

typedef struct {
    int selected;
    char message[96];
    double messageUntil;
} Shop;

void ShopInit(Shop *shop);
// 購入または装備変更があったときtrue（自動セーブ用）
bool ShopUpdate(Shop *shop, Economy *economy);
void ShopDraw(const Shop *shop, const Economy *economy);

#endif
