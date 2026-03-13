#include "sysinfo.h"

#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/modulemgr.h>
#include <psp2/power.h>
#include <psp2/io/stat.h>
#include <psp2/io/devctl.h>
#include <psp2/registrymgr.h>
#include <psp2/appmgr.h>

#include <stdio.h>
#include <string.h>

// ─── Internal helpers ────────────────────────────────────────────────────────

/**
 * Model detection using the registry only — no phantom APIs.
 *
 * /CONFIG/SYSTEM machine_type values (confirmed from community research):
 *   0x00 = Vita 1000 (OLED)
 *   0x20 = Vita 2000 (Slim)
 *   0x50 = PS TV (Dolce)
 *
 * scePowerGetBatteryLifePercent() returns < 0 on PS TV as a cross-check.
 */
static VitaModel detect_model(void) {
    int machine_type = 0;
    sceRegMgrGetKeyInt("/CONFIG/SYSTEM", "machine_type", &machine_type);

    if (machine_type == 0x50)
        return MODEL_PSTV;
    if (machine_type == 0x20)
        return MODEL_VITA_2000;

    // Cross-check: PS TV has no battery
    if (scePowerGetBatteryLifePercent() < 0)
        return MODEL_PSTV;

    return MODEL_VITA_1000;
}

static void model_to_string(VitaModel model, char *out, size_t len) {
    switch (model) {
        case MODEL_VITA_1000: snprintf(out, len, "PS Vita 1000 (OLED)");  break;
        case MODEL_VITA_2000: snprintf(out, len, "PS Vita 2000 (Slim)");  break;
        case MODEL_PSTV:      snprintf(out, len, "PlayStation TV");        break;
        default:              snprintf(out, len, "Unknown");               break;
    }
}

/**
 * sceKernelGetSystemSwVersion is in modulemgr.h.
 * SceKernelFwInfo is just a typedef for SceKernelSystemSwVersion — use the
 * real struct name to avoid typedef resolution issues across vitasdk versions.
 */
static void get_firmware(char *out, size_t len) {
    SceKernelSystemSwVersion fw;
    memset(&fw, 0, sizeof(fw));
    fw.size = sizeof(fw);

    if (sceKernelGetSystemSwVersion(&fw) >= 0 && fw.versionString[0] != '\0') {
        // versionString like "03.740.011" → trim to "3.74"
        char *s = fw.versionString;
        while (*s == '0' && *(s+1) != '.') s++;
        int dots = 0, i = 0;
        while (*s && i < (int)len - 1) {
            if (*s == '.') { dots++; if (dots == 2) break; }
            out[i++] = *s++;
        }
        out[i] = '\0';
    } else {
        snprintf(out, len, "Unknown");
    }
}

/**
 * CFW detection via taiHEN config file presence.
 * taiHEN always writes its config to one of these paths — no kernel access needed.
 * Enso additionally creates a boot-time marker we can check.
 */
static void get_cfw(char *out, size_t len) {
    // Enso marker: it installs an extra payload that leaves a file at ur0:
    SceIoStat st;
    memset(&st, 0, sizeof(st));

    bool has_tai  = (sceIoGetstat("ur0:tai/config.txt",  &st) >= 0 ||
    sceIoGetstat("ux0:tai/config.txt",  &st) >= 0);
    bool has_enso = (sceIoGetstat("ur0:tai/boot_config.txt", &st) >= 0);

    if (has_enso)
        snprintf(out, len, "HENkaku Enso");
    else if (has_tai)
        snprintf(out, len, "HENkaku");
    else
        snprintf(out, len, "Stock");
}

/**
 * Region comes from the system registry.
 * Key: /CONFIG/NP - account_id (we use system locale as a proxy since
 * SNPS region isn't a simple registry key on all firmware versions).
 *
 * More reliable: check /CONFIG/SYSTEM - language
 */
static void get_region(char *out, size_t len) {
    int lang = 0;
    sceRegMgrGetKeyInt("/CONFIG/SYSTEM", "language", &lang);
    // SCE language IDs:  0=Japanese, 1=English(US), 2=French, 3=Spanish, etc.
    const char *regions[] = {
        "JPN", "USA", "FRA", "ESP", "DEU",
        "ITA", "NLD", "POR", "RUS", "KOR",
        "CHT", "CHS", "FIN", "SWE", "DAN",
        "NOR", "POL", "PTB", "ENG"
    };
    int count = sizeof(regions) / sizeof(regions[0]);
    snprintf(out, len, "%s", (lang >= 0 && lang < count) ? regions[lang] : "UNK");
}

static void get_storage(const char *mount, StorageInfo *out) {
    out->free_bytes  = 0;
    out->total_bytes = 0;

    uint64_t free_size  = 0;
    uint64_t total_size = 0;
    // sceAppMgrGetDevInfo is the correct Vita userspace API for mount point sizes
    if (sceAppMgrGetDevInfo(mount, &total_size, &free_size) >= 0) {
        out->free_bytes  = free_size;
        out->total_bytes = total_size;
    }
}

// ─── Public API ──────────────────────────────────────────────────────────────

int sysinfo_get(SysInfo *out) {
    if (!out) return -1;
    memset(out, 0, sizeof(SysInfo));

    // Model (detect first — battery logic depends on it)
    out->model = detect_model();
    model_to_string(out->model, out->model_name, sizeof(out->model_name));

    // Battery — PS TV has no battery; scePowerGetBatteryLifePercent returns <0 on it
    out->battery_present = (out->model != MODEL_PSTV);
    if (out->battery_present) {
        int pct = scePowerGetBatteryLifePercent();
        out->battery_pct = (pct >= 0) ? pct : -1;
        out->is_charging = scePowerIsBatteryCharging();
    } else {
        out->battery_pct = -1;
        out->is_charging = false;
    }

    // Firmware + CFW
    get_firmware(out->firmware, sizeof(out->firmware));
    get_cfw(out->cfw, sizeof(out->cfw));

    // Region
    get_region(out->region, sizeof(out->region));

    // CPU / GPU clocks (MHz)
    out->cpu_mhz = scePowerGetArmClockFrequency();
    out->gpu_mhz = scePowerGetGpuClockFrequency();

    // Storage
    get_storage("ux0:", &out->ux0);
    get_storage("ur0:", &out->ur0);

    return 0;
}
