#pragma once

#include <3ds.h>
#include <stdbool.h>
#include <stdint.h>

// ─── ANSI colors (work with libctru console) ─────────────────────────────────
#define COL_RESET   "\x1b[0m"
#define COL_BOLD    "\x1b[1m"
#define COL_CYAN    "\x1b[36m"
#define COL_YELLOW  "\x1b[33m"
#define COL_GREEN   "\x1b[32m"
#define COL_MAGENTA "\x1b[35m"
#define COL_BLUE    "\x1b[34m"
#define COL_WHITE   "\x1b[37m"
#define COL_DIM     "\x1b[2m"

// ─── Model IDs ───────────────────────────────────────────────────────────────
// Matches CFG_GetSystemModel() return values exactly
typedef enum {
    MODEL_3DS       = 0,  // Original 3DS
    MODEL_3DSXL     = 1,  // Original 3DS XL
    MODEL_2DS       = 2,  // 2DS (flat brick)
    MODEL_N3DS      = 3,  // New 3DS
    MODEL_N3DSXL    = 4,  // New 3DS XL
    MODEL_N2DSXL    = 5,  // New 2DS XL
} Model3DS;

// ─── System info bundle ──────────────────────────────────────────────────────
typedef struct {
    Model3DS model;
    bool     is_xl;           // true for 3DSXL, N3DSXL — show (XL) tag
    char     model_name[32];
    char     firmware[32];    // e.g. "11.17.0-50E"
    char     region[8];       // e.g. "USA"
    bool     is_new_3ds;      // affects CPU speed label
    int      battery_pct;
    bool     is_charging;
    uint64_t sd_free;         // bytes
    uint64_t sd_total;
    uint64_t nand_free;
    uint64_t nand_total;
} SysInfo;
