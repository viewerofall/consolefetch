# consolefetch

> neofetch-style system info displays for retro and handheld consoles

[![License](https://img.shields.io/github/license/viewerofall/consolefetch)](https://github.com/viewerofall/consolefetch/blob/main/LICENSE)
[![Platform](https://img.shields.io/badge/platforms-PS%20Vita%20%7C%203DS%20%7C%20Wii-blue)](https://github.com/viewerofall/consolefetch)

A collection of native homebrew fetch tools — one per platform, each written in C using the platform's own SDK. Displays hardware info, firmware, storage, and a per-model ASCII art representation of the device.

---

## Platforms

| Platform | Folder | Format | Store |
|---|---|---|---|
| PlayStation Vita / PS TV | [`vitafetch/`](./vitafetch) | `.vpk` | [VitaDB](https://vitadb.rinnegatamante.it) |
| Nintendo 3DS family | [`3dsfetch/`](./3dsfetch) | `.3dsx` `.cia` | [Universal-DB](https://db.universal-team.net) |
| Nintendo Wii / vWii / modded Wii | [`wiifetch/`](./wiifetch) | `.dol` | Homebrew Browser |
### All apps have yet to be added to a store, please wait for them to be incorporated if you want them via that way
---

## vitafetch/

**vitafetch** — neofetch for PS Vita and PlayStation TV

- Detects Vita 1000, Vita 2000, and PS TV with unique ASCII art per model
- Shows firmware, CFW (HENkaku / Enso / Stock), region, CPU/GPU clocks, battery, ux0 and ur0 storage
- Built with vitasdk and vita2d

→ [Full details](./vitafetch/README.md)

---

## 3dsfetch/

**3dsfetch** — neofetch for the Nintendo 3DS family

- Detects Original 3DS, 2DS, New 3DS, and New 2DS XL with unique ASCII art per model
- XL variants shown with an (XL) tag under the art
- Shows firmware, region, CPU, GPU, RAM, display specs, battery, SD and NAND storage
- Bottom screen shows GitHub info and exit hint
- Built with devkitARM and libctru

→ [Full details](./3dsfetch/README.md)

---

## wiifetch/

**wiifetch** — neofetch for the Nintendo Wii

- Detects stock Wii, modded Wii (custom IOS), and vWii (Wii U backwards compat) with a variant logo per mode
- CRT scanline startup animation with typed prompt sequence
- Shows IOS version, region, language, video mode, aspect ratio, CPU, GPU, RAM, NAND, SD storage, and uptime
- Forked from [NiioFetch](https://github.com/abdelali221/NiioFetch) by abdelali221
- Built with devkitPPC, libogc, libfat

→ [Full details](./wiifetch/README.md)

---

## Building

Each platform has its own toolchain and Makefile. See the README in each subfolder for setup and build instructions.

---

## License

[MIT](./LICENSE) — © 2026 viewerofall
