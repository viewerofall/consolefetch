#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Global consoles
PrintConsole topScreen, bottomScreen;

// ═══════════════════════════════════════════════════════════════════════════════
// HARDWARE SPECS TABLE
// Static data — no point querying what we already know per model
// ═══════════════════════════════════════════════════════════════════════════════

typedef struct {
    const char *cpu;
    const char *gpu;
    const char *ram;
    const char *top_res;
    const char *bot_res;
    const char *top_size;   // inches
    const char *sound;
    bool        has_3d;
    bool        has_nfc;
} HWSpec;

// Indexed by Model3DS enum value (0-5)
static const HWSpec hw_specs[] = {
    // MODEL_3DS
    { "ARM11 2x268MHz", "DMP PICA200", "128MB FCRAM",
        "800x240 (3D)", "320x240", "3.53\"", "Stereo", true,  false },
        // MODEL_3DSXL
        { "ARM11 2x268MHz", "DMP PICA200", "128MB FCRAM",
            "800x240 (3D)", "320x240", "4.88\"", "Stereo", true,  false },
            // MODEL_2DS
            { "ARM11 2x268MHz", "DMP PICA200", "128MB FCRAM",
                "800x240",      "320x240", "3.53\"", "Mono",   false, false },
                // MODEL_N3DS
                { "ARM11 4x804MHz", "DMP PICA200", "256MB FCRAM",
                    "800x240 (3D)", "320x240", "3.53\"", "Stereo", true,  true  },
                    // MODEL_N3DSXL
                    { "ARM11 4x804MHz", "DMP PICA200", "256MB FCRAM",
                        "800x240 (3D)", "320x240", "4.88\"", "Stereo", true,  true  },
                        // MODEL_N2DSXL
                        { "ARM11 4x804MHz", "DMP PICA200", "256MB FCRAM",
                            "800x240",      "320x240", "4.88\"", "Stereo", false, true  },
};

// ═══════════════════════════════════════════════════════════════════════════════
// SYSINFO
// ═══════════════════════════════════════════════════════════════════════════════

static void sysinfo_get(SysInfo *s) {
    memset(s, 0, sizeof(SysInfo));

    // ── Model ────────────────────────────────────────────────────────────────
    u8 model = 0;
    CFGU_GetSystemModel(&model);
    s->model      = (Model3DS)model;
    s->is_new_3ds = (model == MODEL_N3DS || model == MODEL_N3DSXL || model == MODEL_N2DSXL);
    s->is_xl      = (model == MODEL_3DSXL || model == MODEL_N3DSXL);

    switch (s->model) {
        case MODEL_3DS:    snprintf(s->model_name, sizeof(s->model_name), "Nintendo 3DS");        break;
        case MODEL_3DSXL:  snprintf(s->model_name, sizeof(s->model_name), "Nintendo 3DS XL");     break;
        case MODEL_2DS:    snprintf(s->model_name, sizeof(s->model_name), "Nintendo 2DS");         break;
        case MODEL_N3DS:   snprintf(s->model_name, sizeof(s->model_name), "New Nintendo 3DS");     break;
        case MODEL_N3DSXL: snprintf(s->model_name, sizeof(s->model_name), "New Nintendo 3DS XL");  break;
        case MODEL_N2DSXL: snprintf(s->model_name, sizeof(s->model_name), "New Nintendo 2DS XL");  break;
        default:           snprintf(s->model_name, sizeof(s->model_name), "Unknown 3DS");          break;
    }

    // ── Firmware ─────────────────────────────────────────────────────────────
    u32 version = osGetKernelVersion();
    int ver_major = (version >> 24) & 0xFF;
    int ver_minor = (version >> 16) & 0xFF;
    int ver_rev   = (version >>  8) & 0xFF;

    u8 region = 0;
    CFGU_SecureInfoGetRegion(&region);
    const char region_char[] = {'J','U','E','C','K','T'};
    char rc = (region < 6) ? region_char[region] : '?';
    snprintf(s->firmware, sizeof(s->firmware), "%d.%d.0-%d%c",
             ver_major, ver_minor, ver_rev, rc);

    // ── Region ───────────────────────────────────────────────────────────────
    const char *regions[] = {"JPN","USA","EUR","CHN","KOR","TWN"};
    snprintf(s->region, sizeof(s->region), "%s",
             (region < 6) ? regions[region] : "UNK");

    // ── Battery ──────────────────────────────────────────────────────────────
    // PTMU returns 0-5 bars. Map to sane percentages.
    u8 bat = 0;
    PTMU_GetBatteryLevel(&bat);
    const int bat_map[] = {0, 5, 20, 40, 60, 100};
    s->battery_pct = (bat <= 5) ? bat_map[bat] : 100;

    u8 charging = 0;
    PTMU_GetBatteryChargeState(&charging);
    s->is_charging = (charging != 0);

    // ── Storage ──────────────────────────────────────────────────────────────
    FS_Archive sdmcArchive;
    if (R_SUCCEEDED(FSUSER_OpenArchive(&sdmcArchive, ARCHIVE_SDMC,
        fsMakePath(PATH_EMPTY, "")))) {
        FS_ArchiveResource res;
    if (R_SUCCEEDED(FSUSER_GetArchiveResource(&res, SYSTEM_MEDIATYPE_SD))) {
        s->sd_total = (uint64_t)res.clusterSize * res.totalClusters;
        s->sd_free  = (uint64_t)res.clusterSize * res.freeClusters;
    }
    FSUSER_CloseArchive(sdmcArchive);
        }

        FS_ArchiveResource nand_res;
        if (R_SUCCEEDED(FSUSER_GetArchiveResource(&nand_res, SYSTEM_MEDIATYPE_CTR_NAND))) {
            s->nand_total = (uint64_t)nand_res.clusterSize * nand_res.totalClusters;
            s->nand_free  = (uint64_t)nand_res.clusterSize * nand_res.freeClusters;
        }
}

