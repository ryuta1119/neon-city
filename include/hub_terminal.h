#ifndef NEON_HUB_TERMINAL_H
#define NEON_HUB_TERMINAL_H

#include "raylib.h"
#include <stdbool.h>

// 施設の種類
typedef enum {
    HUB_TERMINAL_NONE,
    HUB_TERMINAL_MISSION,
    HUB_TERMINAL_SHOP
} HubTerminalKind;

// 現在の施設の状態
typedef struct {
    HubTerminalKind active;
} HubTerminals;

// 初期化
void HubTerminalsInit(HubTerminals *hub);

// 施設画面が開いているか
bool HubTerminalsIsOpen(const HubTerminals *hub);

// 施設画面を閉じる
void HubTerminalsClose(HubTerminals *hub);

// プレイヤー位置から操作対象を探す
void HubTerminalsInteract(
    HubTerminals *hub,
    Vector2 playerPosition
);

// マップ上に施設を描画
void HubTerminalsDrawWorld(void);

// 操作案内・施設画面を描画
void HubTerminalsDrawUI(
    const HubTerminals *hub,
    Vector2 playerPosition
);

#endif