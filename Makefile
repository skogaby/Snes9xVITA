TARGET = Snes9xVITA
OBJS = src/apu.o src/bsx.o src/c4emu.o src/cheats.o src/controls.o src/cpu.o src/cpuexec.o \
	   src/dsp.o src/fxemu.o src/globals.o src/memmap.o src/obc1.o src/ppu.o src/sa1.o \
	   src/sdd1.o src/seta.o src/snapshot.o src/spc7110.o src/srtc.o src/tile.o \
	   libretro/libretro.o libretro/memory_stream.o vita/font_data.o vita/font.o vita/file_chooser.o \
       vita/utils.o vita/vita_input.o vita/vita_audio.o vita/vita_video.o vita/vita_menu.o vita/main.o

LIBS = -lpsplib -lSceDisplay_stub -lSceGxm_stub -lScePower_stub -lSceCtrl_stub -lSceTouch_stub \
       -lSceAudio_stub -lScePower_stub -lSceRtc_stub -lc_stub -lpng16 -lz -lm_stub -lvita2d

PREFIX   = $(DEVKITARM)/bin/arm-none-eabi
CC       = $(PREFIX)-gcc
CFLAGS   = -Wall -O3 -specs=psp2.specs -DVITA
CFLAGS  += -w -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables -ftree-vectorize \
		   -mfloat-abi=hard -ffast-math -fsingle-precision-constant -ftree-vectorizer-verbose=2 -fopt-info-vec-optimized -funroll-loops \
           -mword-relocations -fno-rtti -Wno-deprecated -Wno-comment -Wno-sequence-point
CFLAGS  += -DHAVE_STRINGS_H -DHAVE_STDINT_H -DHAVE_INTTYPES_H -D__LIBRETRO__ -DRIGHTSHIFT_IS_SAR -DINLINE=inline -DCORRECT_VRAM_READS \
           -DFRONTEND_SUPPORTS_RGB565 -DCPU_SHUTDOWN
ASFLAGS = $(CFLAGS)

CORE_DIR := .
INCFLAGS := -I$(CORE_DIR)/libretro -I$(CORE_DIR)/src

all: $(TARGET)_fixup.elf

%_fixup.elf: %.elf
	psp2-fixup -q -S $< $@

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $(INCFLAGS) $^ $(LIBS) -o $@

clean:
	@rm -rf $(TARGET)_fixup.elf $(TARGET).elf $(OBJS)

copy: $(TARGET)_fixup.elf
	@cp $(TARGET)_fixup.elf ~/shared/vitasample.elf
	@echo "Copied!"