// ═══════════════════════════════════════════════════════════════════════════════
// ASCII ART
// Top screen console: exactly 50 cols x 30 rows.
// Art zone:  cols 1-23  (23 chars wide, NO trailing spaces past col 23)
// Info zone: cols 25-50 (25 chars wide)
// Gap at col 24 acts as separator.
// All art lines are padded to exactly 23 chars with spaces.
// ═══════════════════════════════════════════════════════════════════════════════

//                              1234567890123456789012 3
#define ART_W 22   // art content width (col 1 to 22, col 23 = space gap)

static const char *art_3ds[] = {
    " .-------------------. ",
    " |                   | ",
    " |  ~ top screen ~   | ",
    " |                   | ",
    " |                   | ",
    " |___________________|",
    " |o .-----------.    |",
    " |  |           | /\\ |",
    " |+ |           |(  )|",
    " |  |           | \\/ |",
    " |  '-----------'    |",
    " |[sel]  (h)  [sta]  |",
    " '-------------------'",
    NULL
};
static const int art_3ds_lines = 13;

static const char *art_2ds[] = {
    " .-------------------. ",
    " |  ~ top screen ~   | ",
    " |                   | ",
    " |-------------------| ",
    " |  ~ btm screen ~   | ",
    " |-------------------| ",
    " |o .-----------.    |",
    " |  |           | /\\ |",
    " |+ |           |(  )|",
    " |  |           | \\/ |",
    " |  '-----------'    |",
    " |[sel]  (h)  [sta]  |",
    " '-------------------'",
    NULL
};
static const int art_2ds_lines = 13;

static const char *art_n3ds[] = {
    "[ZL].-------------[ZR] ",
    " |  |             | | ",
    " |  |~ top screen~| | ",
    " |  |             | | ",
    " |  |             | | ",
    " |__|_____________|_| ",
    " |o .-----------.  /\\ |",
    " |  |           | (  )|",
    " |+ |           |* \\/ |",
    " |  |           |     |",
    " |  '-----------'     |",
    " |[sel]   (h)  [sta]  |",
    " '---------------------",
    NULL
};
static const int art_n3ds_lines = 13;

static const char *art_n2dsxl[] = {
    "[ZL].-------------[ZR] ",
    " |  |~ top screen~| | ",
    " |  |             | | ",
    " |__|_____________|_| ",
    " |  |~ btm screen~| | ",
    " |  |             | | ",
    " |--|-------------|--|",
    " |o .-----------.  /\\ |",
    " |  |           | (  )|",
    " |+ |           |* \\/ |",
    " |  '-----------'     |",
    " |[sel]   (h)  [sta]  |",
    " '---------------------",
    NULL
};
static const int art_n2dsxl_lines = 13;

