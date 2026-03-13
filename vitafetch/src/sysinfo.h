#pragma once

#include <psp2/types.h>
#include <stdbool.h>
#include <stdint.h>

// ─── Model IDs ───────────────────────────────────────────────────────────────

typedef enum {
    MODEL_VITA_1000 = 0,  // Original OLED Vita
    MODEL_VITA_2000,      // Slim Vita
    MODEL_PSTV,           // PlayStation TV / Vita TV
    MODEL_UNKNOWN
} VitaModel;

// ─── Storage info ────────────────────────────────────────────────────────────

typedef struct {
    uint64_t free_bytes;
    uint64_t total_bytes;
} StorageInfo;

// ─── Main info bundle ────────────────────────────────────────────────────────

typedef struct {
    VitaModel   model;
    char        model_name[32];     // e.g. "PS Vita 1000 (OLED)"
    char        firmware[16];       // e.g. "3.74"
    char        cfw[32];            // e.g. "HENkaku Ensō" or "Stock"
    char        region[16];         // e.g. "USA"
    int         cpu_mhz;            // e.g. 333 or 444
    int         gpu_mhz;            // e.g. 111 or 222
    int         battery_pct;        // 0–100, -1 if no battery (PS TV)
    bool        is_charging;
    bool        battery_present;
    StorageInfo ux0;                // Memory card / internal storage
    StorageInfo ur0;                // System storage
} SysInfo;

// ─── Public API ──────────────────────────────────────────────────────────────

/**
 * Populate a SysInfo struct with live data from SceKernel, ScePower, etc.
 * Returns 0 on success, negative SCE error code on failure.
 */
int sysinfo_get(SysInfo *out);
