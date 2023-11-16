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

#ifndef NDS_LOADER_ARM9_H
#define NDS_LOADER_ARM9_H


#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	RUN_NDS_OK = 0,
	RUN_NDS_STAT_FAILED,
	RUN_NDS_GETCWD_FAILED,
	RUN_NDS_PATCH_DLDI_FAILED,
} eRunNdsRetCode;

#define LOAD_DEFAULT_NDS 0

eRunNdsRetCode runNdsFile (const char* filename, int argc, const char** argv);

bool installBootStub(bool havedsiSD);
void installExcptStub(void);

#ifdef __cplusplus
}
#endif

#endif // NDS_LOADER_ARM7_H