static const char **get_art(Model3DS model, int *lines) {
    switch (model) {
        case MODEL_3DS:
        case MODEL_3DSXL:   *lines = art_3ds_lines;    return art_3ds;
        case MODEL_2DS:     *lines = art_2ds_lines;    return art_2ds;
        case MODEL_N3DS:
        case MODEL_N3DSXL:  *lines = art_n3ds_lines;   return art_n3ds;
        case MODEL_N2DSXL:  *lines = art_n2dsxl_lines; return art_n2dsxl;
        default:            *lines = art_3ds_lines;    return art_3ds;
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// RENDERING
// ═══════════════════════════════════════════════════════════════════════════════

// 1-indexed row/col cursor positioning
#define GOTO(r, c) printf("\x1b[%d;%dH", (r), (c))

// Top screen layout constants
#define TOP_COLS  50
#define TOP_ROWS  30
#define ART_COL   1
#define INFO_COL  25   // art is max 23 chars wide, +1 gap = col 25

// Bottom screen is 40 cols x 30 rows
#define BOT_COLS  40
#define BOT_ROWS  30

// Center a string on the bottom screen
#define BOT_CENTER(str) ((BOT_COLS - (int)strlen(str)) / 2 + 1)

static void fmt_storage(char *out, size_t len, uint64_t free_b, uint64_t total_b) {
    if (total_b == 0) { snprintf(out, len, "N/A"); return; }
    // Use integer MB to avoid double/printf issues on ARM11
    uint32_t free_mb  = (uint32_t)(free_b  / (1024 * 1024));
    uint32_t total_mb = (uint32_t)(total_b / (1024 * 1024));
    if (total_mb >= 1024) {
        // Show as X.XGB using integer tenths
        uint32_t free_gb10  = (free_mb  * 10) / 1024;
        uint32_t total_gb10 = (total_mb * 10) / 1024;
        snprintf(out, len, "%lu.%lu/%lu.%luGB",
                 (unsigned long)(free_gb10  / 10), (unsigned long)(free_gb10  % 10),
                 (unsigned long)(total_gb10 / 10), (unsigned long)(total_gb10 % 10));
    } else {
        snprintf(out, len, "%lu/%luMB", (unsigned long)free_mb, (unsigned long)total_mb);
    }
}

static void fmt_bat_bar(char *out, size_t len, int pct) {
    int filled = pct / 10;
    char bar[12] = {0};
    for (int i = 0; i < 10; i++)
        bar[i] = (i < filled) ? '#' : '.';
    snprintf(out, len, "[%s]%3d%%", bar, pct);
}

// Print a label+value pair at (row, INFO_COL)
// label is colored, value is white. Returns next row.
static int info_row(int row, const char *lcol, const char *label,
                    const char *val) {
    GOTO(row, INFO_COL);
    printf("%s%-10s" COL_RESET "%s", lcol, label, val);
    return row + 1;
                    }

                    static void draw(const SysInfo *s) {
                        // ── Top screen ───────────────────────────────────────────────────────────
                        consoleSelect(&topScreen);
                        printf("\x1b[2J");

                        int art_lines;
                        const char **art = get_art(s->model, &art_lines);

                        // Art — start at row 2 for a small top margin
                        for (int i = 0; i < art_lines; i++) {
                            GOTO(i + 2, ART_COL);
                            printf(COL_CYAN "%s" COL_RESET, art[i]);
                        }

                        if (s->is_xl) {
                            GOTO(art_lines + 2, ART_COL + 4);
                            printf(COL_DIM "(XL)" COL_RESET);
                        }

                        // ── Info column ───────────────────────────────────────────────────────────
                        // Get hardware spec for this model
                        int mi = (int)s->model;
                        if (mi < 0 || mi > 5) mi = 0;
                        const HWSpec *hw = &hw_specs[mi];

                        int row = 2;

                        // Model title
                        GOTO(row, INFO_COL);
                        printf(COL_YELLOW COL_BOLD "%s" COL_RESET, s->model_name);
                        row += 2;

                        // ── System ───────────────────────────────────────────────────────────────
                        GOTO(row++, INFO_COL); printf(COL_DIM "-------------------------" COL_RESET);
                        row = info_row(row, COL_MAGENTA, "Firmware: ", s->firmware);
                        row = info_row(row, COL_MAGENTA, "Region:   ", s->region);
                        row++;

                        // ── Hardware ─────────────────────────────────────────────────────────────
                        GOTO(row++, INFO_COL); printf(COL_DIM "-------------------------" COL_RESET);
                        row = info_row(row, COL_BLUE, "CPU:      ", hw->cpu);
                        row = info_row(row, COL_BLUE, "GPU:      ", hw->gpu);
                        row = info_row(row, COL_BLUE, "RAM:      ", hw->ram);
                        row++;

                        // ── Display ──────────────────────────────────────────────────────────────
                        GOTO(row++, INFO_COL); printf(COL_DIM "-------------------------" COL_RESET);
                        row = info_row(row, COL_CYAN, "Top LCD:  ", hw->top_res);
                        row = info_row(row, COL_CYAN, "Bot LCD:  ", hw->bot_res);
                        row = info_row(row, COL_CYAN, "Size:     ", hw->top_size);

                        char feat[32];
                        snprintf(feat, sizeof(feat), "%s%s%s",
                                 hw->has_3d  ? "3D " : "",
                                 hw->has_nfc ? "NFC " : "",
                                 hw->sound);
                        row = info_row(row, COL_CYAN, "Features: ", feat);
                        row++;

                        // ── Battery & Storage ────────────────────────────────────────────────────
                        GOTO(row++, INFO_COL); printf(COL_DIM "-------------------------" COL_RESET);

                        char bat_bar[20];
                        fmt_bat_bar(bat_bar, sizeof(bat_bar), s->battery_pct);
                        GOTO(row, INFO_COL);
                        if (s->is_charging)
                            printf(COL_GREEN "Battery:  " COL_RESET "%s " COL_YELLOW "+" COL_RESET, bat_bar);
                        else
                            printf(COL_GREEN "Battery:  " COL_RESET "%s", bat_bar);
                        row++;

                        char stor[24];
                        fmt_storage(stor, sizeof(stor), s->sd_free, s->sd_total);
                        row = info_row(row, COL_GREEN, "SD:       ", stor);

                        fmt_storage(stor, sizeof(stor), s->nand_free, s->nand_total);
                        row = info_row(row, COL_GREEN, "NAND:     ", stor);

                        // ── Bottom screen ─────────────────────────────────────────────────────────
                        consoleSelect(&bottomScreen);
                        printf("\x1b[2J");

                        // Each string is manually measured so BOT_CENTER gives exact col
                        const char *title   = "~ 3dsfetch ~";
                        const char *sub     = "neofetch for Nintendo 3DS";
                        const char *div     = "--------------------------";
                        const char *ghub    = "github.com/viewerofall";
                        const char *hint    = "Press START to exit";

                        GOTO(2,  BOT_CENTER(title));  printf(COL_YELLOW COL_BOLD "%s" COL_RESET, title);
                        GOTO(3,  BOT_CENTER(sub));    printf(COL_DIM "%s" COL_RESET, sub);
                        GOTO(5,  BOT_CENTER(div));    printf(COL_DIM "%s" COL_RESET, div);
                        GOTO(7,  BOT_CENTER(ghub));   printf(COL_WHITE "%s" COL_RESET, ghub);
                        GOTO(9,  BOT_CENTER(div));    printf(COL_DIM "%s" COL_RESET, div);
                        GOTO(13, BOT_CENTER(hint));   printf(COL_DIM "%s" COL_RESET, hint);

                        consoleSelect(&topScreen);
                    }

                    // ═══════════════════════════════════════════════════════════════════════════════
                    // MAIN
                    // ═══════════════════════════════════════════════════════════════════════════════

                    int main(void) {
                        gfxInitDefault();
                        consoleInit(GFX_TOP,    &topScreen);
                        consoleInit(GFX_BOTTOM, &bottomScreen);
                        consoleSelect(&topScreen);

                        ptmuInit();
                        cfguInit();
                        fsInit();

                        SysInfo info;
                        sysinfo_get(&info);
                        draw(&info);

                        while (aptMainLoop()) {
                            hidScanInput();
                            if (hidKeysDown() & KEY_START) break;
                            gfxFlushBuffers();
                            gfxSwapBuffers();
                            gspWaitForVBlank();
                        }

                        fsExit();
                        cfguExit();
                        ptmuExit();
                        gfxExit();
                        return 0;
                    }
