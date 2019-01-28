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

#ifndef ARGS_H
#define ARGS_H

#include <string>
#include <vector>

/* Convert a file path of any type (e.g. .nds or .argv) into a path to the NDS
 * file to be opened. The returned path may be absolute or relative to the
 * current working directory.
 * Returns true on success, false on failure.
 */
bool argsNdsPath(const std::string& filePath, std::string& ndsPath);

/* Convert a file path of any type into an argument array by filling the array
 * that is passed in. The first argument will be the full path to an NDS file.
 * Returns true on success, false on failure.
 */
bool argsFillArray(const std::string& filePath, std::vector<std::string>& argarray);

/* Return a list of all file extensions that can be browsed and opened.
 */
std::vector<std::string> argsGetExtensionList();

#endif // ARGS_H
