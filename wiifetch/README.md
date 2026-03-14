# wiifetch

> neofetch for the Nintendo Wii

A native Wii homebrew fetch tool written in C using libogc. Displays system info alongside an ASCII Wii logo with a CRT scanline startup animation. Detects stock Wii, modded Wii, and vWii (Wii U backwards compatibility mode) automatically.

Inspiration & bits of code from [NiioFetch](https://github.com/abdelali221/NiioFetch) by abdelali221.

![Examplar icon for it](wiifetch/icon_animated.gif)


---

## Features

- **Console detection** — automatically detects stock Wii, modded Wii (custom IOS 200+), and vWii (IOS 57) and switches logo and hostname accordingly
- **CRT scanline animation** — logo draws in two interlaced passes like a CRT phosphor scanline fill, then wipes and types the prompt before the fetch renders
- **System info** — IOS version, region, language, video mode, aspect ratio, CPU, GPU, RAM layout, NAND size, SD card free/total, uptime
- **Color output** — cyan/white themed with a color palette block at the bottom
- **Safe for 4:3** — layout fits within NTSC/PAL safe area, no overflow

### Info displayed

| Field | Source |
|---|---|
| OS | Homebrew Channel (variant per console type) |
| IOS | `IOS_GetVersion()` |
| Region | `CONF_GetRegion()` |
| Language | `CONF_GetLanguage()` |
| Video | `CONF_GetVideo()` |
| Aspect | `CONF_GetAspectRatio()` |
| CPU | 729 MHz IBM Broadway (PPC) |
| GPU | 243 MHz ATI Hollywood |
| RAM | 64 MB MEM1 + 24 MB MEM2 |
| NAND | 512 KB internal |
| SD | Free/total via `statvfs` |
| Uptime | `SYS_Time()` / `TB_TIMER_CLOCK` |

---

## Requirements

- [devkitPro](https://devkitpro.org) with the `wii-dev` package group

```bash
dkp-pacman -S wii-dev
```

This pulls in devkitPPC, libogc, libfat, libwiiuse, and libbte — everything the Makefile needs.

---

## Building

```bash
cd wiifetch
make
```

Output is `wiifetch.dol`. Rename to `boot.dol` when copying to SD.

```bash
make clean   # remove build artifacts
```

---

## Installation

Copy to your SD card:

```
sd:/apps/wiifetch/
├── boot.dol     ← 
├── meta.xml
└── icon.png     ← 128x128 PNG
```

Launch from the Homebrew Channel. Press **HOME** (Wiimote) or **START** (GameCube controller) to exit.

---

## Console variants

| Detected as | IOS range | Logo | Hostname |
|---|---|---|---|
| Nintendo Wii | stock IOS (< 57, not 200+) | Standard Wii | `user@wii` |
| Wii (modded) | custom IOS 200–255 | Wii + modchip section | `user@mwii` |
| Wii U (vWii) | IOS 57 | Wii + vWii suffix | `user@vwii` |

---

## License

MIT 2026 - joemo
