#include "savegame.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

static bool SavedPositionValid(const TileMap *map, float x, float y)
{
    if (!isfinite(x) || !isfinite(y)) return false;
    return MapCanOccupy(map, (Rectangle){
        x - PLAYER_SIZE / 2.0f, y - PLAYER_SIZE / 2.0f,
        PLAYER_SIZE, PLAYER_SIZE
    });
}

static bool TrailingWhitespaceOnly(const char *text)
{
    while (*text) {
        if (!isspace((unsigned char)*text)) return false;
        text++;
    }
    return true;
}

bool SaveGameLoad(Player *player, Economy *economy,
                  const TileMap *map, const char *path)
{
    FILE *file = fopen(path, "r");
    if (!file) return false;

    char line[512] = {0};
    if (!fgets(line, sizeof(line), file)) {
        fclose(file);
        return false;
    }
    // 途中で切れた行や複数行のファイルは受け付けない
    if (!strchr(line, '\n') && !feof(file)) {
        fclose(file);
        return false;
    }
    int next = fgetc(file);
    fclose(file);
    if (next != EOF) return false;

    float x = 0, y = 0;
    int n = 0;
    Economy loaded;
    EconomyInit(&loaded);

    if (strncmp(line, "NC1 ", 4) == 0) {
        if (sscanf(line, "NC1 %f %f %n", &x, &y, &n) != 2 || n == 0 ||
            !TrailingWhitespaceOnly(line + n)) return false;
    } else if (strncmp(line, "NC2 ", 4) == 0) {
        int read = sscanf(line,
            "NC2 %f %f %d %d %d %d %d %d %d %d %d %d %n",
            &x, &y, &loaded.credits, &loaded.completedMissions,
            &loaded.ownsPulseRifle, &loaded.ownsIonBlade,
            &loaded.equippedGun, &loaded.equippedBlade,
            &loaded.gunPowerLevel, &loaded.fireRateLevel,
            &loaded.bladePowerLevel, &loaded.armorLevel, &n);
        if (read != 12 || n == 0 || !TrailingWhitespaceOnly(line + n) ||
            !EconomyIsValid(&loaded)) return false;
    } else {
        return false;
    }

    if (!SavedPositionValid(map, x, y)) return false;

    // ロードが完全に成功した場合にのみ、呼び出し元の状態を変更する
    Player newPlayer;
    PlayerInit(&newPlayer);
    newPlayer.position = (Vector2){x, y};
    *player = newPlayer;
    *economy = loaded;
    return true;
}

bool SaveGameExists(const TileMap *map, const char *path)
{
    Player temporaryPlayer;
    Economy temporaryEconomy;
    return SaveGameLoad(&temporaryPlayer, &temporaryEconomy, map, path);
}

bool SaveGameWrite(const Player *player, const Economy *economy,
                   const char *path)
{
    if (!EconomyIsValid(economy)) return false;
    char temporaryPath[1024];
    int nameSize = snprintf(temporaryPath, sizeof(temporaryPath), "%s.tmp", path);
    if (nameSize < 0 || (size_t)nameSize >= sizeof(temporaryPath)) return false;

    FILE *file = fopen(temporaryPath, "w");
    if (!file) return false;

    int written = fprintf(file,
        "NC2 %.6f %.6f %d %d %d %d %d %d %d %d %d %d\n",
        player->position.x, player->position.y,
        economy->credits, economy->completedMissions,
        economy->ownsPulseRifle, economy->ownsIonBlade,
        economy->equippedGun, economy->equippedBlade,
        economy->gunPowerLevel, economy->fireRateLevel,
        economy->bladePowerLevel, economy->armorLevel);
    int closeResult = fclose(file);
    if (written < 0 || closeResult != 0) {
        remove(temporaryPath);
        return false;
    }
    // 同じディレクトリで置換し、途中まで書いたセーブの公開を避ける
    if (rename(temporaryPath, path) != 0) {
        remove(temporaryPath);
        return false;
    }
#ifdef __EMSCRIPTEN__
    return EM_ASM_INT({
        try {
            localStorage.setItem('neon-city-save-v1', FS.readFile(UTF8ToString($0), {encoding: 'utf8'}));
            return 1;
        } catch (error) { console.warn('Save persistence unavailable', error); return 0; }
    }, path) != 0;
#else
    return true;
#endif
}
