
#ifndef DLDI_PATCHER_H
#define DLDI_PATCHER_H

#include <nds/ndstypes.h>

typedef signed int addr_t;
typedef unsigned char data_t;
bool dldiPatchBinary (data_t *binData, u32 binSize);

#endif // DLDI_PATCHER_H
