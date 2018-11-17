/*-----------------------------------------------------------------
 Copyright (C) 2005 - 2017
	Michael "Chishm" Chisholm
	Dave "WinterMute" Murphy
	Claudio "sverx"
	Michael "mtheall" Theall

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

#include <limits.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <unistd.h>
#include <utility>
#include <vector>

#include "args.h"

using namespace std;

static const string NDS_EXT = ".nds";
static const string ARG_EXT = ".argv";
static const char SEPARATORS[] = "\n\r\t ";

/* Checks if s1 ends with s2, ignoring case.
   Returns true if it does, false otherwise.
 */
static bool strCaseEnd(const string& s1, const string& s2) {
	return (s1.size() >= s2.size() &&
			strcasecmp(s1.c_str() + s1.size() - s2.size(), s2.c_str()) == 0);
}

/* Parses the contents of the file given by filename into argarray. Arguments
   are tokenized based on whitespace.
 */
static bool parseArgFileAll(const string& filename, vector<string>& argarray) {
	FILE *argfile = fopen(filename.c_str(), "rb");
	if (!argfile) {
		return false;
	}

	char *line = NULL;
	size_t lineSize = 0;
	while (__getline(&line, &lineSize, argfile) >= 0) {
		// Find comment and end string there
		char *pstr = strchr(line, '#');
		if (pstr) {
			*pstr = '\0';
		}

		// Tokenize arguments
		char *saveptr;
		pstr = strtok_r(line, SEPARATORS, &saveptr);

		while (pstr) {
			argarray.emplace_back(pstr);
			pstr = strtok_r(NULL, SEPARATORS, &saveptr);
		}
	}

	if (line) {
		free(line);
	}

	fclose(argfile);

	return argarray.size() > 0;
}

/* Parses the argument file given by filename and returns the NDS file that it
 * points to.
 */
static bool parseArgFileNds(const std::string& filename, std::string& ndsPath) {
	bool success = false;
	FILE *argfile = fopen(filename.c_str(), "rb");
	if (!argfile) {
		return false;
	}

	char *line = NULL;
	size_t lineSize = 0;
	while (__getline(&line, &lineSize, argfile) >= 0) {
		char *pstr = NULL;

		// Find comment and end string there
		pstr = strchr(line, '#');
		if (pstr) {
			*pstr = '\0';
		}

		// Tokenize arguments
		char *saveptr;
		pstr = strtok_r(line, SEPARATORS, &saveptr);

		if (pstr) {
			// Only want the first token, which should be the NDS file name
			ndsPath = pstr;
			success = true;
			break;
		}
	}

	if (line) {
		free(line);
	}

	fclose(argfile);

	return success;
}

/* Converts a plain filename into an absolute path. If it's already an absolute
 * path, it is returned as-is. If basePath is NULL, the current working directory
 * is used.
 * Returns true on success, false on failure.
 */
static bool toAbsPath(const string& filename, const char* basePath, string& filePath) {
	// Copy existing absolute or empty paths
	if (filename.size() == 0 || filename[0] == '/') {
		filePath = filename;
		return true;
	}

	if (basePath == NULL) {
		// Get current working directory (uses C-strings)
		vector<char> cwd(PATH_MAX);
		if (getcwd (cwd.data(), cwd.size()) == NULL) {
			// Path was too long, abort
			return false;
		}
		// Copy CWD into path
		filePath = cwd.data();
	} else {
		// Just copy the base path
		filePath = basePath;
	}

	// Ensure there's a path separator
	if (filePath.back() != '/') {
		filePath += '/';
	}

	// Now append the filename
	filePath += filename;

	return true;
}

bool argsNdsPath(const std::string& filePath, std::string& ndsPath) {
	if (strCaseEnd(filePath, NDS_EXT)) {
		ndsPath = filePath;
		return true;
	}

	if (strCaseEnd(filePath, ARG_EXT)) {
		return parseArgFileNds(filePath, ndsPath);
	}

	return false;
}

bool argsFillArray(const string& filePath, vector<string>& argarray) {
	// Ensure argarray is empty
	argarray.clear();

	if (strCaseEnd(filePath, NDS_EXT)) {
		string absPath;
		if (!toAbsPath(filePath, NULL, absPath)) {
			return false;
		}
		argarray.push_back(move(absPath));
	}

	if (strCaseEnd(filePath, ARG_EXT)) {
		if (!parseArgFileAll(filePath, argarray)) {
			return false;
		}
	}

	return argarray.size() > 0 && strCaseEnd(argarray[0], NDS_EXT);
}
