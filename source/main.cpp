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
#include <nds.h>
#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>

#include <string.h>
#include <unistd.h>

#include "nds_loader_arm9.h"
#include "file_browse.h"

#include "hbmenu_banner.h"

void stop (void) 
{
	while (1) {
		swiWaitForVBlank();
	}
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------

	const char* argarray[2];
	char filePath[MAXPATHLEN * 2];
	int pathLen;
	std::string filename;

	videoSetMode(MODE_5_2D);
	vramSetBankA(VRAM_A_MAIN_BG);

	// set up our bitmap background
	bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0,0);
	
	//decompress(hbmenu_bannerBitmap, BG_GFX,  LZ77Vram);
	dmaCopy(hbmenu_bannerBitmap, BG_GFX, hbmenu_bannerBitmapLen);

	// Subscreen as a console
	videoSetModeSub(MODE_0_2D);
	vramSetBankH(VRAM_H_SUB_BG);
	consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 15, 0, false, true);

	iprintf ("Init'ing FAT...");
	if (fatInitDefault()) {
		iprintf ("okay\n");
	} else {
		iprintf ("fail!\n");
		stop();
	} 
	
	filename = browseForFile (".nds");
	
	installBootStub();

	// Construct a command line if we weren't supplied with one
	getcwd (filePath, MAXPATHLEN);
	pathLen = strlen (filePath);
	strcpy (filePath + pathLen, filename.c_str());
	argarray[0] = filePath;

	iprintf ("Running %s\n", argarray[0]);
	int err = runNdsFile (argarray[0], 1, argarray);
	
	iprintf ("Start failed. Error %i\n", err);
	
	stop();

	return 0;
}
