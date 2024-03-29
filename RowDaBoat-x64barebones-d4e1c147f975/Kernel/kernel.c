#include <idtLoader.h>
#include <interrupts.h>
#include <lib.h>
#include <moduleLoader.h>
#include <scheduler.h>
#include <stdint.h>
#include <string.h>
#include <video_driver.h>
#include <window_manager.h>

extern uint8_t text;
extern uint8_t rodata;
extern uint8_t data;
extern uint8_t bss;
extern uint8_t endOfKernelBinary;
extern uint8_t endOfKernel;

static const uint64_t PageSize = 0x1000;

static void *const sampleCodeModuleAddress = (void *)0x400000;
static void *const sampleDataModuleAddress = (void *)0x500000;

void clearBSS(void *bssAddress, uint64_t bssSize) {
	memset(bssAddress, 0, bssSize);
}

void *getStackBase() {
	return (void *)((uint64_t)&endOfKernel + PageSize * 8 //The size of the stack itself, 32KiB
			- sizeof(PageSize)		      //Begin at the top of the stack
	);
}

void *initializeKernelBinary() {
	void *moduleAddresses[] = {
	    sampleCodeModuleAddress,
	    sampleDataModuleAddress};

	loadModules(&endOfKernelBinary, moduleAddresses);
	clearBSS(&bss, &endOfKernel - &bss);

	return getStackBase();
}

int main() {
	_cli();

	setWindows();
	load_idt();

	const char *name = "shell";
	sys_startProcFg((uint64_t)sampleCodeModuleAddress, 1, &name);
	_sti();
	_hlt();

	return 0;
}
