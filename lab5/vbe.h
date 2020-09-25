#include <lcom/lcf.h>

#include "utils.h"
#include <machine/int86.h>
#include "vbe_macros.h"

void * (vg_init)(uint16_t mode);

void vg_fillPixel(uint16_t x, uint16_t y, uint32_t color);

void (vg_make_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);

int vg_draw_pattern(uint8_t no_rectangles, uint32_t first, uint8_t step);

xpm_image_t vg_draw_pixmap(xpm_map_t xpm, uint16_t x, uint16_t y);

int get_mode_info(uint16_t mode,vbe_mode_info_t * info);

int vbe_get_contr_info();

void* farToPtr(uint32_t far,mmap_t map);

