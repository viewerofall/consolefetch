#include "ascii.h"

// All art is exactly 30 characters wide per line.
// At CHAR_W=10px that is 300px — safely left of INFO_X=380.

// ─── Vita 1000 (OLED) ────────────────────────────────────────────────────────
// ROUNDED corners (. top, ' bottom). Analog sticks (o) sit ON the curved top
// edge. Face buttons /\ ( ) \/ are between screen border and outer shell.
// Bottom edge curves with ' — immediately distinct from 2000.
//
static const char *art_vita1000[] = {
    //   [123456789012345678901234567890]
    ".-(o)--------------------(o)-.",
    "|[L]  .---------------.  [R] |",
    "|     |               |  /\\  |",
    "|(+)  |               | ( )  |",
    "|     |               |  \\/  |",
    "|     '---------------'      |",
    "| [SEL]    (PS)    [STA]     |",
    "'----------------------------'",
    NULL
};
static const int art_vita1000_lines = 8;

// ─── Vita 2000 (Slim) ────────────────────────────────────────────────────────
// FLAT/SHARP top [L]___[R] and bottom |____| — no curves anywhere.
// Sticks (o) appear on row 2 flanking the screen, not on the top edge.
// The squared silhouette is the immediate visual tell vs the 1000.
//
static const char *art_vita2000[] = {
    //   [123456789012345678901234567890]
    "[L]________________________[R]",
    "|(o) .------------------.(o) |",
    "|    |                  |    |",
    "|(+) |                  | /\\ |",
    "|    |                  |( ) |",
    "|    |                  | \\/ |",
    "|    '------------------'    |",
    "| [SEL]    (PS)    [STA]     |",
    "|____________________________|",
    NULL
};
static const int art_vita2000_lines = 9;

// ─── PlayStation TV ───────────────────────────────────────────────────────────
static const char *art_pstv[] = {
    //   [123456789012345678901234567890]
    ".____________________________.",
    "|                            |",
    "|      PlayStation  TV       |",
    "|   (PWR)        [USB]       |",
    "|____________________________|",
    "  |HDMI| LAN | DC IN |       ",
    "  '----'-----'-------'       ",
    NULL
};
static const int art_pstv_lines = 7;

// ─── Unknown fallback ─────────────────────────────────────────────────────────
static const char *art_unknown[] = {
    " .----------.",
    " |   VITA   |",
    " |    ??    |",
    " '----------'",
    NULL
};
static const int art_unknown_lines = 4;

// ─── Public API ───────────────────────────────────────────────────────────────

const char **ascii_get_for_model(VitaModel model, int *line_count) {
    switch (model) {
        case MODEL_VITA_1000:
            if (line_count) *line_count = art_vita1000_lines;
            return art_vita1000;
        case MODEL_VITA_2000:
            if (line_count) *line_count = art_vita2000_lines;
            return art_vita2000;
        case MODEL_PSTV:
            if (line_count) *line_count = art_pstv_lines;
            return art_pstv;
        default:
            if (line_count) *line_count = art_unknown_lines;
            return art_unknown;
    }
}
