#include "main.h"

#include "settings.h"
#include "theme.h"

#include "native/filesys.h"
#include "native/main.h"
#include "native/thread.h"

#include "av/utox_av.h"

#include <stdlib.h>
#include <string.h>

/* The utox_ functions contained in src/main.c are wrappers for the platform native_ functions
 * if you need to localize them to a specific platform, move them from here, to each
 * src/<platform>/main.x and change from utox_ to native_
 */

bool utox_data_save_tox(uint8_t *data, size_t length) {
    FILE *fp = utox_get_file("tox_save.tox", NULL, UTOX_FILE_OPTS_WRITE);
    if (!fp) {
        return true;
    }

    if (fwrite(data, length, 1, fp) != 1) {
        fclose(fp);
        return true;
    }

    flush_file(fp);
    fclose(fp);

    return false;
}

uint8_t *utox_data_load_tox(size_t *size) {
    const char name[][20] = { "tox_save.tox", "tox_save.tox.atomic", "tox_save.tmp", "tox_save" };

    for (uint8_t i = 0; i < 4; i++) {
        size_t length = 0;

        FILE *fp = utox_get_file(name[i], &length, UTOX_FILE_OPTS_READ);
        if (!fp) {
            continue;
        }

        uint8_t *data = calloc(1, length + 1);
        if (fread(data, length, 1, fp) != 1) {
            fclose(fp);
            free(data);
            // Return NULL, because if a Tox save exits we don't want to fall
            // back to an old version, we need the user to decide what to do.
            return NULL;
        }

        fclose(fp);
        *size = length;
        return data;
    }

    return NULL;
}

bool utox_data_save_ftinfo(char hex[TOX_PUBLIC_KEY_SIZE * 2], uint8_t *data, size_t length) {
    char name[TOX_PUBLIC_KEY_SIZE * 2 + sizeof(".ftinfo")];
    snprintf(name, sizeof(name), "%.*s.ftinfo", TOX_PUBLIC_KEY_SIZE * 2, hex);

    FILE *fp = utox_get_file(name, NULL, UTOX_FILE_OPTS_WRITE);

    if (!fp) {
        return false;
    }

    if (fwrite(data, length, 1, fp) != 1) {
        fclose(fp);
        return false;
    }

    fclose(fp);

    return true;
}

void utox_init(void) {
    UTOX_SAVE *save = config_load();
    free(save);

    thread(utox_av_ctrl_thread, NULL);
}
