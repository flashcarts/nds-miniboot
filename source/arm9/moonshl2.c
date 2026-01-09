// SPDX-License-Identifier: Zlib
//
// Copyright (c) 2026 Adrian "asie" Siekierka

#ifdef MOONSHL2

#include <string.h>
#include "moonshl2.h"
#include "console.h"
#include "ff.h"
#include "dka.h"

extern void checkErrorFatFs(const char *msg, int result);

char executable_path[MOONSHL2_EXTLINK_STRING_LEN+1];
static char arg_path[MOONSHL2_EXTLINK_STRING_LEN+1];

int tolower(int c) {
    return (c >= 'A' && c <= 'Z') ? (c + 32) : c;
}

void moonshl2_init(FATFS *fs) {
    FIL fp;
    FRESULT fres;
    uint32_t id;
    unsigned int br;

    // Set default path to reload.dat.
    memcpy(executable_path, MOONSHL2_RELOAD_PATH, sizeof(MOONSHL2_RELOAD_PATH));
    arg_path[0] = 0;

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

            // Create ARGV.
            int exe_path_len = strlen(executable_path);

            if (exe_path_len > 4 && !(
                executable_path[exe_path_len - 4] == '.'
                && tolower(executable_path[exe_path_len - 3]) == 'n'
                && tolower(executable_path[exe_path_len - 2]) == 'd'
                && tolower(executable_path[exe_path_len - 1]) == 's')) {

                // File to execute is not a .nds; redirect to loader with .nds replaced with .dat.
                memcpy(arg_path, executable_path, exe_path_len + 1);

                f_lseek(&fp, MOONSHL2_EXTLINK_BOOT_PATH_ASCII_OFFSET);
                checkErrorFatFs("Could not read " MOONSHL2_EXTLINK_PATH,
                    f_read(&fp, executable_path, MOONSHL2_EXTLINK_STRING_LEN, &br));

                exe_path_len = strlen(executable_path);
                
                if (exe_path_len <= 4) {
                    eprintf("extlink boot error\n"); while(1);
                }

                executable_path[exe_path_len - 3] = 'd';
                executable_path[exe_path_len - 2] = 'a';
                executable_path[exe_path_len - 1] = 't';
            }

            // Invalidate extlink file after use.
            id = 0;
            f_lseek(&fp, 0);
            checkErrorFatFs("Could not write to " MOONSHL2_EXTLINK_PATH,
                f_write(&fp, &id, sizeof(id), &br));
        }

        f_close(&fp);
    }

    dprintf("Target: %s\n", executable_path);
}

void moonshl2_build_argv(void) {
    int exe_path_len = strlen(executable_path);
    int arg_path_len = strlen(arg_path);
    if (!arg_path_len)
        arg_path_len = -1;

    DKA_ARGV->cmdline = (char*) (0x2FFE000 - MOONSHL2_EXTLINK_STRING_LEN * 2 - 4);
    DKA_ARGV->cmdline_size = exe_path_len + arg_path_len + 2;
    __aeabi_memcpy(DKA_ARGV->cmdline, executable_path, exe_path_len + 1);
    if (arg_path_len)
        __aeabi_memcpy(DKA_ARGV->cmdline + exe_path_len + 1, arg_path, arg_path_len + 1);
    DKA_ARGV->magic = DKA_ARGV_MAGIC;
}

#endif /* MOONSHL2 */