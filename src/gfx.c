#include "gfx.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>

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
	Color* pixBuff, Color col,
	int32_t x, int32_t startY, int32_t endY,
	uint32_t w, uint32_t h
) {
	if (startY > 0 && endY > 0 && startY < endY) {
		for (int y = startY; y < endY; ++y) {
			pixBuff[(y * w) + x] = col;
		}
	}
}

void Gfx_Line(
	Color* pixBuff, Color col,
	int x0, int y0, int x1, int y1,
	uint32_t w, uint32_t h
) {
	int dx =  abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx + dy, e2; /* error value e_xy */

	for (;;) {
		pixBuff[(y0 * w) + x0] = col;
		if (x0 == x1 && y0 == y1) break;
		e2 = 2 * err;
		if (e2 >= dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
		if (e2 <= dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
	}
}

