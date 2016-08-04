TARGET = Snes9xVITA

PSP_APP_NAME=Snes9xVITA
PSP_APP_VER=1.1.0

LIBRETRO=libretro
CORE=src
VITA=vita

INCLUDES = -I$(LIBRETRO) -I$(CORE) -I$(VITA)

BUILD_APP = $(CORE)/apu.o $(CORE)/bsx.o $(CORE)/c4emu.o $(CORE)/cheats.o $(CORE)/controls.o $(CORE)/cpu.o $(CORE)/cpuexec.o \
	        $(CORE)/dsp.o $(CORE)/fxemu.o $(CORE)/globals.o $(CORE)/memmap.o $(CORE)/obc1.o $(CORE)/ppu.o $(CORE)/sa1.o \
	        $(CORE)/sdd1.o $(CORE)/seta.o $(CORE)/snapshot.o $(CORE)/spc7110.o $(CORE)/srtc.o $(CORE)/tile.o
BUILD_PORT = $(LIBRETRO)/libretro.o $(LIBRETRO)/memory_stream.o $(VITA)/utils.o $(VITA)/vita_input.o $(VITA)/vita_audio.o \
             $(VITA)/vita_video.o $(VITA)/vita_menu.o $(VITA)/main.o

OBJS = $(BUILD_APP) $(BUILD_PORT)

LIBS = -lpsplib -lvita2d -lfreetype -lpng -lz -lm -lSceCommonDialog_stub -lSceSysmodule_stub -lSceDisplay_stub -lSceGxm_stub 	\
	-lSceCtrl_stub -lSceAudio_stub -lSceRtc_stub -lScePower_stub -lSceAppUtil_stub \
    -lSceCommonDialog_stub

DEFINES = -DPSP -DVITA -DPSP_APP_NAME=\"$(PSP_APP_NAME)\" -DPSP_APP_VER=\"$(PSP_APP_VER)\" -DHAVE_STRINGS_H \
          -DHAVE_STDINT_H -DHAVE_INTTYPES_H -D__LIBRETRO__ -DRIGHTSHIFT_IS_SAR -DINLINE=inline -DCORRECT_VRAM_READS \
          -DFRONTEND_SUPPORTS_RGB565 -DCPU_SHUTDOWN

PREFIX  = arm-vita-eabi
AS = $(PREFIX)-as
CC = $(PREFIX)-gcc
CXX		= $(PREFIX)-g++
READELF = $(PREFIX)-readelf
OBJDUMP = $(PREFIX)-objdump
CFLAGS  = -Wl,-q -O3 $(INCLUDES) $(DEFINES) -fno-exceptions \
		  -fno-unwind-tables -fno-asynchronous-unwind-tables -ftree-vectorize \
		  -mfloat-abi=hard -ffast-math -fsingle-precision-constant -ftree-vectorizer-verbose=2 \
          -fopt-info-vec-optimized -funroll-loops
CXXFLAGS = $(CFLAGS) -fno-rtti -Wno-deprecated -Wno-comment -Wno-sequence-point
ASFLAGS = $(CFLAGS)



all: eboot.bin

eboot.bin: $(TARGET).velf
	vita-make-fself $(TARGET).velf eboot.bin
	vita-mksfoex -s TITLE_ID=SKOG00001 "Snes9xVITA" param.sfo

$(TARGET).velf: $(TARGET).elf
		$(PREFIX)-strip -g $<
		vita-elf-create  $< $@ 

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $(ASFLAGS) $^ $(LIBS) -o $@

clean:
	@rm -rf $(TARGET).elf $(TARGET).velf $(OBJS) $(DATA)/*.h

copy: $(TARGET).velf
	@cp $(TARGET).velf ~/shared/vitasample.elf
	@echo "Copied!"
    
