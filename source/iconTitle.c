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

#include "hbmenu_banner.h"
#include "font6x8.h"

#define TITLE_POS_X	(2*8)
#define TITLE_POS_Y	(17*8)

#define ICON_POS_X	32
#define ICON_POS_Y	80

#define TEXT_WIDTH	((32-4)*8/6)

#define buf_SIZE	0x440
#define ICON_OFFS	0x20
#define PAL_OFFS	0x220
#define TITLE_OFFS	0x340

OAMTable Sprites;

void writecharRS (int row, int col, u16 car) {
	u16 oldval=BG_GFX[row*(512/8/2)+(col/2)];
	oldval&=(col%2)?0xFF:0xFF00;
	oldval|=(col%2)?(car<<8):car;
	BG_GFX[row*(512/8/2)+col/2]=oldval;
}

void writeRow (int rownum, const char* text) {
	int i,len,p=0;
	len=strlen(text);

	if (len>TEXT_WIDTH) len=TEXT_WIDTH;
	for (i=0;i<(TEXT_WIDTH-len)/2;i++)
		writecharRS (rownum, i, 0);
	for (i=(TEXT_WIDTH-len)/2;i<((TEXT_WIDTH-len)/2+len);i++)
		writecharRS (rownum, i, text[p++]-' ');
	for (i=((TEXT_WIDTH-len)/2+len);i<TEXT_WIDTH;i++)
		writecharRS (rownum, i, 0);
}

void clearIcon (void) {
	int zero=0;
	// zero out the first bytes of SPRITE_GFX
	swiFastCopy(&zero, SPRITE_GFX, (32*32/2/4)|COPY_MODE_WORD|COPY_MODE_FILL);
}

void iconTitleInit (void) {

	int i,zero=0;

	// set bank A and D for background memory (128K+128K)
	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankD(VRAM_D_MAIN_BG_0x06020000);

	// define BG3 to use second half of BG_GFX instead of 1st half (offset 8x16k)
	bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 8,0);

	// set the BG3 scaling 1:1
	REG_BG3PA=1<<8;
	REG_BG3PB=0;
	REG_BG3PC=0;
	REG_BG3PD=1<<8;

	// set the BG3 scroll at 0,0
	REG_BG3X=0;
	REG_BG3Y=0;

	// load compressed bitmap
	decompress(hbmenu_bannerBitmap, &BG_GFX[(128*1024/2)], LZ77Vram);

	// zero out the first 128KB of BG_GFX
	swiFastCopy(&zero, BG_GFX, (128*1024/4)|COPY_MODE_WORD|COPY_MODE_FILL);

	// set up BG2 for title/info (MAP:0 tiles:1)
	bgInit(2, BgType_Rotation, BgSize_R_512x512, 0, 1);

	// set the BG scaling (8 to 6 horizontal scaling)
	REG_BG2PA=(8<<8)/6;
	REG_BG2PB=0;
	REG_BG2PC=0;
	REG_BG2PD=1<<8;

	// set the background upper/left corner to the place where title/info will be written
	REG_BG2X=-TITLE_POS_X<<8;
	REG_BG2Y=-TITLE_POS_Y<<8;

	// load font (compressed to save some bytes...)
	decompress(font6x8Tiles, &BG_GFX[(16*1024/2)], LZ77Vram);

	// load font palette
	swiFastCopy(font6x8Pal, BG_PALETTE, (16*2/4)|COPY_MODE_WORD);

	// set bank B for MAIN sprite
	vramSetBankB(VRAM_B_MAIN_SPRITE_0x06400000);

	// disable all the sprites
	for (i=1;i<128;i++) {
		Sprites.oamBuffer[i].attribute[0]=ATTR0_DISABLED|OBJ_Y(192);  // out of screen
		Sprites.oamBuffer[i].attribute[1]=ATTR1_SIZE_8|OBJ_X(256);    // out of screen
		Sprites.oamBuffer[i].attribute[2]=0;
	}

	// configure sprite #0
	Sprites.oamBuffer[0].attribute[0]=ATTR0_NORMAL|OBJ_Y(ICON_POS_Y);
	Sprites.oamBuffer[0].attribute[1]=ATTR1_SIZE_32|OBJ_X(ICON_POS_X);
	Sprites.oamBuffer[0].attribute[2]=0;

	// update OAM
	dmaCopy (&Sprites, OAM, sizeof(OAMTable));

	// set video mode and activate BG2 (affine), BG3 (bitmap) and sprites
	videoSetMode (MODE_4_2D|DISPLAY_BG2_ACTIVE|DISPLAY_BG3_ACTIVE
												 |DISPLAY_SPR_ACTIVE|DISPLAY_SPR_1D);

	// everything's ready :)

	writeRow (0,"...initializing...");
	writeRow (1,"===>>> HBMenu+ <<<===");
	writeRow (2,"(this text should disappear...");
	writeRow (3,"...otherwise, trouble!)");

}


void iconTitleUpdate (int isdir, const char* name) {

	writeRow (0,name);
	writeRow (1,"");
	writeRow (2,"");
	writeRow (3,"");

	if (isdir) {
		// text
		writeRow (2,"[directory]");
		// icon
		clearIcon();
	} else {

		FILE *fp;
		unsigned int Icon_title_offset;
		int ret;
		char buf[buf_SIZE];  // the read buffer

		// open file for reading info
		fp=fopen (name,"rb");
		if (fp==NULL) {
			// text
			writeRow (2,"(can't open file!)");
			// icon
			clearIcon();
			fclose (fp); return;
		}

		ret=fseek (fp,0x68,SEEK_SET);
		if (ret==0)
			ret=fread (&Icon_title_offset, sizeof(int), 1, fp); // read if seek succeed
		else
			ret=0;  // if seek fails set to !=1

		if (ret!=1) {
			// text
			writeRow (2,"(can't read file!)");
			// icon
			clearIcon();
			fclose (fp); return;
		}

		if (Icon_title_offset==0) {
			// text
			writeRow (2,"(no title/icon)");
			// icon
			clearIcon();
			fclose (fp); return;
		}

		ret=fseek (fp,Icon_title_offset,SEEK_SET);
		if (ret==0)
			ret=fread (&buf, buf_SIZE, 1, fp); // read if seek succeed
		else
			ret=0;  // if seek fails set to !=1

		if (ret!=1) {
			// text
			writeRow (2,"(can't read icon/title!)");
			// icon
			clearIcon();
			fclose (fp); return;
		}

		// close file!
		fclose (fp);

		// turn unicode into ascii (kind of)
		// and convert 0x0A into 0x00
		int i,len,len2;
		for (i=0;i<0x100;i=i+2) {
			if ((buf[TITLE_OFFS+i]==0x0A) || (buf[TITLE_OFFS+i]==0xFF))
				buf[TITLE_OFFS+(i/2)]=0;
			else
				buf[TITLE_OFFS+(i/2)]=buf[TITLE_OFFS+i];
		}

		// text
		writeRow (1,&buf[TITLE_OFFS]);
		len=strlen(&buf[TITLE_OFFS]);
		writeRow (2,&buf[TITLE_OFFS+len+1]);
		len2=strlen(&buf[TITLE_OFFS+len+1]);
		writeRow (3,&buf[TITLE_OFFS+len+len2+1+1]);
		// icon
		swiFastCopy(&buf[ICON_OFFS], SPRITE_GFX, (32*32/2/4)|COPY_MODE_WORD);
		swiFastCopy(&buf[PAL_OFFS], SPRITE_PALETTE, (16*2/4)|COPY_MODE_WORD);
	}
}

