// SPDX-License-Identifier: Zlib
//
// Copyright (c) 2026 Adrian "asie" Siekierka

#ifdef MOONSHL2

#include <string.h>
#include "moonshl2.h"
#include "console.h"
#include "ff.h"

extern void checkErrorFatFs(const char *msg, int result);

char executable_path[MOONSHL2_EXTLINK_STRING_LEN+1];

void moonshl2_init(FATFS *fs) {
    FIL fp;
    FRESULT fres;
    uint32_t id;
    unsigned int br;

    // Set default path to reload.dat.
    memcpy(executable_path, MOONSHL2_RELOAD_PATH, sizeof(MOONSHL2_RELOAD_PATH));

    // Open extlink.dat.
    fres = f_open(&fp, MOONSHL2_EXTLINK_PATH, FA_READ | FA_WRITE | FA_OPEN_EXISTING);

    if (fres == FR_NO_PATH || fres == FR_NO_FILE) {
        // No extlink file found; use default path.
    } else if (fres != FR_OK) {
        // Error loading extlink file.
        checkErrorFatFs("Could not open " MOONSHL2_EXTLINK_PATH, fres);
    } else {
        // Extlink file found; use its path.
        checkErrorFatFs("Could not read " MOONSHL2_EXTLINK_PATH,
            f_read(&fp, &id, sizeof(id), &br));
        
        if (id != MOONSHL2_EXTLINK_ID || f_size(&fp) < MOONSHL2_EXTLINK_ASCII_OFFSET(MOONSHL2_EXTLINK_STRING_COUNT)) {
            // Invalid extlink file; use default path.
        } else {
            f_lseek(&fp, MOONSHL2_EXTLINK_FILE_PATH_ASCII_OFFSET);
            checkErrorFatFs("Could not read " MOONSHL2_EXTLINK_PATH,
                f_read(&fp, executable_path, MOONSHL2_EXTLINK_STRING_LEN, &br));
            executable_path[MOONSHL2_EXTLINK_STRING_LEN] = 0;
        }

        // Invalidate extlink file after use.
        id = 0;
        f_lseek(&fp, 0);
        checkErrorFatFs("Could not write to " MOONSHL2_EXTLINK_PATH,
            f_write(&fp, &id, sizeof(id), &br));

        f_close(&fp);
    }

    dprintf("Target: %s\n", executable_path);
}

#endif /* MOONSHL2 */