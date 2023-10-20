#define FENSTER_HEADER 1
#include "fenster.h"

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include <string.h>

#include "gfx.h"
#include "vector.h"
#include "ui/microui.h"
#include "ui/renderer.h"

#define WIDTH  320
#define HEIGHT 240
#define SCALE  2
#define SCREEN_WIDTH (WIDTH * SCALE)
#define SCREEN_HEIGHT (HEIGHT * SCALE)
uint32_t pixBuff[WIDTH * HEIGHT] = { 0xFF0000FF };
uint32_t screenBuff[SCREEN_WIDTH * SCREEN_HEIGHT] = { 0xFF0000FF };

#include "worldmap.h"

void BlitBuff() {
	for (uint32_t y = 0; y < SCREEN_HEIGHT; y++) {
		for (uint32_t x = 0; x < SCREEN_WIDTH; x++) {
			screenBuff[(y * SCREEN_WIDTH) + x] = pixBuff[((y / SCALE) * WIDTH) + (x / SCALE)];
		}
	}
}

static int text_width(mu_Font font, const char *text, int len) {
	if (len == -1) { len = strlen(text); }
	return r_get_text_width(text, len);
}

static int text_height(mu_Font font) {
	return r_get_text_height();
}

int main(void) {
	struct fenster win = {
		.title = "raycaster",
		.width = SCREEN_WIDTH,
		.height = SCREEN_HEIGHT,
		.buf = screenBuff
	};
	fenster_open(&win);
	double maxFrameTime = 1000.0f / 30.0f;

	mu_Context *ctx = malloc(sizeof(mu_Context));
	mu_init(ctx);
	ctx->text_width = text_width;
	ctx->text_height = text_height;
	int mouseOldX = 0, mouseOldY = 0, mouseDownOld = 0;

	// const struct Rect topHalf = { 0, 0, WIDTH, HEIGHT / 2 };
	// const struct Rect bottomHalf = { 0, HEIGHT / 2, WIDTH, HEIGHT / 2 };

	struct FVec2 player = { 20, 20 };
	float playerAngle = 0, // Angle at which player is facing (in rads)
	      playerDx = cos(playerAngle),
	      playerDy = sin(playerAngle);

	int64_t timeFrameStart = fenster_time();
	double frameTime = 0;
	while (fenster_loop(&win) == 0) {
		for (int i = 0; i < 128; i++) {
			if (win.keys[i]) {
				switch (i) {
					case 'W': {
						struct FVec2 newPos = { player.x + playerDx, player.y + playerDy };
						if (map[(int)(player.y/mapUnit)][(int)(newPos.x/mapUnit)] != '#') player.x += playerDx;
						if (map[(int)(newPos.y/mapUnit)][(int)(player.x/mapUnit)] != '#') player.y += playerDy;
						break;
					}
					case 'S': {
						struct FVec2 newPos = { player.x - playerDx, player.y - playerDy };
						if (map[(int)(player.y/mapUnit)][(int)(newPos.x/mapUnit)] != '#') player.x -= playerDx;
						if (map[(int)(newPos.y/mapUnit)][(int)(player.x/mapUnit)] != '#') player.y -= playerDy;
						break;
					}
					case 'A': {
						playerAngle -= 0.1;
						if (playerAngle < 0) playerAngle = 2 * M_PI;
						playerDx = cos(playerAngle);
						playerDy = sin(playerAngle);
						break;
					}
					case 'D': {
						playerAngle += 0.1;
						if (playerAngle > 2 * M_PI) playerAngle -= 2 * M_PI;
						playerDx = cos(playerAngle);
						playerDy = sin(playerAngle);
						break;
					}
					default: break;
				}
			}
		}

		Gfx_FillAll(pixBuff, WIDTH, HEIGHT, COLOR_RGB(00, 00, 00));

		for (int y = 0; y < mapHeight; ++y) {
			for (int x = 0; x < mapWidth; ++x) {
				char obj = map[y][x];
				Color col = COLOR_RGB(0, 0, 0);
				switch (obj) {
					case '#': col = COLOR_RGB(0, 0, 255); break;
					case ' ': break;
					default: assert(0);
				}
				Gfx_FillRect(pixBuff, WIDTH, HEIGHT, (struct Rect) { x * mapUnit, y * mapUnit, mapUnit, mapUnit }, col);
			}
		}

		Gfx_Line(pixBuff, COLOR_RGB(255, 0, 0), player.x, player.y, player.x + playerDx * 10, player.y + playerDy * 10, WIDTH, HEIGHT);
		pixBuff[((long int)player.y * WIDTH) + (long int)player.x] = COLOR_RGB(255, 255, 0);

		BlitBuff();

		// MUI - INPUT
		if (mouseOldX != win.x || mouseOldY != win.y) { // if any mouse position changed
			mu_input_mousemove(ctx, win.x, win.y);
		}
		if (mouseDownOld != win.mouse) { // if mouse pressed state changed
			if (win.mouse == 1) {
				mu_input_mousedown(ctx, win.x, win.y, MU_MOUSE_LEFT);
			} else {
				mu_input_mouseup(ctx, win.x, win.y, MU_MOUSE_LEFT);
			}
		}

		// MUI - GUI
		mu_begin(ctx);
		if (mu_begin_window(ctx, "Demo Window", mu_rect(40, 40, 100, 100))) {
			mu_text(ctx, "Hello World!\nLMAO\nSAX SUX");
			mu_button(ctx, "Press me");
			mu_end_window(ctx);
		}
		mu_end(ctx);

		// MUI - DRAW & Update Old Values
		r_drawAll(ctx, screenBuff, SCREEN_WIDTH, SCREEN_HEIGHT);
		mouseOldX = win.x;
		mouseOldX = win.y;
		mouseDownOld = win.mouse;

		int64_t timeFrameEnd = fenster_time();
		frameTime = (double)timeFrameEnd - timeFrameStart;
		if (frameTime < maxFrameTime) {
			fenster_sleep(maxFrameTime - frameTime);
		}
		timeFrameStart = fenster_time();
	}
	fenster_close(&win);
	return 0;
}

