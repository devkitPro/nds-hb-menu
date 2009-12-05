#include <nds.h>

void runNdsLoaderCheck (void)
{
	if(*((vu32*)0x027FFE24) == (u32)0x027FFE04)
	{
		irqDisable (IRQ_ALL);
		*((vu32*)0x027FFE34) = (u32)0x06000000;
		swiSoftReset();
	} 
}
