#include "render.h"

#include <vita2d.h>
#include <stdio.h>
#include <string.h>

// ─── Module-level state ──────────────────────────────────────────────────────

static vita2d_pgf *s_font = NULL;

// ─── Init / Fini ─────────────────────────────────────────────────────────────

int render_init(void) {
    vita2d_init();
    vita2d_set_clear_color(COLOR_BG);
    s_font = vita2d_load_default_pgf();
    return (s_font != NULL) ? 0 : -1;
}

void render_fini(void) {
    if (s_font) {
        vita2d_free_pgf(s_font);
        s_font = NULL;
    }
    vita2d_fini();
}

// ─── Drawing helpers ─────────────────────────────────────────────────────────

/**
 * Draw a "Label:  value" pair using a fixed value column offset so all values
 * line up regardless of label length. VALUE_COL_OFFSET is pixels from INFO_X.
 */
#define VALUE_COL_OFFSET 160

static int draw_kv(int x, int y, unsigned int label_color, const char *label,
                   unsigned int value_color, const char *value) {
    vita2d_pgf_draw_text(s_font, x, y, label_color, 1.0f, label);
    vita2d_pgf_draw_text(s_font, x + VALUE_COL_OFFSET, y, value_color, 1.0f, value);
    return y + LINE_HEIGHT;
                   }

                   static int draw_sep(int x, int y, int width) {
                       // Consume a full LINE_HEIGHT, draw the line at the midpoint.
                       // This guarantees equal breathing room above and below the separator.
                       int mid = y + LINE_HEIGHT / 2;
                       vita2d_draw_line((float)x, (float)mid,
                                        (float)(x + width), (float)mid, COLOR_DIM);
                       return y + LINE_HEIGHT;
                   }

                   static void fmt_storage(char *out, size_t len, uint64_t free_b, uint64_t total_b) {
                       if (total_b == 0) {
                           snprintf(out, len, "N/A");
                           return;
                       }
                       double free_gb  = (double)free_b  / (1024.0 * 1024.0 * 1024.0);
                       double total_gb = (double)total_b / (1024.0 * 1024.0 * 1024.0);
                       if (total_gb >= 1.0)
                           snprintf(out, len, "%.1f / %.1f GB", free_gb, total_gb);
                       else {
                           double free_mb  = (double)free_b  / (1024.0 * 1024.0);
                           double total_mb = (double)total_b / (1024.0 * 1024.0);
                           snprintf(out, len, "%.0f / %.0f MB", free_mb, total_mb);
                       }
                   }

                   // ─── ASCII art renderer ──────────────────────────────────────────────────────
                   /**
                    * PGF is a proportional font so normal text rendering mangles ASCII art.
                    * We fake monospace by rendering each character individually at a fixed
                    * CHAR_W pixel stride. This keeps box-drawing chars aligned.
                    */
                   #define CHAR_W 10  // pixels per character cell at scale 1.0f

                   static void draw_art_line(int x, int y, unsigned int color, const char *line) {
                       char ch[2] = {0, 0};
                       int cx = x;
                       for (int i = 0; line[i] != '\0'; i++) {
                           ch[0] = line[i];
                           vita2d_pgf_draw_text(s_font, cx, y, color, 1.0f, ch);
                           cx += CHAR_W;
                       }
                   }

                   // ─── Main draw ───────────────────────────────────────────────────────────────

                   void render_draw(const char **art, int art_lines, const SysInfo *info) {
                       // ── ASCII art (left column) ──────────────────────────────────────────────
                       for (int i = 0; i < art_lines; i++) {
                           draw_art_line(ART_X, ART_Y + (i * LINE_HEIGHT), COLOR_CYAN, art[i]);
                       }

                       // ── Info (right column) ──────────────────────────────────────────────────
                       int y = INFO_Y;
                       char val[128];

                       vita2d_pgf_draw_text(s_font, INFO_X, y, COLOR_YELLOW, 1.1f, info->model_name);
                       y += LINE_HEIGHT + 4;
                       y = draw_sep(INFO_X, y, 280);

                       y = draw_kv(INFO_X, y, COLOR_PINK,   "Firmware:", COLOR_WHITE,  info->firmware);
                       y = draw_kv(INFO_X, y, COLOR_PINK,   "CFW:",      COLOR_GREEN,  info->cfw);
                       y = draw_kv(INFO_X, y, COLOR_PINK,   "Region:",   COLOR_WHITE,  info->region);

                       y = draw_sep(INFO_X, y, 280);

                       snprintf(val, sizeof(val), "%d MHz", info->cpu_mhz);
                       y = draw_kv(INFO_X, y, COLOR_ORANGE, "CPU:",      COLOR_WHITE, val);
                       snprintf(val, sizeof(val), "%d MHz", info->gpu_mhz);
                       y = draw_kv(INFO_X, y, COLOR_ORANGE, "GPU:",      COLOR_WHITE, val);

                       y = draw_sep(INFO_X, y, 280);

                       if (info->battery_present) {
                           if (info->is_charging)
                               snprintf(val, sizeof(val), "%d%% (charging)", info->battery_pct);
                           else
                               snprintf(val, sizeof(val), "%d%%", info->battery_pct);
                       } else {
                           snprintf(val, sizeof(val), "N/A (PS TV)");
                       }
                       y = draw_kv(INFO_X, y, COLOR_GREEN,  "Battery:",  COLOR_WHITE, val);

                       y = draw_sep(INFO_X, y, 280);

                       fmt_storage(val, sizeof(val), info->ux0.free_bytes, info->ux0.total_bytes);
                       y = draw_kv(INFO_X, y, COLOR_CYAN,   "ux0:",      COLOR_WHITE, val);
                       fmt_storage(val, sizeof(val), info->ur0.free_bytes, info->ur0.total_bytes);
                       y = draw_kv(INFO_X, y, COLOR_CYAN,   "ur0:",      COLOR_WHITE, val);

                       vita2d_pgf_draw_text(s_font, ART_X, SCREEN_H - 24, COLOR_DIM, 0.8f,
                                            "Press START to exit  |  vitafetch");
                   }
