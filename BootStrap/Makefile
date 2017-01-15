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
all: checkarm7 checkarm9 _DS_MENU.DAT ez5sys.bin akmenu4.nds TTMENU.DAT _BOOT_MP.NDS bootstrap_dsi.nds

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

# CIA code to go here

bootstrap_dsi.nds : $(TARGET).arm7.elf $(TARGET).arm9i.elf
	ndstool -c $@ -7 $(TARGET).arm7.elf -9 $(TARGET).arm9i.elf



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

