# vitafetch

A neofetch-style system info display for the PlayStation Vita and PlayStation TV.
Written in C using [vitasdk](https://vitasdk.org/).

**The first neofetch-style homebrew for PS Vita.**

```
   .--'''''''''''''''''''''-. 
  / _  _________________  _ \      PS Vita 1000 (OLED)
 | (_)| |             | |(_)|      ─────────────────────────────
 |    | |             | |   |      Firmware:   3.74
 | O  | |             | |  O|      CFW:        HENkaku Enso
 |    | |_____________| |   |      Region:     USA
 |  __                 __   |      ─────────────────────────────
 | /  \  d(+)  (O)(X)  /  \ |      CPU:        333 MHz
 | \__/  ___  _____  \__/  |      GPU:        111 MHz
 |      |___|/ ___ \      |       ─────────────────────────────
 |      SELECT START      |       Battery:    82%
  \_________________________/      ─────────────────────────────
                                   ux0 (MC):  12.4 GB / 32.0 GB
                                   ur0 (SYS): 0.8 GB / 1.0 GB
```

## Supported Models

| Model | ASCII Art |
|-------|-----------|
| PS Vita 1000 (OLED) | Rounded clamshell |
| PS Vita 2000 (Slim) | Sharp-edged clamshell |
| PlayStation TV | Set-top box |

## Info Displayed

- Model name
- Firmware version
- CFW detection (HENkaku / Enso / Stock)
- Region
- CPU clock (MHz)
- GPU clock (MHz)
- Battery percentage + charging state
- ux0 and ur0 free/total storage

## Building

### Requirements

- [vitasdk](https://vitasdk.org/) installed and `$VITASDK` set
- `cmake` 3.19+
- `vita2d`, `libpng`, `libjpeg`, `libfreetype` (all available via vdpm)

```bash
# Install dependencies via vdpm (vitasdk package manager)
$VITASDK/bin/vdpm libvita2d

# Build
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=$VITASDK/share/vita.toolchain.cmake
make
```

This produces `vitafetch.vpk` in the build directory.

## Installing

Transfer `vitafetch.vpk` to your Vita via VitaShell or FTP and install it.
Requires HENkaku or Enso CFW.

## Controls

| Button | Action |
|--------|--------|
| START  | Exit   |

## Project Structure

```
vitafetch/
├── CMakeLists.txt
├── README.md
├── sce_sys/                    # LiveArea assets (icon, background)
│   └── livearea/contents/
└── src/
    ├── main.c                  # Entry point + main loop
    ├── sysinfo.c / .h          # Hardware info queries (vitasdk APIs)
    ├── ascii.c  / .h           # Per-model ASCII art
    └── render.c / .h           # vita2d drawing + layout
```
