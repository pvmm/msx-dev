export PATH := $(PWD)/../../bin:$(PATH)
MAKE := make
Z80_CC := sdcc
Z80_AS := sdasz80
HEXTOOL := hex2bin
#HEXTOOL := ./tools/MSXhex

BUILDDIR := build
TARGET := $(BUILDDIR)/main
CODE := 0x4000
DATA := 0xc000
ROM_MAX := 0x8000
Z80_CFLAGS := --less-pedantic -mz80 --Werror --fsigned-char --std-sdcc99 --opt-code-size  --fomit-frame-pointer $(EXTRA_Z80_CFLAGS) -I. -I../../common -DUSE_DEBUG_MODE
Z80_LDFLAGS := --no-std-crt0

C_SOURCES := $(wildcard *.c)
ASM_SOURCES := $(wildcard *.asm)
HEADERS := $(wildcard *.h)
OBJECTS := $(patsubst %.c,$(BUILDDIR)/%.rel,$(C_SOURCES)) $(patsubst %.asm,$(BUILDDIR)/%.rel,$(ASM_SOURCES))
Z80_LIBS :=

.PHONY: all clean openmsx run
all: $(TARGET).rom
$(TARGET).rom: $(OBJECTS) $(BUILDDIR)/crt0.rel $(BUILDDIR)
	$(Z80_CC) $(Z80_CFLAGS) $(Z80_LDFLAGS) $(Z80_LIBS) --code-loc $(CODE) --data-loc $(DATA) $(BUILDDIR)/crt0.rel $(OBJECTS) -o $(TARGET).ihx
	#$(HEXTOOL) $(TARGET).ihx -o $(TARGET).rom -e ROM -p 0xFF -s 0x4000 -l $(ROM_MAX)
	$(HEXTOOL) -e bin -p FF -l $(ROM_MAX) $(TARGET).ihx
	@cp $(TARGET).bin $(TARGET).rom

$(BUILDDIR):
	@mkdir $(BUILDDIR)

openmsx: run

run: $(TARGET).rom
	DEBUG=1 openmsx -carta $(TARGET).rom -machine C-BIOS_MSX2 -script debug.tcl

$(BUILDDIR)/%.rel: ../../common/%.c $(BUILDDIR) ../../common/common.h
	$(Z80_CC) $(Z80_CFLAGS) $(SCREEN_LAYOUT) -c $< -o $@

$(BUILDDIR)/%.rel: %.c $(HEADERS) $(BUILDDIR)
	$(Z80_CC) $(Z80_CFLAGS) $(SCREEN_LAYOUT) -c $< -o $@

$(BUILDDIR)/%.rel: %.asm $(HEADERS) $(BUILDDIR)
	$(Z80_AS) -g -o $@ $<

$(BUILDDIR)/crt0.rel: ../crt0/crt0.z80
	$(Z80_AS) -g -o $@ $<

clean:
	@rm -rf $(OBJECTS) build
