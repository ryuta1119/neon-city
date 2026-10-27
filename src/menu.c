#include "neon_menu.h"

static const char *menuItems[] = {
    "NEW GAME",
    "CONTINUE",
    "SETTINGS",
    "EXIT"
};

static float ClampVolume(float value)
{
    if (value < 0.0f) return 0.0f;
    if (value > 1.0f) return 1.0f;
    return value;
}

void MenuInit(Menu *menu, bool hasSave)
{
    menu->selected = 0;
    menu->settingsOpen = false;
    menu->hasSave = hasSave;
    menu->musicVolume = 0.65f;
}

static MenuAction ActivateMenuItem(Menu *menu, int item)
{
    switch (item)
    {
        case 0:
            return MENU_NEW_GAME;

        case 1:
            if (menu->hasSave)
                return MENU_CONTINUE;
            break;

        case 2:
            menu->settingsOpen = true;
            break;

        case 3:
            return MENU_EXIT;
    }

    return MENU_NONE;
}

MenuAction MenuUpdate(Menu *menu)
{
    if (menu->settingsOpen)
    {
        if (IsKeyPressed(KEY_ESCAPE))
            menu->settingsOpen = false;

        if (IsKeyPressed(KEY_LEFT))
            menu->musicVolume -= 0.05f;

        if (IsKeyPressed(KEY_RIGHT))
            menu->musicVolume += 0.05f;

        // マウスによる音量調整
        Rectangle slider = {380, 340, 520, 56};

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
            CheckCollisionPointRec(
                GetMousePosition(), slider))
        {
            float x = GetMouseX();

            menu->musicVolume =
                (x - 400.0f) / 480.0f;
        }

        menu->musicVolume =
            ClampVolume(menu->musicVolume);

        return MENU_NONE;
    }

    if (IsKeyPressed(KEY_DOWN))
        menu->selected = (menu->selected + 1) % 4;

    if (IsKeyPressed(KEY_UP))
        menu->selected = (menu->selected + 3) % 4;

    if (IsKeyPressed(KEY_ENTER))
        return ActivateMenuItem(menu, menu->selected);

    // マウス操作
    for (int i = 0; i < 4; i++)
    {
        Rectangle button = {
            440,
            300 + i * 65,
            400,
            54
        };

        if (CheckCollisionPointRec(
                GetMousePosition(), button))
        {
            Vector2 delta = GetMouseDelta();

            if (delta.x != 0 || delta.y != 0)
                menu->selected = i;

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                menu->selected = i;
                return ActivateMenuItem(menu, i);
            }
        }
    }

    return MENU_NONE;
}

void MenuDraw(const Menu *menu)
{
    ClearBackground((Color){12, 15, 30, 255});

    // 背景グリッド
    for (int x = 0; x < 1280; x += 64)
    {
        DrawLine(
            x, 0, x, 720,
            (Color){25, 35, 60, 255}
        );
    }

    for (int y = 0; y < 720; y += 64)
    {
        DrawLine(
            0, y, 1280, y,
            (Color){25, 35, 60, 255}
        );
    }

    if (menu->settingsOpen)
    {
        DrawText(
            "SETTINGS",
            465, 180, 60, SKYBLUE
        );

        DrawText(
            "BGM VOLUME",
            400, 295, 25, WHITE
        );

        DrawRectangle(
            400, 360, 480, 16,
            DARKGRAY
        );

        DrawRectangle(
            400, 360,
            (int)(480 * menu->musicVolume),
            16,
            SKYBLUE
        );

        DrawText(
            TextFormat(
                "%d%%",
                (int)(menu->musicVolume * 100)
            ),
            890, 350, 25, WHITE
        );

        DrawText(
            "LEFT / RIGHT : ADJUST",
            450, 440, 20, LIGHTGRAY
        );

        DrawText(
            "ESC : BACK",
            535, 490, 20, LIGHTGRAY
        );

        return;
    }

    DrawText(
        "NEON CITY",
        420, 155, 70, SKYBLUE
    );

    DrawText(
        "A CYBERPUNK ACTION RPG",
        465, 245, 20, LIGHTGRAY
    );

    for (int i = 0; i < 4; i++)
    {
        Rectangle button = {
            440,
            300 + i * 65,
            400,
            54
        };

        bool selected = (menu->selected == i);

        Color border = selected ? SKYBLUE : DARKGRAY;

        DrawRectangleRec(
            button,
            (Color){20, 28, 48, 255}
        );

        DrawRectangleLinesEx(
            button,
            2,
            border
        );

        Color textColor = selected ? SKYBLUE : WHITE;

        if (i == 1 && !menu->hasSave)
            textColor = GRAY;

        DrawText(
            menuItems[i],
            (int)button.x + 35,
            (int)button.y + 16,
            22,
            textColor
        );
    }

    DrawText(
        "BGM : BASS OVER",
        30, 670, 20, GRAY
    );
}