#ifndef MICROUI_SOFTWARE_RENDERER_H_INCLUDED_
#define MICROUI_SOFTWARE_RENDERER_H_INCLUDED_ 1
#pragma once

#include "gfx.h"
#include "ui/microui.h"

int r_get_text_width(const char *text, int len);
int r_get_text_height(void);

void r_drawAll(
	mu_Context* ctx, Color* buff,
	uint32_t w, uint32_t h
);

#endif // MICROUI_SOFTWARE_RENDERER_H_INCLUDED_

