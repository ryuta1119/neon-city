#include "hub_terminal.h"

#define TERMINAL_COUNT 2
#define INTERACT_DISTANCE 88.0f

// ===============================
// 施設データ
// ===============================

typedef struct {
    HubTerminalKind kind;
    Vector2 position;
    const char *name;
    const char *icon;
    Color color;
} TerminalData;

// CENTRAL HUB内の施設
static const TerminalData terminals[TERMINAL_COUNT] = {
    {
        HUB_TERMINAL_MISSION,
        {928.0f, 544.0f},
        "MISSION",
        "M",
        {55, 210, 245, 255}
    },
    {
        HUB_TERMINAL_SHOP,
        {1184.0f, 544.0f},
        "SHOP",
        "S",
        {255, 90, 190, 255}
    }
};

// ===============================
// 初期化
// ===============================

void HubTerminalsInit(HubTerminals *hub)
{
    hub->active = HUB_TERMINAL_NONE;
}

// ===============================
// 施設画面の状態
// ===============================

bool HubTerminalsIsOpen(const HubTerminals *hub)
{
    return hub->active != HUB_TERMINAL_NONE;
}

void HubTerminalsClose(HubTerminals *hub)
{
    hub->active = HUB_TERMINAL_NONE;
}

// ===============================
// 近くの施設を検索
// ===============================

static int FindNearbyTerminal(Vector2 position)
{
    float maxDistanceSquared =
        INTERACT_DISTANCE * INTERACT_DISTANCE;

    int nearest = -1;
    float nearestDistance = maxDistanceSquared;

    for (int i = 0; i < TERMINAL_COUNT; i++)
    {
        float dx =
            position.x - terminals[i].position.x;

        float dy =
            position.y - terminals[i].position.y;

        float distanceSquared =
            dx * dx + dy * dy;

        if (distanceSquared <= nearestDistance)
        {
            nearestDistance = distanceSquared;
            nearest = i;
        }
    }

    return nearest;
}

// ===============================
// 施設操作
// ===============================

void HubTerminalsInteract(
    HubTerminals *hub,
    Vector2 playerPosition
)
{
    if (HubTerminalsIsOpen(hub))
        return;

    int index = FindNearbyTerminal(playerPosition);

    if (index < 0)
        return;

    hub->active = terminals[index].kind;
}

// ===============================
// マップ上の施設描画
// ===============================

void HubTerminalsDrawWorld(void)
{
    for (int i = 0; i < TERMINAL_COUNT; i++)
    {
        const TerminalData *terminal = &terminals[i];

        int x = (int)terminal->position.x;
        int y = (int)terminal->position.y;

        // 発光風の背景
        DrawCircle(
            x,
            y,
            42,
            Fade(terminal->color, 0.15f)
        );

        // 端末本体
        DrawRectangle(
            x - 28,
            y - 30,
            56,
            60,
            (Color){12, 20, 38, 255}
        );

        // 外枠
        DrawRectangleLinesEx(
            (Rectangle){
                (float)x - 28,
                (float)y - 30,
                56,
                60
            },
            3,
            terminal->color
        );

        // 画面部分
        DrawRectangle(
            x - 20,
            y - 20,
            40,
            34,
            Fade(terminal->color, 0.25f)
        );

        // アイコン
        DrawText(
            terminal->icon,
            x - 9,
            y - 15,
            30,
            terminal->color
        );

        // ラベル
        int textWidth = MeasureText(
            terminal->name,
            16
        );

        DrawText(
            terminal->name,
            x - textWidth / 2,
            y - 58,
            16,
            WHITE
        );
    }
}

// ===============================
// 施設専用画面
// ===============================

static void DrawTerminalScreen(HubTerminalKind kind)
{
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    // 背景を暗くする
    DrawRectangle(
        0,
        0,
        screenWidth,
        screenHeight,
        Fade(BLACK, 0.82f)
    );

    Rectangle panel = {
        screenWidth / 2.0f - 380,
        screenHeight / 2.0f - 220,
        760,
        440
    };

    DrawRectangleRec(
        panel,
        (Color){15, 23, 43, 255}
    );

    Color accent =
        kind == HUB_TERMINAL_MISSION
            ? SKYBLUE
            : (Color){255, 90, 190, 255};

    DrawRectangleLinesEx(
        panel,
        3,
        accent
    );

    // 上部アクセント
    DrawRectangle(
        (int)panel.x,
        (int)panel.y,
        (int)panel.width,
        8,
        accent
    );

    int left = (int)panel.x + 50;
    int top = (int)panel.y + 50;

    if (kind == HUB_TERMINAL_MISSION)
    {
        DrawText(
            "MISSION TERMINAL",
            left,
            top,
            36,
            SKYBLUE
        );

        DrawText(
            "MISSION MANAGEMENT SYSTEM",
            left,
            top + 65,
            20,
            LIGHTGRAY
        );

        DrawLine(
            left,
            top + 110,
            left + 650,
            top + 110,
            SKYBLUE
        );

        DrawText(
            "Available mission types:",
            left,
            top + 145,
            22,
            WHITE
        );

        DrawText(
            "ELIMINATION / SURVIVAL / RETRIEVAL",
            left,
            top + 185,
            20,
            LIGHTGRAY
        );

        DrawText(
    "ENTER : START TRAINING",
    left,
    top + 250,
    20,
    GREEN
);
    }
    else if (kind == HUB_TERMINAL_SHOP)
    {
        DrawText(
            "SHOP TERMINAL",
            left,
            top,
            36,
            accent
        );

        DrawText(
            "WEAPON & EQUIPMENT SYSTEM",
            left,
            top + 65,
            20,
            LIGHTGRAY
        );

        DrawLine(
            left,
            top + 110,
            left + 650,
            top + 110,
            accent
        );

        DrawText(
            "Available categories:",
            left,
            top + 145,
            22,
            WHITE
        );

        DrawText(
            "WEAPONS / EQUIPMENT / UPGRADES",
            left,
            top + 185,
            20,
            LIGHTGRAY
        );

        DrawText(
            "Trading system: COMING SOON",
            left,
            top + 250,
            20,
            GRAY
        );
    }

    DrawText(
        "ESC / E : CLOSE",
        left,
        top + 340,
        22,
        WHITE
    );
}

// ===============================
// UI描画
// ===============================

void HubTerminalsDrawUI(
    const HubTerminals *hub,
    Vector2 playerPosition
)
{
    // 施設画面を開いている場合
    if (HubTerminalsIsOpen(hub))
    {
        DrawTerminalScreen(hub->active);
        return;
    }

    // 近くの施設を調べる
    int index = FindNearbyTerminal(playerPosition);

    if (index < 0)
        return;

    const TerminalData *terminal = &terminals[index];

    const char *message = TextFormat(
        "[E] INTERACT : %s",
        terminal->name
    );

    int fontSize = 24;
    int textWidth = MeasureText(message, fontSize);

    int x = GetScreenWidth() / 2 - textWidth / 2;
    int y = GetScreenHeight() - 90;

    DrawRectangle(
        x - 20,
        y - 12,
        textWidth + 40,
        52,
        Fade(BLACK, 0.85f)
    );

    DrawText(
        message,
        x,
        y,
        fontSize,
        terminal->color
    );
}