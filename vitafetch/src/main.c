#include <psp2/kernel/processmgr.h>
#include <psp2/ctrl.h>
#include <vita2d.h>

#include "sysinfo.h"
#include "ascii.h"
#include "render.h"

int main(void) {
    // ── Init ─────────────────────────────────────────────────────────────────
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);

    if (render_init() < 0) {
        // If vita2d fails we can't display anything — just exit cleanly
        sceKernelExitProcess(1);
        return 1;
    }

    // ── Gather system info once ───────────────────────────────────────────────
    SysInfo info;
    sysinfo_get(&info);

    int art_lines = 0;
    const char **art = ascii_get_for_model(info.model, &art_lines);

    // ── Main loop ─────────────────────────────────────────────────────────────
    SceCtrlData pad;
    for (;;) {
        sceCtrlReadBufferPositive(0, &pad, 1);

        // EXIT on START
        if (pad.buttons & SCE_CTRL_START)
            break;

        vita2d_start_drawing();
        vita2d_clear_screen();

        render_draw(art, art_lines, &info);

        vita2d_end_drawing();
        vita2d_swap_buffers();
    }

    // ── Cleanup ───────────────────────────────────────────────────────────────
    render_fini();
    sceKernelExitProcess(0);
    return 0;
}
