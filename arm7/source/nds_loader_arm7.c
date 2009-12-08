#include <nds.h>

void runNdsLoaderCheck (void)
{
	if(*((vu32*)0x02FFFE24) == (u32)0x02FFFE04)
	{
		irqDisable (IRQ_ALL);
		*((vu32*)0x02FFFE34) = (u32)0x06000000;
		swiSoftReset();
	} 
}
