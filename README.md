# consolefetch

> neofetch-style system info displays for retro and handheld consoles

![License](https://img.shields.io/github/license/viewerofall/consolefetch)
![Platform](https://img.shields.io/badge/platforms-PS%20Vita%20%7C%203DS-blue)

A collection of native homebrew fetch tools — one per platform, each written in C using the platform's own SDK. Displays hardware info, firmware, storage, and a per-model ASCII art representation of the device.

---

## Platforms

| Platform | Folder | Format | Store |
|----------|--------|--------|-------|
| PlayStation Vita / PS TV | [`vita/`](vita/) | `.vpk` | [VitaDB](https://vitadb.rinnegatamante.it) |
| Nintendo 3DS family | [`3ds/`](3ds/) | `.3dsx` `.cia` | [Universal-DB](https://db.universal-team.net) |

---

## vita/

**vitafetch** — neofetch for PS Vita and PlayStation TV

- Detects Vita 1000, Vita 2000, and PS TV with unique ASCII art per model
- Shows firmware, CFW (HENkaku / Enso / Stock), region, CPU/GPU clocks, battery, ux0 and ur0 storage
- Built with vitasdk and vita2d

→ [Full details](vita/README.md)

---

## 3ds/

**3dsfetch** — neofetch for the Nintendo 3DS family

- Detects Original 3DS, 2DS, New 3DS, and New 2DS XL with unique ASCII art per model
- XL variants shown with an (XL) tag under the art
- Shows firmware, region, CPU, GPU, RAM, display specs, battery, SD and NAND storage
- Bottom screen shows GitHub info and exit hint
- Built with devkitARM and libctru

→ [Full details](3ds/README.md)

---

## Building

Each platform has its own toolchain and Makefile. See the README in each subfolder for setup and build instructions.

---

## Planned

- `wii/` — wiifetch for Nintendo Wii

---

## License

[MIT](LICENSE) — © 2026 viewerofall
