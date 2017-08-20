#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules

export TARGET		:=	bootstrap
export TOPDIR		:=	$(CURDIR)


.PHONY: checkarm7 checkarm9

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all: checkarm7 checkarm9 ismat.dat _DS_MENU.DAT ez5sys.bin akmenu4.nds TTMENU.DAT _BOOT_MP.NDS bootstrap.cia

_DS_MENU.DAT	:	$(TARGET).nds
	@dlditool r4tfv2.dldi $<
	@r4denc $< $@

ez5sys.bin	:	$(TARGET).nds
	@cp $< $@
	@dlditool EZ5V2.dldi $@

akmenu4.nds	:	$(TARGET).nds
	@cp $< $@
	@dlditool ak2_sd.dldi $@

TTMENU.DAT	:	$(TARGET).nds
	@cp $< $@
	@dlditool TTCARDIOLibrary.dldi $@

_BOOT_MP.NDS	:	$(TARGET).nds
	@cp $< $@
	@dlditool mpcf.dldi $@

ismat.dat	:	$(TARGET).nds
	@cp $< $@
	@dlditool mati.dldi $@

#---------------------------------------------------------------------------------
# Building separate .dsi and .nds binaries here is done purely to allow the .nds
# binary to be used to replace various flashcart boot menus.
#
# This is *NOT* for general use, standard .nds files with a full header are DSi
# hybrid binaries.
#
#---------------------------------------------------------------------------------
bootstrap.cia: bootstrap.dsi
	make_cia --srl=bootstrap.dsi

#---------------------------------------------------------------------------------
bootstrap.dsi : $(TARGET).arm7.elf $(TARGET).arm9i.elf
	ndstool -u 00030004 -g HBLA -b bootstrap.bmp "nds-hb-menu bootstrap;built with devkitARM;http://devkitpro.org" -c $@ -7 $(TARGET).arm7.elf -9 $(TARGET).arm9i.elf

#---------------------------------------------------------------------------------
$(TARGET).nds	:	$(TARGET).arm7.elf $(TARGET).arm9.elf
	ndstool	-h 0x200 -c $(TARGET).nds -7 $(TARGET).arm7.elf -9 $(TARGET).arm9.elf

checkarm7:
	$(MAKE) -C arm7

checkarm9:
	$(MAKE) -C arm9

#---------------------------------------------------------------------------------
$(TARGET).arm7.elf:
	$(MAKE) -C arm7
	
#---------------------------------------------------------------------------------
$(TARGET).arm9.elf:
	$(MAKE) -C arm9


#---------------------------------------------------------------------------------
clean:
	$(MAKE) -C arm9 clean
	$(MAKE) -C arm7 clean
	rm -f $(TARGET).nds $(TARGET).arm7.elf $(TARGET).arm9.elf _DS_MENU.DAT ez5sys.bin akmenu4.nds TTMENU.DAT _BOOT_MP.NDS
	rm -f bootstrap.dsi $(TARGET).arm9i.elf

