#include "gfx.h"

#include <string.h>

void Gfx_FillAll(Color *pixBuff, uint32_t w, uint32_t h, Color col) {
	for (uint32_t i = 0; i < w * h; i++) {
		*(pixBuff + i) = col;
	}
}

void Gfx_FillRect(Color* pixBuff, uint32_t buffW, uint32_t buffH, struct Rect r, Color col) {
	for (uint32_t y = r.y; y < r.y + r.h; y++) {
		for (uint32_t x = r.x; x < r.x + r.w; x++) {
			pixBuff[(y * buffW) + x] = col;
		}
	}
}

void Gfx_VertLine(
	Color* pixBuff, uint32_t buffW, uint32_t buffH,
	int32_t x, int32_t startY, int32_t endY,
	Color col
) {
	if (startY > 0 && endY > 0 && startY < endY) {
		for (int y = startY; y < endY; ++y) {
			pixBuff[(y * buffW) + x] = col;
		}
	}
}

