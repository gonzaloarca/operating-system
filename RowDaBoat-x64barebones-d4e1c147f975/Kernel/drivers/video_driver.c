#include <video_driver.h>

struct vbe_mode_info_structure {
	uint16_t attributes;		// deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
	uint8_t window_a;			// deprecated
	uint8_t window_b;			// deprecated
	uint16_t granularity;		// deprecated; used while calculating bank numbers
	uint16_t window_size;
	uint16_t segment_a;
	uint16_t segment_b;
	uint32_t win_func_ptr;		// deprecated; used to switch banks from protected mode without returning to real mode
	uint16_t pitch;			// number of bytes per horizontal line
	uint16_t width;			// width in pixels
	uint16_t height;			// height in pixels
	uint8_t w_char;			// unused...
	uint8_t y_char;			// ...
	uint8_t planes;
	uint8_t bpp;			// bits per pixel in this mode
	uint8_t banks;			// deprecated; total number of banks in this mode
	uint8_t memory_model;
	uint8_t bank_size;		// deprecated; size of a bank, almost always 64 KB but may be 16 KB...
	uint8_t image_pages;
	uint8_t reserved0;

	uint8_t red_mask;
	uint8_t red_position;
	uint8_t green_mask;
	uint8_t green_position;
	uint8_t blue_mask;
	uint8_t blue_position;
	uint8_t reserved_mask;
	uint8_t reserved_position;
	uint8_t direct_color_attributes;

	uint32_t framebuffer;		// physical address of the linear frame buffer; write here to draw to the screen
	uint32_t off_screen_mem_off;
	uint16_t off_screen_mem_size;	// size of memory in the framebuffer but not being displayed on the screen
	uint8_t reserved1[206];
} __attribute__ ((packed));

// Busco VBEModeInfoBlock que es donde se encuentra la estructura
struct vbe_mode_info_structure * screenInfo = (struct vbe_mode_info_structure *) 0x5C00;

// Funcion de interrupts.asm
extern void haltcpu();

int sys_writePixel(int x, int y, int rgb) {
	char (*screen)[screenInfo->width][3] = (char (*)[(screenInfo->width)][3]) ((uint64_t)screenInfo->framebuffer);
	screen[y][x][0] = rgb & 0xFF;
	screen[y][x][1] = (rgb >> 8) & 0xFF;
	screen[y][x][2] = (rgb >> 16) & 0xFF;
	return 0;
}

//  Funcion que se encarga de dibujar un caracter en pantalla(con la font default) dado su esquina izq superior y un color
int drawChar( char c, int x, int y, int rgb, int backgroundColour ){	
	const unsigned char * letra = getCharMap(c);
	if( letra == 0 ){
		letra = getCharMap(32);		//debe printear vacio
	}
	for( int i = 0; i < FONT_HEIGHT; i++ ){
		unsigned char row = letra[i];
		for( int j = FONT_WIDTH - 1; j >= 0; j-- ){
			unsigned char aux = row & 1;
			if( aux == 1 ){
				sys_writePixel( x + j, y + i, rgb);
			}
			else {
				sys_writePixel( x + j, y + i, backgroundColour );
			}
			row = row >> 1;
		}
	}
	return 0;
}