/* _loader.c */
#include <std_io.h>
#include <stdint.h>

extern char bss;
extern char endOfBinary;

int main();

int _start() {
	//Clean BSS
	memset(&bss, 0, &endOfBinary - &bss);

	return main();
}
