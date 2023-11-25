/*-----------------------------------------------------------------
 Copyright (C) 2005 - 2010
	Michael "Chishm" Chisholm
	Dave "WinterMute" Murphy

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

------------------------------------------------------------------*/
#include <string.h>
#include <nds.h>
#include <nds/arm9/dldi.h>
#include <calico/arm/common.h>
#include <calico/nds/env.h>
#include <sys/stat.h>
#include <limits.h>

#include <unistd.h>
#include <fat.h>

#include "load_bin.h"

#ifndef _NO_BOOTSTUB_
#include "bootstub_bin.h"
#endif

#include "nds_loader_arm9.h"

/*
	b	startUp

storedFileCluster:
	.word	0x0FFFFFFF		@ default BOOT.NDS
wantToPatchDLDI:
	.word	0x00000001		@ by default patch the DLDI section of the loaded NDS
@ Used for passing arguments to the loaded app
argStart:
	.word	_end - _start
argSize:
	.word	0x00000000
dldiOffset:
	.word	_dldi_start - _start
dsiSD:
	.word	0
dsiMode:
	.word	0
*/

typedef struct BootLdrHeader {
	u32 entrypoint;
	u32 storedFileCluster;
	u32 wantToPatchDldi;
	u32 argStart;
	u32 argSize;
	u32 dldiOffset;
	u32 hasTwlSd;
	u32 isTwlMode;
} BootLdrHeader;

static bool dldiPatchLoader(BootLdrHeader* loader)
{
	alignas(ARM_CACHE_LINE_SZ) static u8 io_dldi_data[DLDI_MAX_ALLOC_SZ];
	DLDI_INTERFACE* io = (DLDI_INTERFACE*)io_dldi_data;

	if (!dldiDumpInternal(io)) {
		// No DLDI patch
		return false;
	}

	DLDI_INTERFACE* area = (void*)((u8*)loader + loader->dldiOffset);
	return dldiApplyPatch(area, io);
}

static eRunNdsRetCode runNds (const void* loader, u32 loaderSize, u32 cluster, int argc, const char** argv)
{
	char* argStart;
	u16* argData;
	u16 argTempVal = 0;
	int argSize;
	const char* argChar;

	// Direct CPU access to VRAM bank C
	VRAM_C_CR = VRAM_ENABLE | VRAM_C_LCD;
	// Load the loader/patcher into the correct address
	armCopyMem32 (VRAM_C, loader, loaderSize);

	BootLdrHeader* hdr = (BootLdrHeader*)MM_VRAM_C;

	// Set the parameters for the loader
	hdr->storedFileCluster = cluster;
	hdr->isTwlMode = systemIsTwlMode();

	if(argv[0][0]=='s' && argv[0][1]=='d') {
		hdr->wantToPatchDldi = 0;
		hdr->hasTwlSd = 1;
	} else {
		hdr->hasTwlSd = 0;
	}

	// Give arguments to loader
	argStart = (char*)MM_VRAM_C + hdr->argStart;
	argStart = (char*)(((int)argStart + 3) & ~3);	// Align to word
	argData = (u16*)argStart;
	argSize = 0;

	for (; argc > 0 && *argv; ++argv, --argc)
	{
		for (argChar = *argv; *argChar != 0; ++argChar, ++argSize)
		{
			if (argSize & 1)
			{
				argTempVal |= (*argChar) << 8;
				*argData = argTempVal;
				++argData;
			}
			else
			{
				argTempVal = *argChar;
			}
		}
		if (argSize & 1)
		{
			*argData = argTempVal;
			++argData;
		}
		argTempVal = 0;
		++argSize;
	}
	*argData = argTempVal;

	hdr->argStart = (uptr)argStart - MM_VRAM_C;
	hdr->argSize = argSize;

	if(hdr->wantToPatchDldi) {
		// Patch the loader with a DLDI for the card
		if (!dldiPatchLoader((BootLdrHeader*)VRAM_C)) {
			return RUN_NDS_PATCH_DLDI_FAILED;
		}
	}

	// Give the VRAM to the ARM7
	VRAM_C_CR = VRAM_ENABLE | VRAM_C_ARM7_0x06000000;

	// Reset into a passme loop
	*((vu32*)0x02FFFFFC) = 0;
	*(u32*)&g_envAppNdsHeader->title[4] = 0xE59FF018;
	g_envAppNdsHeader->arm9_entrypoint = (u32)&g_envAppNdsHeader->title[4];
	g_envAppNdsHeader->arm7_entrypoint = 0x06000000;
	g_envAppTwlHeader->arm7_mbk_map_settings[0] = mbkMakeMapping(MM_TWLWRAM_MAP, MM_TWLWRAM_MAP+MM_TWLWRAM_BANK_SZ, MbkMapSize_256K);
	g_envExtraInfo->pm_chainload_flag = 1;

	exit(0);
}

eRunNdsRetCode runNdsFile (const char* filename, int argc, const char** argv)  {
	struct stat st;
	char filePath[PATH_MAX];
	int pathLen;
	const char* args[1];

	if (stat (filename, &st) < 0) {
		return RUN_NDS_STAT_FAILED;
	}

	if (argc <= 0 || !argv) {
		// Construct a command line if we weren't supplied with one
		if (!getcwd (filePath, PATH_MAX)) {
			return RUN_NDS_GETCWD_FAILED;
		}
		pathLen = strlen (filePath);
		strcpy (filePath + pathLen, filename);
		args[0] = filePath;
		argv = args;
	}

	bool havedsiSD = false;

	if(argv[0][0]=='s' && argv[0][1]=='d') havedsiSD = true;

	installBootStub(havedsiSD);

	return runNds (load_bin, load_bin_size, st.st_ino, argc, argv);
}

bool installBootStub(bool havedsiSD) {
#ifndef _NO_BOOTSTUB_
	void* bootstub = g_envNdsBootstub;
	BootLdrHeader *bootloader = (BootLdrHeader*)((u8*)bootstub+bootstub_bin_size);

	armCopyMem32(bootstub,bootstub_bin,bootstub_bin_size);
	armCopyMem32(bootloader,load_bin,load_bin_size);
	bool ret = false;

	bootloader->isTwlMode = systemIsTwlMode();
	if( havedsiSD) {
		ret = true;
		bootloader->wantToPatchDldi = 0;
		bootloader->hasTwlSd = 1;
	} else {
		ret = dldiPatchLoader(bootloader);
	}

	g_envNdsBootstub->arm9_entrypoint = (void*)((u32)bootstub+(u32)g_envNdsBootstub->arm9_entrypoint);
	g_envNdsBootstub->arm7_entrypoint = (void*)((u32)bootstub+(u32)g_envNdsBootstub->arm7_entrypoint);
	*(u32*)(g_envNdsBootstub+1) = load_bin_size;

	DC_FlushAll();

	return ret;
#else
	return true;
#endif

}
