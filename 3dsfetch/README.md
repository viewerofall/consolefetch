# 3dsfetch

> neofetch-style system info display for the Nintendo 3DS family

![License](https://img.shields.io/github/license/viewerofall/consolefetch)
![Release](https://img.shields.io/github/v/release/viewerofall/consolefetch)
![Platform](https://img.shields.io/badge/platform-Nintendo%203DS-red)

```
[ZL].-------------[ZR]      New Nintendo 3DS
 |  |~ top screen~| |       -------------------------
 |  |             | |       Firmware:  11.17.0-50U
 |__|_____________|_|       Region:    USA
 |o .-----------.  /\ |     -------------------------
 |  |           | (  )|     CPU:       ARM11 4x804MHz
 |+ |           |* \/ |     GPU:       DMP PICA200
 |  '-----------'     |     RAM:       256MB FCRAM
 |[sel]   (h)  [sta]  |     -------------------------
 '---------------------     Top LCD:   800x240 (3D)
                             Bot LCD:   320x240
              (XL)           Size:      4.88"
                             Features:  3D NFC Stereo
                             -------------------------
                             Battery:   [########..] 80%
                             SD:        8.3/29.8GB
                             NAND:      0.2/1.0GB
```

---

## Features

- Per-model ASCII art — Original 3DS, 2DS, New 3DS, New 2DS XL
- XL variants automatically tagged with `(XL)` under the art
- Firmware version with region suffix (e.g. `11.17.0-50U`)
- Full hardware spec table per model — CPU, GPU, RAM, display resolution, screen size, features
- Visual battery bar with charging indicator
- SD card and NAND storage usage
- Bottom screen shows title, GitHub link, and exit hint

## Requirements

- Any Nintendo 3DS family console
- Custom firmware — [Luma3DS](https://github.com/LumaTeam/Luma3DS) recommended
- Homebrew Launcher (for `.3dsx`) or any CIA installer (for `.cia`)

## Installation

### 3DSX — Homebrew Launcher

1. Download `3dsfetch.3dsx` from the [latest release](https://github.com/viewerofall/consolefetch/releases/latest)
2. Copy to `/3ds/3dsfetch/3dsfetch.3dsx` on your SD card
3. Launch via Homebrew Launcher

### CIA — Permanent install

1. Download `3dsfetch.cia` from the [latest release](https://github.com/viewerofall/consolefetch/releases/latest)
2. Copy to your SD card
3. Install with FBI or any CIA installer
4. Launch from the home menu

Press **START** to exit either version.

## Building from Source

### Requirements

- [devkitPro](https://devkitpro.org) with 3DS support
- Add devkitPro's pacman repo to `/etc/pacman.conf` then:

```bash
sudo pacman -S devkitARM libctru 3dstools devkit-env general-tools
```

- For CIA builds only:

```bash
yay -S bannertool-git makerom-git
```

### Build

```bash
# Source devkitPro environment if not already in your shell
source /etc/profile.d/devkit-env.sh

# 3DSX (Homebrew Launcher)
make

# CIA (permanent install) — requires banner.png (256x128) in project root
make cia

# Clean
make clean
```

## Project Structure

```
3ds/
├── Makefile
├── app.rsf          # CIA permissions spec
├── silent.wav       # Silent banner audio for CIA
├── icon.png         # 48x48 app icon
├── banner.png       # 256x128 CIA banner (not in repo — provide before make cia)
└── src/
    ├── main.c       # All logic: sysinfo, ASCII art, rendering, main loop
    └── main.h       # Structs, enums, color defines
```

## Supported Models

| Model | Art | XL Tag |
|-------|-----|--------|
| Original 3DS | Clamshell, no C-stick | — |
| Original 3DS XL | Same as above | ✓ |
| Nintendo 2DS | Flat slab, both screens | — |
| New Nintendo 3DS | Clamshell + C-stick + ZL/ZR | — |
| New Nintendo 3DS XL | Same as above | ✓ |
| New Nintendo 2DS XL | Flat clamshell + C-stick + ZL/ZR | — |

## Hardware Specs Shown

| Field | Source |
|-------|--------|
| CPU | Static per model (ARM11 2x268MHz / 4x804MHz) |
| GPU | Static (DMP PICA200, all models) |
| RAM | Static per model (128MB / 256MB FCRAM) |
| Display | Static per model (resolution + screen size) |
| Features | Static per model (3D, NFC, sound type) |
| Firmware | `osGetKernelVersion()` |
| Region | `CFGU_SecureInfoGetRegion()` |
| Battery | `PTMU_GetBatteryLevel()` + `PTMU_GetBatteryChargeState()` |
| Storage | `FSUSER_GetArchiveResource()` for SD and CTR NAND |

## License

[MIT](../../LICENSE) — © 2026 viewerofall
