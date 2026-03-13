#pragma once

#include "sysinfo.h"
#include "ascii.h"

// vita2d screen dimensions
#define SCREEN_W 960
#define SCREEN_H 544

// Layout config
#define ART_X        20     // Left margin for ASCII art
#define ART_Y        60     // Top margin
#define INFO_X       380    // Info column X — leave room for 36-char art at CHAR_W=10
#define INFO_Y       60     // Info column Y
#define LINE_HEIGHT  28     // Pixels between lines

// Colors (RGBA8 packed: AABBGGRR in vita2d convention)
#define COLOR_WHITE     0xFFFFFFFF
#define COLOR_CYAN      0xFFFFFF00  // vita2d uses ABGR so cyan = 0xFFFFFF00
#define COLOR_YELLOW    0xFF00FFFF
#define COLOR_GREEN     0xFF00FF00
#define COLOR_ORANGE    0xFF0099FF
#define COLOR_PINK      0xFFFF00FF
#define COLOR_DIM       0xFFAAAAAA
#define COLOR_BG        0xFF1A1A2E  // dark navy

/**
 * Initialise vita2d and load the system font.
 * Must be called before render_draw().
 * Returns 0 on success.
 */
int render_init(void);

/**
 * Draw one full frame: ASCII art on the left, info on the right.
 * Call between vita2d_start_drawing / vita2d_end_drawing.
 */
void render_draw(const char **art, int art_lines, const SysInfo *info);

/**
 * Clean up vita2d resources.
 */
void render_fini(void);
