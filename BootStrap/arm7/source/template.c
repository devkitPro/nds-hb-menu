#include <calico.h>

__attribute__((section(".prepad"))) const char dummy[16] = "hbmenu bootstrap";

int main(int argc, char* argv[])
{
	// Read settings from NVRAM
	envReadNvramSettings();

	// Set up extended keypad server (X/Y/hinge)
	keypadStartExtServer();

	// Configure and enable VBlank interrupt
	lcdSetIrqMask(DISPSTAT_IE_ALL, DISPSTAT_IE_VBLANK);
	irqEnable(IRQ_VBLANK);

	// Set up RTC
	rtcInit();
	rtcSyncTime();

	// Initialize power management
	pmInit();

	// Set up block device peripherals
	blkInit();

	// Main loop (mostly idle)
	while (pmMainLoop()) {
		threadWaitForVBlank();
	}

	return 0;
}
