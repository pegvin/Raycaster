#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <assert.h>
#include "ui/microui.h"
#include "ui/renderer.h"
#include "ui/icon_atlas.inl"

// A Big Thanks to @wtjcn for his fixedsys font at https://github.com/wtjcn/microui
// it just makes everything much more clear

static mu_Rect clipRect = { 0, 0, INT_MAX, INT_MAX };
#define IS_IN_CLIP_RECT(_x, _y) \
	(_x > clipRect.x && _y > clipRect.y && _x < clipRect.x + clipRect.w && _y < clipRect.y + clipRect.h)

int r_get_text_width(const char *text, int len) {
	int res = 0;
	for (const char *p = text; *p && len--; p++) {
		if ((*p & 0xc0) == 0x80) { continue; }
		int chr = mu_min((unsigned char) *p, 127);
		res += atlas[ATLAS_FONT + chr].w;
	}
	return res;
}

int r_get_text_height(void) {
	return 16;
}

void r_set_clip_rect(mu_Rect rect) {
	clipRect = rect;
}

void r_draw_rect(Color* buff, uint32_t w, uint32_t h, mu_Rect rect, mu_Color color) {
	Color col = COLOR_RGBA(color.r, color.g, color.b, color.a);
	Gfx_FillRect(
		buff, w, h,
		(struct Rect){
			mu_clamp(rect.x, clipRect.x, INT_MAX), mu_clamp(rect.y, clipRect.y, INT_MAX),
			mu_clamp(rect.w, 0, clipRect.w), mu_clamp(rect.h, 0, clipRect.h)
		},
		col
	);
}

void r_draw_text(
	Color* buff, uint32_t w, uint32_t h,
	const char *text, mu_Vec2 pos, mu_Color color
) {
	mu_Rect dst = { pos.x, pos.y, 0, 0 };
	for (const char *p = text; *p; p++) {
		if ((*p & 0xc0) == 0x80) { continue; }
		int chr = mu_min((unsigned char) *p, 127);
		mu_Rect src = atlas[ATLAS_FONT + chr];
		dst.w = src.w;
		dst.h = src.h;

		for (int y = 0; y < src.h; y++) {
			for (int x = 0; x < src.w; x++) {
				mu_Vec2 in = { src.x + x, src.y + y };
				mu_Vec2 out = { dst.x + x, dst.y + y };
				if (IS_IN_CLIP_RECT(out.x, out.y)) {
					if (atlas_texture[in.y][in.x] == '*') {
						buff[(out.y * w) + out.x] = COLOR_RGBA(color.r, color.g, color.b, color.a);
					}
				}
			}
		}

		dst.x += dst.w;
	}
}

void r_draw_icon(
	Color* buff, uint32_t w, uint32_t h,
	int id, mu_Rect oldDst, mu_Color color
) {
	mu_Rect src = atlas[id];
	mu_Rect dst = mu_rect(
		oldDst.x + (oldDst.w - src.w) / 2,
		oldDst.y + (oldDst.h - src.h) / 2,
		src.w, src.h
	);

	for (int y = 0; y < src.h; y++) {
		for (int x = 0; x < src.w; x++) {
			mu_Vec2 in = { src.x + x, src.y + y };
			mu_Vec2 out = { dst.x + x, dst.y + y };
			if (IS_IN_CLIP_RECT(out.x, out.y)) {
				if (atlas_texture[in.y][in.x] == '*') {
					buff[(out.y * w) + out.x] = COLOR_RGBA(color.r, color.g, color.b, color.a);
				}
			}
		}
	}
}

void r_drawAll(mu_Context* ctx, Color* buff, uint32_t w, uint32_t h) {
	mu_Command* cmd = NULL;
	while (mu_next_command(ctx, &cmd)) {
		switch (cmd->type) {
			case MU_COMMAND_TEXT: {
				r_draw_text(buff, w, h, cmd->text.str, cmd->text.pos, cmd->text.color);
				break;
			}
			case MU_COMMAND_RECT: {
				r_draw_rect(buff, w, h, cmd->rect.rect, cmd->rect.color);
				break;
			}
			case MU_COMMAND_ICON: {
				r_draw_icon(buff, w, h, cmd->icon.id, cmd->icon.rect, cmd->icon.color);
				break;
			}
			case MU_COMMAND_CLIP: {
				r_set_clip_rect(cmd->clip.rect);
				break;
			}
		}
	}
}

