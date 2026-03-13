#---------------------------------------------------------------------------------
# devkitARM 3DS Makefile — 3dsfetch
#---------------------------------------------------------------------------------
APP_TITLE   := 3dsfetch
APP_AUTHOR  := viewerofall
APP_DESC    := neofetch for Nintendo 3DS
APP_VERSION := 1.0.0

TARGET      := 3dsfetch
BUILD       := build
SOURCES     := src
INCLUDES    := src

ICON        := icon.png      # 48x48 PNG
BANNER_IMG  := banner.png    # 256x128 PNG — provide before running make cia
BANNER_SND  := silent.wav    # silent audio included in repo
RSF         := app.rsf

#---------------------------------------------------------------------------------
# devkitARM toolchain
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPRO)),)
  $(error "DEVKITPRO not set — source /etc/profile.d/devkit-env.sh")
endif

DEVKITARM   := $(DEVKITPRO)/devkitARM
TOOLS       := $(DEVKITPRO)/tools/bin
include $(DEVKITARM)/3ds_rules

#---------------------------------------------------------------------------------
# Includes and lib dirs
#---------------------------------------------------------------------------------
INCLUDE     := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
               -I$(CURDIR)/$(BUILD) \
               -I$(DEVKITPRO)/libctru/include

LIBDIRS     := $(DEVKITPRO)/libctru

#---------------------------------------------------------------------------------
# Compiler flags
#---------------------------------------------------------------------------------
ARCH        := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft

CFLAGS      := -g -Wall -Wextra -O2 \
               -fomit-frame-pointer \
               $(ARCH) \
               $(INCLUDE) \
               -D__3DS__

CXXFLAGS    := $(CFLAGS) -fno-rtti -fno-exceptions
ASFLAGS     := -g $(ARCH)
LDFLAGS     := -specs=3dsx.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)
LIBS        := -lctru -lm

#---------------------------------------------------------------------------------
# Source / object discovery
#---------------------------------------------------------------------------------
CFILES      := $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.c))
OFILES      := $(CFILES:%.c=$(BUILD)/%.o)

#---------------------------------------------------------------------------------
# Targets
#---------------------------------------------------------------------------------
.PHONY: all cia clean

all: $(TARGET).3dsx

# ── SMDH ─────────────────────────────────────────────────────────────────────
$(TARGET).smdh: $(ICON)
	$(TOOLS)/smdhtool --create "$(APP_TITLE)" "$(APP_DESC)" "$(APP_AUTHOR)" $(ICON) $@

# ── 3DSX (Homebrew Launcher) ──────────────────────────────────────────────────
$(TARGET).3dsx: $(TARGET).elf $(TARGET).smdh
	$(TOOLS)/3dsxtool $< $@ --smdh=$(TARGET).smdh
	@echo "Built: $(TARGET).3dsx"

# ── ELF ──────────────────────────────────────────────────────────────────────
$(TARGET).elf: $(OFILES)
	$(CC) $(LDFLAGS) $(OFILES) $(foreach dir,$(LIBDIRS),-L$(dir)/lib) $(LIBS) -o $@

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# ── CIA (installable) ─────────────────────────────────────────────────────────
# Requires: banner.png (256x128), bannertool, makerom
cia: $(TARGET).cia

$(TARGET).banner: $(BANNER_IMG) $(BANNER_SND)
	@which bannertool > /dev/null || (echo "ERROR: bannertool not found. Install with: yay -S bannertool-git" && exit 1)
	bannertool makebanner -i $(BANNER_IMG) -a $(BANNER_SND) -o $@

$(TARGET).cia: $(TARGET).elf $(TARGET).smdh $(TARGET).banner $(RSF)
	@which makerom > /dev/null || (echo "ERROR: makerom not found. Install with: yay -S makerom-git" && exit 1)
	makerom -f cia \
		-target t \
		-exefslogo \
		-desc app:4 \
		-DAPP_ENCRYPTED=false \
		-elf $(TARGET).elf \
		-icon $(TARGET).smdh \
		-banner $(TARGET).banner \
		-rsf $(RSF) \
		-o $@
	@echo "Built: $(TARGET).cia"

# ── Clean ─────────────────────────────────────────────────────────────────────
clean:
	rm -rf $(BUILD) \
	       $(TARGET).3dsx $(TARGET).elf $(TARGET).smdh $(TARGET).map \
	       $(TARGET).cia  $(TARGET).banner
