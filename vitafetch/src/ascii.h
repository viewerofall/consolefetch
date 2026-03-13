#pragma once

#include "sysinfo.h"

// Max lines any ASCII art block will have
#define ASCII_MAX_LINES 16
#define ASCII_MAX_WIDTH 40

/**
 * Returns a null-terminated array of strings (the ASCII art lines)
 * for the given model, and sets *line_count to the number of lines.
 * The returned pointer is to static storage — do not free.
 */
const char **ascii_get_for_model(VitaModel model, int *line_count);
