#ifndef RAYCASTER_GFX_H_INCLUDED_
#define RAYCASTER_GFX_H_INCLUDED_ 1
#pragma once

#include <stdint.h>

struct Rect {
	uint32_t x, y, w, h;
};

typedef uint32_t Color; // 0xAARRGGBB

// r, g, b, a values should be in hexadecimal format without `0x`
#define COLOR_RGBA(r, g, b, a) 0x##a##r##g##b
#define COLOR_RGB(r, g, b) COLOR_RGBA(r, g, b, FF)

// Fill Whole Buffer With `col` Color
void Gfx_FillAll(Color* pixBuff, uint32_t w, uint32_t h, Color col);
void Gfx_FillRect(Color* pixBuff, uint32_t buffW, uint32_t buffH, struct Rect r, Color col);
void Gfx_VertLine(
	Color* pixBuff, uint32_t buffW, uint32_t buffH,
	int32_t x, int32_t startY, int32_t endY,
	Color col
);

#endif // RAYCASTER_GFX_H_INCLUDED_

