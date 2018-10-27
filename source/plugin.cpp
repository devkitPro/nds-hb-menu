/*-----------------------------------------------------------------
 Copyright (C) 2018
	Michael "Chishm" Chisholm

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

#include "plugin.h"
#include <dirent.h>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>

using namespace std;

void loadPlugins (vector<string>& extensions, const char *pluginDir, const char *pluginExt) {
	DIR *pdir = opendir (pluginDir);
	size_t pluginExtLen = strlen(pluginExt);
	struct dirent* pent;

	while( (pent = readdir(pdir)) != NULL ) {
		if (pent->d_type != DT_REG) {
			// Only examine files
			continue;
		}

		size_t name_len = strlen(pent->d_name);
		if (name_len <= pluginExtLen) {
			// File name is not long enough
			continue;
		}

		if (strcasecmp(pent->d_name + name_len - pluginExtLen, pluginExt) != 0) {
			// Doesn't end in the NDS extension
			continue;
		}

		// The plugin name up until the ".nds" is what file extensions it handles
		pent->d_name[name_len - pluginExtLen] = '\0';

		extensions.push_back(pent->d_name);
	}

	closedir(pdir);
}
