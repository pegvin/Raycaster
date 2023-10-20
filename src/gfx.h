#ifndef RAYCASTER_GFX_H_INCLUDED_
#define RAYCASTER_GFX_H_INCLUDED_ 1
#pragma once

#include <stdint.h>

struct Rect {
	uint32_t x, y, w, h;
};

typedef uint32_t Color; // 0xAARRGGBB

#define COLOR_RGBA(r, g, b, a) (((uint32_t)(a) << 24) + ((uint32_t)(r) << 16) + ((uint32_t)(g) << 8) + (uint32_t)(b))
#define COLOR_RGB(r, g, b) COLOR_RGBA(r, g, b, 0xFF)

// Fill Whole Buffer With `col` Color
void Gfx_FillAll(Color* pixBuff, uint32_t w, uint32_t h, Color col);
void Gfx_FillRect(Color* pixBuff, uint32_t buffW, uint32_t buffH, struct Rect r, Color col);

void Gfx_VertLine(
	Color* pixBuff, Color col,
	int32_t x, int32_t startY, int32_t endY,
	uint32_t w, uint32_t h
);

void Gfx_Line(
	Color* pixBuff, Color col,
	int x0, int y0, int x1, int y1,
	uint32_t w, uint32_t h
);

#endif // RAYCASTER_GFX_H_INCLUDED_

