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

#include "file_browse.h"
#include <vector>
#include <algorithm>
#include <sys/dir.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <nds.h>

#define PATH_LEN 1024

#define SCREEN_COLS 32
#define ENTRIES_PER_SCREEN 22
#define ENTRIES_START_ROW 2
#define ENTRY_PAGE_LENGTH 10

using namespace std;

struct DirEntry
{
	string name;
	bool isDirectory;
} ;

bool nameEndsWith (const string& name, const string& extension)
{
	if (name.size() == 0 || name.size() < extension.size() || extension.size() == 0) {
		return false;
	}
	
	return strcasecmp (name.c_str() + name.size() - extension.size(), extension.c_str()) == 0;
}

bool dirEntryPredicate (const DirEntry& lhs, const DirEntry& rhs)
{
	if (!lhs.isDirectory && rhs.isDirectory) {
		return false;
	}
	if (lhs.isDirectory && !rhs.isDirectory) {
		return true;
	}
	return strcasecmp(lhs.name.c_str(), rhs.name.c_str()) < 0;
}

void getDirectoryContents (vector<DirEntry>& dirContents, const string& extension) 
{
	struct stat st;
	char filename[PATH_LEN];
	DIR_ITER* dir;

	dirContents.clear();

	dir = diropen ("."); 
	
	if (dir == NULL) {
		iprintf ("Unable to open the directory.\n");
	} else {
		while (dirnext(dir, filename, &st) == 0) {
			DirEntry dirEntry;
			
			dirEntry.name = filename;
			dirEntry.isDirectory = (st.st_mode & S_IFDIR) ? true : false;
			
			if (dirEntry.name.compare(".") != 0 && (dirEntry.isDirectory || nameEndsWith(dirEntry.name, extension)))
			{
				dirContents.push_back (dirEntry);
			}
		}
	}	
	
	dirclose (dir);	
	
	sort(dirContents.begin(), dirContents.end(), dirEntryPredicate);
}

void getDirectoryContents (vector<DirEntry>& dirContents) 
{
	getDirectoryContents (dirContents, "");
}

void showDirectoryContents (const vector<DirEntry>& dirContents, int startRow)
{
	char path[PATH_LEN];
	
	
	getcwd(path, PATH_LEN);
	
	// Clear the screen
	iprintf ("\x1b[2J");
	
	// Print the path
	if (strlen(path) < SCREEN_COLS) {
		iprintf ("%s", path);
	} else {
		iprintf ("%s", path + strlen(path) - SCREEN_COLS);
	}
	
	// Move to 2nd row
	iprintf ("\x1b[1;0H");
	// Print line of dashes
	iprintf ("--------------------------------");
	
	// Print directory listing
	for (int i = 0; i < ((int)dirContents.size() - startRow) && i < ENTRIES_PER_SCREEN; i++) {
		const DirEntry* entry = &dirContents.at(i + startRow);
		char entryName[SCREEN_COLS + 1];
		
		// Set row
		iprintf ("\x1b[%d;0H", i + ENTRIES_START_ROW);
		
		if (entry->isDirectory) {
			strncpy (entryName, entry->name.c_str(), SCREEN_COLS);
			entryName[SCREEN_COLS - 3] = '\0';
			iprintf (" [%s]", entryName);
		} else {
			strncpy (entryName, entry->name.c_str(), SCREEN_COLS);
			entryName[SCREEN_COLS - 1] = '\0';
			iprintf (" %s", entryName);
		}
	}
}

string browseForFile (const string& extension)
{
	int pressed = 0;
	int screenOffset = 0;
	int fileOffset = 0;
	vector<DirEntry> dirContents;
	
	getDirectoryContents (dirContents, extension);
	showDirectoryContents (dirContents, screenOffset);
	
	while (true) {
		// Clear old cursors
		for (int i = ENTRIES_START_ROW; i < ENTRIES_PER_SCREEN + ENTRIES_START_ROW; i++) {
			iprintf ("\x1b[%d;0H ", i);
		}
		// Show cursor
		iprintf ("\x1b[%d;0H*", fileOffset - screenOffset + ENTRIES_START_ROW);
		
		// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing else to do
		do {
			scanKeys();
			pressed = keysDownRepeat();
			swiWaitForVBlank();
		} while (!pressed);
	
		if (pressed & KEY_UP) 		fileOffset -= 1;
		if (pressed & KEY_DOWN) 	fileOffset += 1;
		if (pressed & KEY_LEFT) 	fileOffset -= ENTRY_PAGE_LENGTH;
		if (pressed & KEY_RIGHT)	fileOffset += ENTRY_PAGE_LENGTH;
		
		if (fileOffset < 0) 	fileOffset = dirContents.size() - 1;		// Wrap around to bottom of list
		if (fileOffset > ((int)dirContents.size() - 1))		fileOffset = 0;		// Wrap around to top of list

		// Scroll screen if needed
		if (fileOffset < screenOffset) 	{
			screenOffset = fileOffset;
			showDirectoryContents (dirContents, screenOffset);
		}
		if (fileOffset > screenOffset + ENTRIES_PER_SCREEN - 1) {
			screenOffset = fileOffset - ENTRIES_PER_SCREEN + 1;
			showDirectoryContents (dirContents, screenOffset);
		}
		
		if (pressed & KEY_A) {
			DirEntry* entry = &dirContents.at(fileOffset);
			if (entry->isDirectory) {
				// Enter selected directory
				chdir (entry->name.c_str());
				getDirectoryContents (dirContents, extension);
				screenOffset = 0;
				fileOffset = 0;
				showDirectoryContents (dirContents, screenOffset);
			} else {
				// Clear the screen
				iprintf ("\x1b[2J");
				// Return the chosen file
				return entry->name;
			}
		}
		
		if (pressed & KEY_B) {
			// Go up a directory
			chdir ("..");
			getDirectoryContents (dirContents, extension);
			screenOffset = 0;
			fileOffset = 0;
			showDirectoryContents (dirContents, screenOffset);
		}
	}
}
