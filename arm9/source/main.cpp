#include <nds.h>
#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>

#include <string.h>
#include <unistd.h>

#include "nds_loader_arm9.h"
#include "file_browse.h"

const char DEFAULT_FILE[] = "fat0:/_BOOT_DS.NDS";

void stop (void) 
{
	while (1) {
		swiWaitForVBlank();
	}
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------

	struct stat st;
	const char* argarray[2];
	char filePath[MAXPATHLEN * 2];
	int pathLen;
	std::string filename;

	videoSetMode(0);	//not using the main screen

	// Subscreen as a console
	videoSetModeSub(MODE_0_2D);
	vramSetBankH(VRAM_H_SUB_BG);
	consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 15, 0, false, true);

	iprintf("%d argument(s)\n",argc);
	iprintf ("Init'ing FAT...");
	if (fatInitDefault()) {
		iprintf ("okay\n");
	} else {
		iprintf ("fail!\n");
		stop();
	} 
	
//	if (stat (DEFAULT_FILE, &st) >= 0) {
//		filename = DEFAULT_FILE;
//	} else {
		filename = browseForFile (".nds");
//	}

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
