#ifndef NEON_MENU_H
#define NEON_MENU_H

#include "raylib.h"
#include <stdbool.h>

typedef enum {
    MENU_NONE,
    MENU_NEW_GAME,
    MENU_CONTINUE,
    MENU_EXIT
} MenuAction;

typedef struct {
    int selected;
    bool settingsOpen;
    bool hasSave;
    float musicVolume;
} Menu;

void MenuInit(Menu *menu, bool hasSave);

MenuAction MenuUpdate(Menu *menu);

void MenuDraw(const Menu *menu);

#endif