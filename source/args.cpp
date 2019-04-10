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

#include <dirent.h>
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
static const string EXT_EXT = ".ext";
static const char EXT_DIR[] = "/nds";
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

/* Convert a dataFilePath to the path of the ext file that specifies the
 * handler.
 * Returns true on success, false on failure
 */
static bool toExtPath(const string& dataFilePath, string& extFilePath) {
	// Figure out what the file extension is
	size_t extPos = dataFilePath.rfind('.');
	if (extPos == string::npos) {
		return false;
	}

	extPos += 1;
	if (extPos >= dataFilePath.size()) {
		return false;
	}

	// Construct handler path from extension. Handlers are in the EXT_DIR and
	// end with EXT_EXT.
	const string ext = dataFilePath.substr(extPos);
	if (!toAbsPath(ext, EXT_DIR, extFilePath)) {
		return false;
	}

	extFilePath += EXT_EXT;

	return true;
}

bool argsNdsPath(const std::string& filePath, std::string& ndsPath) {
	if (strCaseEnd(filePath, NDS_EXT)) {
		ndsPath = filePath;
		return true;
	} else	if (strCaseEnd(filePath, ARG_EXT)) {
		return parseArgFileNds(filePath, ndsPath);
	} else {
		// This is a data file associated with a handler NDS by an ext file
		string extPath;
		if (!toExtPath(filePath, extPath)) {
			return false;
		}
		string ndsRelPath;
		if (!parseArgFileNds(extPath, ndsRelPath)) {
			return false;
		}
		// Handler is in EXT_DIR
		return toAbsPath(ndsRelPath, EXT_DIR, ndsPath);
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
	} else if (strCaseEnd(filePath, ARG_EXT)) {
		if (!parseArgFileAll(filePath, argarray)) {
			return false;
		}
	} else {
		// This is a data file associated with a handler NDS by an ext file
		string extPath;

		if (!toExtPath(filePath, extPath)) {
			return false;
		}

		// Read the arg file for the extension handler
		if (!parseArgFileAll(extPath, argarray)) {
			return false;
		}

		// Extension handler relative path is relative to EXT_DIR, not CWD
		string absPath;
		if (!toAbsPath(argarray[0], EXT_DIR, absPath)) {
			return false;
		}
		argarray[0] = absPath;

		// Add the data filename to the end. Its path is relative to CWD.
		if (!toAbsPath(filePath, NULL, absPath)) {
			return false;
		}
		argarray.push_back(move(absPath));
	}

	return argarray.size() > 0 && strCaseEnd(argarray[0], NDS_EXT);
}

vector<string> argsGetExtensionList() {
	vector<string> extensionList;

	// Always supported files: NDS binaries and predefined argument lists
	extensionList.push_back(NDS_EXT);
	extensionList.push_back(ARG_EXT);

	// Get a list of extension files: argument lists associated with a file type
	DIR *dir = opendir (EXT_DIR);
	if (dir) {
		for (struct dirent* dirent = readdir(dir); dirent != NULL; dirent = readdir(dir)) {
			// Add the name component of all files ending with EXT_EXT to the list
			if (dirent->d_type != DT_REG) {
				continue;
			}

			if (dirent->d_name[0] != '.' && strCaseEnd(dirent->d_name, EXT_EXT)) {
				size_t extPos = strlen(dirent->d_name) - EXT_EXT.size();
				dirent->d_name[extPos] = '\0';
				extensionList.push_back(dirent->d_name);
			}
		}
		closedir(dir);
	}

	return extensionList;
}
