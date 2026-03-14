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
#include <ogcsys.h>
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

// ─── Console variants ─────────────────────────────────────────────────────────
typedef enum { CON_WII, CON_MWII, CON_VWII } ConsoleType;

// Detect console type via IOS version heuristic:
// vWII runs on Wii U — its IOS versions are 57+. mWII is a modded Wii
// detectable by checking if running IOS > 200 (custom IOS range).
// Real Wii stock IOS slots are typically 9, 36, 53, 55, 56, 58, 61, 80.
static ConsoleType detect_console(void) {
    s32 ios = IOS_GetVersion();
    // vWII on Wii U always runs IOS 57
    if (ios == 57) return CON_VWII;
    // Custom IOS slots 200-255 = modded Wii
    if (ios >= 200) return CON_MWII;
    return CON_WII;
}

// ─── Logo print — taken directly from Niiofetch source ───────────────────────
// Pure printf lines, no padding tricks, exactly as Niiofetch renders them.
// We print logo and info side by side by storing logo lines in an array
// and printing each alongside an info field.

#define WII_LOGO_W  9   // number of logo lines for plain WII / vWII
#define MWII_LOGO_W 17  // mWII has extra lines for the bottom section

// Plain Wii logo lines (white)
static const char *LOGO_WII[] = {
    COL_WHITE "    &&&        &        &&&  &&&&   &&&&",
    COL_WHITE "    &&&&      &&&      &&&&  &&&&   &&&&",
    COL_WHITE "     &&&     &&&&&    &&&&",
    COL_WHITE "     &&&&   &&& &&&   &&&&   &&&&   &&&&",
    COL_WHITE "      &&&   &&& &&&  &&&&    &&&&   &&&&",
    COL_WHITE "      &&&& &&&   &&& &&&&    &&&&   &&&&",
    COL_WHITE "       &&&&&&&   &&&&&&&     &&&&   &&&&",
    COL_WHITE "       &&&&&&     &&&&&      &&&&   &&&&",
    COL_WHITE "        &&&&       &&&&      &&&&   &&&& &&",
};

// mWII = Wii logo + modchip section below
static const char *LOGO_MWII[] = {
    COL_WHITE "    &&&        &        &&&  &&&&   &&&&",
    COL_WHITE "    &&&&      &&&      &&&&  &&&&   &&&&",
    COL_WHITE "     &&&     &&&&&    &&&&",
    COL_WHITE "     &&&&   &&& &&&   &&&&   &&&&   &&&&",
    COL_WHITE "      &&&   &&& &&&  &&&&    &&&&   &&&&",
    COL_WHITE "      &&&& &&&   &&& &&&&    &&&&   &&&&",
    COL_WHITE "       &&&&&&&   &&&&&&&     &&&&   &&&&",
    COL_WHITE "       &&&&&&     &&&&&      &&&&   &&&&",
    COL_WHITE "        &&&&       &&&&      &&&&   &&&& &&",
    COL_GRAY  " -------------------------------------------",
    COL_GRAY  " -------------------------------------------",
    COL_WHITE "                      @@           :@@",
    COL_WHITE "                       .             .",
    COL_WHITE "        +@@@@@@@@@@   @@   @@@@@@  :@@",
    COL_WHITE "       :@@  =@@  *@@  @@  @@-  *@* :@@",
    COL_WHITE "       :@@  =@@  *@@  @@  @@:  +@* :@@",
    COL_WHITE "       :@@  =@@  *@@  @@  @@.  +@* :@@",
};

