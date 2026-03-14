/*
 * wiifetch - neofetch for the Wii
 * Forked from NiioFetch by abdelali221
 * License: GPL-3.0
 *
 * Deps: devkitPPC, libogc, libfat
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/statvfs.h>
#include <time.h>

#include <gccore.h>
#include <wiiuse/wpad.h>
#include <fat.h>
#include <ogc/conf.h>
#include <ogc/system.h>

// ─── Console setup ───────────────────────────────────────────────────────────
static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

// ─── ANSI-style color codes via VT escape (libogc CON supports these) ────────
#define COL_RESET   "\x1b[0m"
#define COL_BOLD    "\x1b[1m"
#define COL_BLUE    "\x1b[34m"
#define COL_CYAN    "\x1b[36m"
#define COL_WHITE   "\x1b[37m"
#define COL_GRAY    "\x1b[90m"
#define COL_YELLOW  "\x1b[33m"
#define COL_GREEN   "\x1b[32m"
#define COL_RED     "\x1b[31m"

// ─── Wii logo ASCII art (polished, 2-tone) ───────────────────────────────────
static const char *WII_LOGO[] = {
    COL_BOLD COL_WHITE "  ██╗    ██╗██╗██╗" COL_RESET,
    COL_BOLD COL_CYAN  "  ██║    ██║██║██║" COL_RESET,
    COL_BOLD COL_WHITE "  ██║ █╗ ██║██║██║" COL_RESET,
    COL_BOLD COL_CYAN  "  ██║███╗██║██║██║" COL_RESET,
    COL_BOLD COL_WHITE "  ╚███╔███╔╝██║██║" COL_RESET,
    COL_BOLD COL_CYAN  "   ╚══╝╚══╝ ╚═╝╚═╝" COL_RESET,
    COL_GRAY "  Nintendo Wii" COL_RESET,
    "",
};
#define LOGO_LINES 8

// ─── Startup animation ───────────────────────────────────────────────────────
static void type_string(const char *str, int delay_us) {
    for (int i = 0; str[i]; i++) {
        putchar(str[i]);
        fflush(stdout);
        usleep(delay_us);
    }
}

static void startup_animation(void) {
    // Clear screen
    printf("\x1b[2J\x1b[H");

    // Simulate terminal prompt typing
    usleep(300000); // 300ms pause before starting
    printf(COL_GREEN COL_BOLD "user@wii" COL_RESET COL_WHITE ":" COL_RESET
           COL_CYAN "~" COL_RESET "$ ");
    fflush(stdout);
    usleep(400000);

    type_string("wiifetch", 80000); // type each char with 80ms delay
    usleep(300000);
    printf("\n");
    fflush(stdout);
    usleep(500000);
}

// ─── System info getters ─────────────────────────────────────────────────────

// Returns IOS version as integer (e.g. 58)
static int get_ios_version(void) {
    return IOS_GetVersion();
}

// Returns system menu version string (CONF_GetTitleVersion returns raw u16)
static void get_sysmenu_version(char *buf, size_t len) {
    // The system title version encodes region+version — map known values
    // 0x0200=3.2U, 0x021e=4.1U, 0x022e=4.3U etc.
    // We just report the raw u16 and known string where possible
    struct {
        u16 ver;
        const char *name;
    } known[] = {
        {0x01C0, "3.0"},  {0x01C2, "3.1"},  {0x01E0, "3.2"},
        {0x0200, "3.3"},  {0x0220, "4.0"},   {0x021E, "4.1"},
        {0x0230, "4.2"},  {0x022E, "4.3"},   {0, NULL}
    };

    u16 raw = CONF_GetSystemMenuVersion();
    for (int i = 0; known[i].name; i++) {
        if (raw == known[i].ver) {
            snprintf(buf, len, "%s", known[i].name);
            return;
        }
    }
    snprintf(buf, len, "Unknown (0x%04X)", raw);
}

// Uptime via Timebase counter — libogc ticks at 1/4 bus clock (162 MHz bus →
// 40.5 MHz TB). SYS_Time() returns TB ticks since boot.
static void get_uptime(char *buf, size_t len) {
    u64 ticks = SYS_Time();
    // TB freq = bus_speed/4. Bus speed on Wii = 243MHz → TB = ~60.75MHz
    // devkitPPC defines TB_TIMER_CLOCK = 60750000
    u64 secs = ticks / TB_TIMER_CLOCK;
    u64 mins  = secs / 60;  secs %= 60;
    u64 hours = mins / 60;  mins %= 60;
    snprintf(buf, len, "%lluh %llum %llus", hours, mins, secs);
}

// Storage: returns free/total in MB for a given mount point (e.g. "sd:/")
static void get_storage(const char *mount, u64 *free_mb, u64 *total_mb) {
    struct statvfs st;
    *free_mb = 0; *total_mb = 0;
    if (statvfs(mount, &st) == 0) {
        u64 block = st.f_bsize;
        *total_mb = (u64)st.f_blocks * block / (1024 * 1024);
        *free_mb  = (u64)st.f_bfree  * block / (1024 * 1024);
    }
}

// Wii region from CONF
static const char *get_region(void) {
    switch (CONF_GetRegion()) {
        case CONF_REGION_JP: return "NTSC-J (Japan)";
        case CONF_REGION_US: return "NTSC-U (USA)";
        case CONF_REGION_EU: return "PAL (Europe)";
        case CONF_REGION_KR: return "NTSC-K (Korea)";
        default:             return "Unknown";
    }
}

// TV mode
static const char *get_video_mode(void) {
    switch (CONF_GetVideo()) {
        case CONF_VIDEO_NTSC:  return "NTSC";
        case CONF_VIDEO_PAL:   return "PAL";
        case CONF_VIDEO_MPAL:  return "MPAL";
        default:               return "Unknown";
    }
}

// Language
static const char *get_language(void) {
    switch (CONF_GetLanguage()) {
        case CONF_LANG_JAPANESE:            return "Japanese";
        case CONF_LANG_ENGLISH:             return "English";
        case CONF_LANG_GERMAN:              return "German";
        case CONF_LANG_FRENCH:              return "French";
        case CONF_LANG_SPANISH:             return "Spanish";
        case CONF_LANG_ITALIAN:             return "Italian";
        case CONF_LANG_DUTCH:               return "Dutch";
        case CONF_LANG_SIMP_CHINESE:        return "Simplified Chinese";
        case CONF_LANG_TRAD_CHINESE:        return "Traditional Chinese";
        case CONF_LANG_KOREAN:              return "Korean";
        default:                            return "Unknown";
    }
}

// Aspect ratio
static const char *get_aspect(void) {
    return CONF_GetAspectRatio() == CONF_ASPECT_16_9 ? "16:9" : "4:3";
}

// ─── Pretty print helpers ─────────────────────────────────────────────────────

// Logo line + info line, side by side
static void print_row(int logo_idx, const char *logo[], int logo_count,
                      const char *key, const char *val) {
    if (logo_idx < logo_count)
        printf("  %-36s", logo[logo_idx]);
    else
        printf("  %-18s", ""); // blank logo column (approx same width as ANSI str)

    if (key && val)
        printf("  " COL_CYAN COL_BOLD "%-18s" COL_RESET COL_WHITE "%s" COL_RESET, key, val);
    printf("\n");
}

static void print_divider(void) {
    printf("  " COL_GRAY
           "────────────────────────────────────────────────────\n"
           COL_RESET);
}

static void print_section_header(const char *title) {
    printf("\n  " COL_BOLD COL_BLUE "══ %s ══" COL_RESET "\n", title);
}

// Color palette dots
static void print_palette(void) {
    printf("  ");
    const char *colors[] = {
        "\x1b[40m", "\x1b[41m", "\x1b[42m", "\x1b[43m",
        "\x1b[44m", "\x1b[45m", "\x1b[46m", "\x1b[47m"
    };
    for (int i = 0; i < 8; i++)
        printf("%s   " COL_RESET, colors[i]);
    printf("\n");
}

// ─── Main ─────────────────────────────────────────────────────────────────────
int main(int argc, char **argv) {
    // Init video
    VIDEO_Init();
    rmode = VIDEO_GetPreferredMode(NULL);
    xfb   = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(xfb);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if (rmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();

    // Init console — renders to framebuffer
    CON_InitEx(rmode, 0, 0, rmode->fbWidth, rmode->xfbHeight);

    // Init controllers
    PAD_Init();
    WPAD_Init();

    // Init filesystem (SD card)
    bool sd_ok = fatInitDefault();

    // ── Startup animation ──────────────────────────────────────────────────
    startup_animation();

    // ── Gather system info ────────────────────────────────────────────────
    char sysmenu[32];
    get_sysmenu_version(sysmenu, sizeof(sysmenu));

    char uptime[48];
    get_uptime(uptime, sizeof(uptime));

    u64 sd_free = 0, sd_total = 0;
    u64 nand_free = 0, nand_total = 0;

    if (sd_ok) {
        get_storage("sd:/", &sd_free, &sd_total);
        get_storage("nand:/", &nand_free, &nand_total);
    }

    char sd_buf[64], nand_buf[64];
    if (sd_ok && sd_total > 0)
        snprintf(sd_buf, sizeof(sd_buf), "%llu MB free / %llu MB total", sd_free, sd_total);
    else
        snprintf(sd_buf, sizeof(sd_buf), "Not mounted");

    if (sd_ok && nand_total > 0)
        snprintf(nand_buf, sizeof(nand_buf), "%llu MB free / %llu MB total", nand_free, nand_total);
    else
        snprintf(nand_buf, sizeof(nand_buf), "Not accessible");

    char ios_buf[16];
    snprintf(ios_buf, sizeof(ios_buf), "IOS%d", get_ios_version());

    // user@wii header
    printf(COL_BOLD COL_GREEN "  user" COL_RESET COL_WHITE "@" COL_RESET
           COL_BOLD COL_CYAN  "wii\n" COL_RESET);
    print_divider();

    // ── Logo + System section ─────────────────────────────────────────────
    int row = 0;
    print_row(row++, WII_LOGO, LOGO_LINES, "OS:",       "Nintendo Wii System Menu");
    print_row(row++, WII_LOGO, LOGO_LINES, "Version:",  sysmenu);
    print_row(row++, WII_LOGO, LOGO_LINES, "IOS:",      ios_buf);
    print_row(row++, WII_LOGO, LOGO_LINES, "Region:",   get_region());
    print_row(row++, WII_LOGO, LOGO_LINES, "Language:", get_language());
    print_row(row++, WII_LOGO, LOGO_LINES, "Video:",    get_video_mode());
    print_row(row++, WII_LOGO, LOGO_LINES, "Aspect:",   get_aspect());
    // Fill remaining logo lines with blanks
    while (row < LOGO_LINES)
        print_row(row++, WII_LOGO, LOGO_LINES, NULL, NULL);

    // ── Hardware section ─────────────────────────────────────────────────
    print_section_header("HARDWARE");
    printf("  " COL_CYAN COL_BOLD "%-18s" COL_RESET COL_WHITE "729 MHz IBM Broadway (PPC)\n" COL_RESET, "CPU:");
    printf("  " COL_CYAN COL_BOLD "%-18s" COL_RESET COL_WHITE "243 MHz ATI Hollywood\n"        COL_RESET, "GPU:");
    printf("  " COL_CYAN COL_BOLD "%-18s" COL_RESET COL_WHITE "88 MB (64 MB MEM1 + 24 MB MEM2)\n" COL_RESET, "RAM:");
    printf("  " COL_CYAN COL_BOLD "%-18s" COL_RESET COL_WHITE "512 KB NAND Flash (internal)\n" COL_RESET, "Storage:");

    // ── Storage section ───────────────────────────────────────────────────
    print_section_header("STORAGE");
    printf("  " COL_CYAN COL_BOLD "%-18s" COL_RESET COL_WHITE "%s\n" COL_RESET, "SD Card:", sd_buf);
    printf("  " COL_CYAN COL_BOLD "%-18s" COL_RESET COL_WHITE "%s\n" COL_RESET, "NAND:", nand_buf);

    // ── Session section ───────────────────────────────────────────────────
    print_section_header("SESSION");
    printf("  " COL_CYAN COL_BOLD "%-18s" COL_RESET COL_WHITE "%s\n" COL_RESET, "Uptime:", uptime);

    // ── Palette ───────────────────────────────────────────────────────────
    printf("\n");
    print_palette();
    printf("\n");

    print_divider();
    printf("  " COL_GRAY "Press HOME to exit." COL_RESET "\n");

    // ── Wait for HOME button ──────────────────────────────────────────────
    while (1) {
        VIDEO_WaitVSync();
        WPAD_ScanPads();
        PAD_ScanPads();

        u32 pressed = WPAD_ButtonsDown(0);
        u32 gc_pressed = PAD_ButtonsDown(0);

        if ((pressed & WPAD_BUTTON_HOME) || (gc_pressed & PAD_BUTTON_START))
            break;
    }

    return 0;
}
