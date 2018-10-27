/*-----------------------------------------------------------------
 Copyright (C) 2005 - 2013
	Michael "Chishm" Chisholm
	Dave "WinterMute" Murphy
	Claudio "sverx"

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
#include <limits.h>

#include <string.h>
#include <unistd.h>

#include "nds_loader_arm9.h"
#include "file_browse.h"
#include "plugin.h"

#include "hbmenu_banner.h"

#include "iconTitle.h"

using namespace std;

static const char PLUGIN_DIR[] = "/nds/plugins";
static const size_t PLUGIN_DIR_LEN = sizeof(PLUGIN_DIR) - 1;
static const char NDS_EXT[] = ".nds";
static const size_t NDS_EXT_LEN = sizeof(NDS_EXT) - 1;

//---------------------------------------------------------------------------------
void stop (void) {
//---------------------------------------------------------------------------------
	while (1) {
		swiWaitForVBlank();
	}
}

char cwd[PATH_MAX];

//---------------------------------------------------------------------------------
static char *getAbsPath(const char *filename) {
//---------------------------------------------------------------------------------
	if (filename[0] == '/') {
		return strdup(filename);
	} else {
		getcwd(cwd, sizeof(cwd));
		size_t cwdPathLen = strlen(cwd);
		size_t filenameLen = strlen(filename);
		char *path = (char *)malloc(cwdPathLen + filenameLen + 1);
		strcpy(path, cwd);
		strcpy(path + cwdPathLen, filename);
		path[cwdPathLen + filenameLen] = '\0';
		return path;
	}
}

//---------------------------------------------------------------------------------
static bool readArgvFile(vector<char*>& argarray, const std::string& filename) {
//---------------------------------------------------------------------------------
	FILE *argfile = fopen(filename.c_str(), "rb");
	char str[PATH_MAX], *pstr;
	const char seps[]= "\n\r\t ";

	while (fgets(str, PATH_MAX, argfile)) {
		// Find comment and end string there
		if( (pstr = strchr(str, '#')) )
			*pstr= '\0';

		// Tokenize arguments
		pstr= strtok(str, seps);

		while (pstr != NULL) {
			argarray.push_back(strdup(pstr));
			pstr= strtok(NULL, seps);
		}
	}
	fclose(argfile);

	if (argarray.size() == 0)
	{
		iprintf("Invalid argv file\n");
		return false;
	}

	// Use absolute path for run NDS file
	char *oldNdsFile = argarray.at(0);
	argarray.at(0) = getAbsPath(oldNdsFile);
	free(oldNdsFile);

	return true;
}

//---------------------------------------------------------------------------------
static bool usePluginArgv(vector<char*>& argarray, const std::string& filename) {
//---------------------------------------------------------------------------------
	// Determine the plugin path based on the file extension.
	size_t extPos = filename.rfind('.');
	if (extPos == string::npos) {
		iprintf("Unknown plugin\n");
		return false;
	}

	// Create the plugin path based on the file extension
	const char *ext = filename.c_str() + extPos + 1;
	char *pluginPath = (char *)malloc(PLUGIN_DIR_LEN + 1 + strlen(ext) + NDS_EXT_LEN + 1);
	strcpy(pluginPath, PLUGIN_DIR);
	strcat(pluginPath, "/");
	strcat(pluginPath, ext);
	strcat(pluginPath, NDS_EXT);
	argarray.push_back(pluginPath);

	// Store selected file path as argument to plugin
	argarray.push_back(getAbsPath(filename.c_str()));
	return true;
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------

	// overwrite reboot stub identifier
	// so tapping power on DSi returns to DSi menu
	extern u64 *fake_heap_end;
	*fake_heap_end = 0;

	iconTitleInit();

	// Subscreen as a console
	videoSetModeSub(MODE_0_2D);
	vramSetBankH(VRAM_H_SUB_BG);
	consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 15, 0, false, true);

	if (!fatInitDefault()) {
		iprintf ("fatinitDefault failed!\n");
		stop();
	}

	keysSetRepeat(25,5);

	vector<string> extensionList;
	extensionList.push_back(".nds");
	extensionList.push_back(".argv");

	loadPlugins(extensionList, PLUGIN_DIR, NDS_EXT);

	if (extensionList.size() == 2) {
		// Browse the NDS directory when there are no plugins loaded
		chdir("/nds");
	} else {
		// Start at the root directory if there are plugins, since data files
		// could be anywhere
		chdir("/");
	}

	while(1) {

		std::string filename = browseForFile(extensionList);

		// Construct a command line
		vector<char*> argarray;

		if (strcasecmp(filename.c_str() + filename.size() - 5, ".argv") == 0) {
			if (!readArgvFile(argarray, filename)) {
				continue;
			}
		} else if (strcasecmp(filename.c_str() + filename.size() - NDS_EXT_LEN, NDS_EXT) == 0) {
			argarray.push_back(getAbsPath(filename.c_str()));
		} else {
			if (!usePluginArgv(argarray, filename)) {
				continue;
			}
		}

		if (argarray.size() == 0) {
			iprintf("no nds file specified\n");
			continue;
		}

		iprintf ("Running %s with %d parameters\n", argarray[0], argarray.size());
		int err = runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0]);
		iprintf ("Start failed. Error %i\n", err);


		while(argarray.size() !=0 ) {
			free(argarray.at(0));
			argarray.erase(argarray.begin());
		}

		while (1) {
			swiWaitForVBlank();
			scanKeys();
			if (!(keysHeld() & KEY_A)) break;
		}

	}

	return 0;
}