// vWII = Wii logo + vWII suffix in cyan
static const char *LOGO_VWII[] = {
    COL_WHITE "&&&        &        &&& &&&  &&& " COL_CYAN "&&+&  &x&  &x&.",
    COL_WHITE "&&&&      &&&      &&&& &&&  &&& " COL_CYAN "&&x&  &&& .&x&.",
    COL_WHITE " &&&     &&&&&     &&&           " COL_CYAN "&&x&  &&& .&x&.",
    COL_WHITE " &&&&   &&& &&&   &&&   &&&  &&& " COL_CYAN "&x&&&    .&&$&",
    COL_WHITE "  &&&   &&& &&&  &&&&   &&&  &&&  " COL_CYAN "&&&&&&&&&&&&",
    COL_WHITE "  &&&& &&&   &&& &&&&   &&&  &&&",
    COL_WHITE "   &&&&&&&   &&&&&&&    &&&  &&&",
    COL_WHITE "   &&&&&&     &&&&&     &&&  &&&",
    COL_WHITE "    &&&&       &&&&     &&&  &&&",
};

// ─── Startup animation — CRT scanline fill ───────────────────────────────────
static void type_string(const char *str, int delay_us) {
    for (int i = 0; str[i]; i++) {
        putchar(str[i]);
        fflush(stdout);
        usleep(delay_us);
    }
}

// Print a logo line or a blank placeholder of the same height
static void print_logo_line(const char **logo, int logo_count, int idx) {
    if (idx < logo_count && logo[idx][0] != '\0')
        printf("%s" COL_RESET "\n", logo[idx]);
    else
        printf("\n");
}

