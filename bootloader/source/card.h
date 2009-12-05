
#ifndef CARD_H
#define CARD_H

#include "disc_io.h"
#include "io_dldi.h"

static inline bool CARD_StartUp (void)
{
	return _io_dldi.fn_startup();
}

static inline bool CARD_IsInserted (void)
{
	return _io_dldi.fn_isInserted();
}

static inline bool CARD_ReadSector (u32 sector, void *buffer)
{
	return _io_dldi.fn_readSectors(sector, 1, buffer);
}

#endif // CARD_H