static void startup_animation(const char **logo, int logo_count,
                              const char *hostname) {
    // ── Pass 1: print odd lines, blank even lines ─────────────────────────
    for (int i = 0; i < logo_count; i++) {
        if (i % 2 == 0)
            print_logo_line(logo, logo_count, i);
        else
            printf("\n");
        fflush(stdout);
        usleep(40000);
    }
    VIDEO_WaitVSync();
    usleep(60000);

    // ── Pass 2: go back up and fill in even lines ─────────────────────────
    // Move cursor back to top of logo block
    for (int i = 0; i < logo_count; i++) {
        printf("\x1b[1A"); // cursor up
    }
    fflush(stdout);

    for (int i = 0; i < logo_count; i++) {
        printf("\x1b[2K"); // erase line
        if (i % 2 == 1)
            print_logo_line(logo, logo_count, i);
        else {
            // reprint odd line so cursor advances correctly
            print_logo_line(logo, logo_count, i);
        }
        fflush(stdout);
        usleep(25000);
    }
    VIDEO_WaitVSync();
    usleep(500000); // hold on full logo

    // ── Pass 3: wipe logo upward line by line ─────────────────────────────
    for (int i = 0; i < logo_count; i++) {
        printf("\x1b[1A\x1b[2K");
        fflush(stdout);
        VIDEO_WaitVSync();
        usleep(25000);
    }

    // ── Pass 4: type the prompt ───────────────────────────────────────────
    usleep(150000);
    printf(COL_GREEN COL_BOLD "%s" COL_RESET
    COL_WHITE ":" COL_RESET
    COL_CYAN "~" COL_RESET "$ ", hostname);
    fflush(stdout);
    usleep(250000);
    type_string("wiifetch", 70000);
    printf("\n");
    fflush(stdout);
    usleep(300000);

    // Erase prompt line
    printf("\x1b[1A\x1b[2K");
    fflush(stdout);
    VIDEO_WaitVSync();
                              }

                              // ─── Pretty print helpers ─────────────────────────────────────────────────────

                              // Visible length of a string — skips ANSI escape sequences
                              static int vis_len(const char *s) {
                                  int len = 0;
                                  while (*s) {
                                      if (*s == '\x1b') {
                                          // skip until 'm'
                                          while (*s && *s != 'm') s++;
                                          if (*s) s++;
                                      } else {
                                          len++; s++;
                                      }
                                  }
                                  return len;
                              }

                              // Print one row: logo on left padded to INFO_COL, key:val on right.
                              // Uses spaces calculated from vis_len to avoid ANSI padding breakage.
                              #define LEFT_PAD  "    "  // 4 space left margin
                              // 4:3 safe area = ~70 cols. Logo = 43, LEFT_PAD = 4, gap = 1 → info starts at 48
                              // That leaves 70-48 = 22 chars for key(10) + space(1) + value(11 max)
                              #define INFO_COL  48
                              #define KEY_W     "%-10s"
                              #define VAL_MAX   18  // truncate values longer than this

                              static void print_row(int idx, const char **logo, int logo_count,
                                                    const char *key, const char *val) {
                                  int printed = 0;
                                  printf(LEFT_PAD);
                                  printed += 4;
                                  if (idx < logo_count && logo[idx][0] != '\0') {
                                      printf("%s" COL_RESET, logo[idx]);
                                      printed += vis_len(logo[idx]);
                                  }
                                  for (int i = printed; i < INFO_COL; i++) putchar(' ');
                                  if (key && val) {
                                      printf(COL_CYAN COL_BOLD KEY_W COL_RESET " %.*s",
                                             key, VAL_MAX, val);
                                  }
                                  printf("\n");
                                                    }

                                                    static void print_divider(void) {
                                                        printf(LEFT_PAD COL_GRAY
                                                        "--------------------------------------------\n"
                                                        COL_RESET);
                                                    }

                                                    static void print_palette(void) {
                                                        printf(LEFT_PAD "  ");
                                                        const char *bg[] = {
                                                            "\x1b[41m", "\x1b[42m", "\x1b[43m", "\x1b[44m",
                                                            "\x1b[45m", "\x1b[46m", "\x1b[47m", "\x1b[40m"
                                                        };
                                                        for (int i = 0; i < 8; i++)
                                                            printf("%s   " COL_RESET, bg[i]);
                                                        printf("\n");
                                                    }

                                                    // Returns IOS version as integer (e.g. 58)
                                                    static int get_ios_version(void) {
                                                        return IOS_GetVersion();
                                                    }

                                                    // Uptime via Timebase counter — libogc ticks at 1/4 bus clock (162 MHz bus →
                                                    // 40.5 MHz TB). SYS_Time() returns TB ticks since boot.
                                                    static void get_uptime(char *buf, size_t len) {
                                                        // TB_TIMER_CLOCK = TB_BUS_CLOCK/4000 (defined in ogcsys.h, unit is kHz)
                                                        // So ticks / (TB_TIMER_CLOCK * 1000) = seconds
                                                        u64 ticks = SYS_Time();
                                                        u64 total_secs = ticks / ((u64)TB_TIMER_CLOCK * 1000);
                                                        u64 mins  = total_secs / 60; u64 secs = total_secs % 60;
                                                        u64 hours = mins / 60;       mins %= 60;
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
                                                            case CONF_REGION_JP: return "NTSC-J";
                                                            case CONF_REGION_US: return "NTSC-U";
                                                            case CONF_REGION_EU: return "PAL";
                                                            case CONF_REGION_KR: return "NTSC-K";
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

                                                    // ─── Main ─────────────────────────────────────────────────────────────────────
                                                    int main(int argc, char **argv) {
                                                        (void)argc; (void)argv;
                                                        VIDEO_Init();
                                                        rmode = VIDEO_GetPreferredMode(NULL);
                                                        xfb   = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
                                                        VIDEO_Configure(rmode);
                                                        VIDEO_SetNextFramebuffer(xfb);
                                                        VIDEO_SetBlack(FALSE);
                                                        VIDEO_Flush();
                                                        VIDEO_WaitVSync();
                                                        if (rmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();

                                                        CON_InitEx(rmode, 0, 0, rmode->fbWidth, rmode->xfbHeight);
                                                        PAD_Init();
                                                        WPAD_Init();

                                                        bool sd_ok = false;
                                                        for (int i = 0; i < 5 && !sd_ok; i++) {
                                                            sd_ok = fatInitDefault();
                                                            if (!sd_ok) usleep(100000);
                                                        }

                                                        // Detect console first so animation uses the right logo
                                                        ConsoleType con = detect_console();
                                                        const char **logo;
                                                        int logo_count;
                                                        const char *con_name;
                                                        const char *hostname;

                                                        switch (con) {
                                                            case CON_VWII:
                                                                logo = LOGO_VWII; logo_count = WII_LOGO_W;
                                                                con_name = "Homebrew Channel (vWii)";
                                                                hostname = "user@vwii"; break;
                                                            case CON_MWII:
                                                                logo = LOGO_MWII; logo_count = MWII_LOGO_W;
                                                                con_name = "Homebrew Channel (modded)";
                                                                hostname = "user@mwii"; break;
                                                            default:
                                                                logo = LOGO_WII; logo_count = WII_LOGO_W;
                                                                con_name = "Homebrew Channel";
                                                                hostname = "user@wii"; break;
                                                        }

                                                        startup_animation(logo, logo_count, hostname);

                                                        // Gather info
                                                        char uptime[48];
                                                        get_uptime(uptime, sizeof(uptime));

                                                        u64 sd_free = 0, sd_total = 0;
                                                        if (sd_ok) {
                                                            get_storage("sd:/", &sd_free, &sd_total);
                                                            if (sd_total == 0)
                                                                get_storage("fat:/", &sd_free, &sd_total);
                                                        }

                                                        char sd_buf[24];
                                                        if (sd_ok && sd_total > 0)
                                                            snprintf(sd_buf, sizeof(sd_buf), "%uM/%uM",
                                                                     (unsigned)(sd_free & 0xFFFFFFFF),
                                                                     (unsigned)(sd_total & 0xFFFFFFFF));
                                                            else
                                                                snprintf(sd_buf, sizeof(sd_buf), "None");

                                                        char ios_buf[16];
                                                        snprintf(ios_buf, sizeof(ios_buf), "IOS%d", get_ios_version());

                                                        // Vertical padding — push content toward center on NTSC (26 lines)
                                                        // 26 lines total: 3 pad + 1 header + 1 divider + 12 rows + 1 blank + 1 palette + 1 divider + 1 footer = 22, leaves 4 at bottom
                                                        printf("\n\n\n");

                                                        // Header
                                                        printf(LEFT_PAD COL_BOLD COL_GREEN "user" COL_RESET
                                                        COL_WHITE "@" COL_RESET
                                                        COL_BOLD COL_CYAN "%s\n" COL_RESET,
                                                        con == CON_VWII ? "vwii" : con == CON_MWII ? "mwii" : "wii");
                                                        print_divider();

                                                        int row = 0;
                                                        int total = logo_count > 12 ? logo_count : 12;

                                                        #define ROW(k, v) print_row(row++, logo, logo_count, k, v)
                                                        ROW("OS:",      con_name);
                                                        ROW("IOS:",     ios_buf);
                                                        ROW("Region:",  get_region());
                                                        ROW("Lang:",    get_language());
                                                        ROW("Video:",   get_video_mode());
                                                        ROW("Aspect:",  get_aspect());
                                                        ROW("CPU:",     "Broadway");
                                                        ROW("GPU:",     "Hollywood");
                                                        ROW("RAM:",     "64+24 MB");
                                                        ROW("NAND:",    "512 KB");
                                                        ROW("SD:",      sd_buf);
                                                        ROW("Uptime:",  uptime);
                                                        while (row < total)
                                                            print_row(row++, logo, logo_count, NULL, NULL);
                                                        #undef ROW

                                                        printf("\n");
                                                        print_palette();
                                                        print_divider();
                                                        printf(LEFT_PAD COL_GRAY "Press HOME or START to exit." COL_RESET "\n");

                                                        while (1) {
                                                            VIDEO_WaitVSync();
                                                            WPAD_ScanPads();
                                                            PAD_ScanPads();
                                                            if ((WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME) ||
                                                                (PAD_ButtonsDown(0)  & PAD_BUTTON_START))
                                                                break;
                                                        }

                                                        return 0;
                                                    }
