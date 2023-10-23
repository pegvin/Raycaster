#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#define FENSTER_HEADER 1
#include "fenster.h"

#include "gfx.h"
#include "vector.h"
#include "worldmap.h"

// GUI
#include "ui/microui.h"
#include "ui/renderer.h"

#define WIDTH  320
#define HEIGHT 240
#define SCALE  2
#define SCREEN_WIDTH (WIDTH * SCALE)
#define SCREEN_HEIGHT (HEIGHT * SCALE)

uint32_t pixBuff[WIDTH * HEIGHT] = { 0xFF0000FF };
uint32_t screenBuff[SCREEN_WIDTH * SCREEN_HEIGHT] = { 0xFF0000FF };

static void BlitBuff() {
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

	mu_Context* ctx = malloc(sizeof(mu_Context));
	mu_init(ctx);
	ctx->text_width = text_width;
	ctx->text_height = text_height;
	int mouseOldX = 0, mouseOldY = 0, mouseDownOld = 0;

	struct DVec2 pos = { 14, 16 };    // Player X & Y Position Vector
	struct DVec2 dir = { -1, 0 };     // Player Direction Vector
	struct DVec2 plane = { 0, 0.66 }; // Camera Plane Vector
	float dayLight = 1.7;

	int64_t TimeNow = 0; // Time at current frame
	int64_t TimeOld = 0; // Time at previous frame
	double frameTime = 0;

	char Text[128] = { '\0' };

	while (fenster_loop(&win) == 0) {
		// Speed modifiers
		double playerSpeed = frameTime * 5.0; // The constant value is in squares/second
		double rotationSpeed = frameTime * 3.0;  // The constant value is in radians/second

		for (int i = 0; i < 128; i++) {
			if (win.keys[i]) {
				switch (i) {
					case 'W': {
						if (worldMap[(int)(pos.x + dir.x * playerSpeed)][(int)pos.y] == ' ') pos.x += dir.x * playerSpeed;
						if (worldMap[(int)pos.x][(int)(pos.y + dir.y * playerSpeed)] == ' ') pos.y += dir.y * playerSpeed;
						break;
					}
					case 'S': {
						if (worldMap[(int)(pos.x - dir.x * playerSpeed)][(int)pos.y] == ' ') pos.x -= dir.x * playerSpeed;
						if (worldMap[(int)pos.x][(int)(pos.y - dir.y * playerSpeed)] == ' ') pos.y -= dir.y * playerSpeed;
						break;
					}
					case 'A': {
						// Both camera direction and camera plane must be rotated
						double oldDirX = dir.x;
						dir.x = dir.x * cos(rotationSpeed) - dir.y * sin(rotationSpeed);
						dir.y = oldDirX * sin(rotationSpeed) + dir.y * cos(rotationSpeed);
						double oldPlaneX = plane.x;
						plane.x = plane.x * cos(rotationSpeed) - plane.y * sin(rotationSpeed);
						plane.y = oldPlaneX * sin(rotationSpeed) + plane.y * cos(rotationSpeed);
						break;
					}
					case 'D': {
						// Both camera direction and camera plane must be rotated
						double oldDirX = dir.x;
						dir.x = dir.x * cos(-rotationSpeed) - dir.y * sin(-rotationSpeed);
						dir.y = oldDirX * sin(-rotationSpeed) + dir.y * cos(-rotationSpeed);
						double oldPlaneX = plane.x;
						plane.x = plane.x * cos(-rotationSpeed) - plane.y * sin(-rotationSpeed);
						plane.y = oldPlaneX * sin(-rotationSpeed) + plane.y * cos(-rotationSpeed);
						break;
					}
					default: break;
				}
			}
		}

		if (mouseOldX != win.x || mouseOldY != win.y) { // if mouse position changed
			mu_input_mousemove(ctx, win.x, win.y);
		}
		if (mouseDownOld != win.mouse) { // if mouse pressed state changed
			if (win.mouse == 1) {
				mu_input_mousedown(ctx, win.x, win.y, MU_MOUSE_LEFT);
			} else {
				mu_input_mouseup(ctx, win.x, win.y, MU_MOUSE_LEFT);
			}
		}

		for(int x = 0; x < WIDTH; x++) {
			// calculate ray position and direction
			double cameraX = 2 * x / (double)WIDTH - 1; // x-coordinate in camera space
			struct DVec2 rayDir = {
				dir.x + plane.x * cameraX,
				dir.y + plane.y * cameraX
			};
			// Which box of the map we're in
			struct IVec2 posInMapGrid = { pos.x, pos.y };

			// Length of ray from current position to next x or y-side
			struct DVec2 sideDist;

			// Length of ray from one x or y-side to next x or y-side
			struct DVec2 deltaDist = {
				(rayDir.x == 0) ? 1e30 : fabs(1 / rayDir.x),
				(rayDir.y == 0) ? 1e30 : fabs(1 / rayDir.y)
			};

			double perpWallDist;

			// What direction to step in x or y-direction (either +1 or -1)
			struct IVec2 stepDir;

			int hit = 0; // Was there a wall hit?
			int side;    // was a NS or a EW wall hit?

			// Calculate step and initial sideDist
			if (rayDir.x < 0) {
				stepDir.x = -1;
				sideDist.x = (pos.x - posInMapGrid.x) * deltaDist.x;
			} else {
				stepDir.x = 1;
				sideDist.x = (posInMapGrid.x + 1.0 - pos.x) * deltaDist.x;
			}

			if (rayDir.y < 0) {
				stepDir.y = -1;
				sideDist.y = (pos.y - posInMapGrid.y) * deltaDist.y;
			} else {
				stepDir.y = 1;
				sideDist.y = (posInMapGrid.y + 1.0 - pos.y) * deltaDist.y;
			}

			while (hit == 0) {
				// Jump to next map square, either in x-direction, or in y-direction
				if (sideDist.x < sideDist.y) {
					sideDist.x += deltaDist.x;
					posInMapGrid.x += stepDir.x;
					side = 0;
				} else {
					sideDist.y += deltaDist.y;
					posInMapGrid.y += stepDir.y;
					side = 1;
				}

				// Check if ray has hit a wall
				if (worldMap[posInMapGrid.x][posInMapGrid.y] != ' ') hit = 1;
			}

			/* Calculate distance projected on camera direction. This is the
			   shortest distance from the point where the wall is hit to the camera plane.
			   Euclidean to center camera point would give fisheye effect! */
			if (side == 0) perpWallDist = (sideDist.x - deltaDist.x);
			else           perpWallDist = (sideDist.y - deltaDist.y);

			// Calculate height of line to draw on screen
			int lineHeight = HEIGHT / perpWallDist;

			// Calculate lowest and highest pixel to fill in current stripe
			int drawStart = (-lineHeight / 2) + (HEIGHT / 2);
			if (drawStart < 0) drawStart = 0;

			int drawEnd = (lineHeight / 2) + (HEIGHT / 2);
			if (drawEnd >= HEIGHT) drawEnd = HEIGHT - 1;

			// Choose wall color
			Color col;
			switch(worldMap[posInMapGrid.x][posInMapGrid.y]) {
				case '1': col = COLOR_RGB(255, 0, 0);     break; // red
				case '2': col = COLOR_RGB(0, 255, 0);     break; // green
				case '3': col = COLOR_RGB(0, 0, 255);     break; // blue
				case '4': col = COLOR_RGB(255, 255, 255); break; // white
				default:  col = COLOR_RGB(255, 255, 0);   break; // yellow
			}

			// Give x and y sides different brightness
			float dayLightTweaked = side == 1 ? dayLight : dayLight - 0.2;
			col = COLOR_RGB(
				(COLOR_GET_R(col) / dayLightTweaked),
				(COLOR_GET_G(col) / dayLightTweaked),
				(COLOR_GET_B(col) / dayLightTweaked)
			);

			// Draw the pixels of the stripe as a vertical line
			Gfx_VertLine(pixBuff, col, x, drawStart, drawEnd, WIDTH, HEIGHT);
		}
		snprintf(
			Text, 128,
			"FPS: %d\nPlayer: %f,%f (%d,%d)\nDirection: %f,%f\nPlane: %f,%f",
			(int)(1.0 / frameTime),
			pos.x, pos.y, (int)pos.x, (int)pos.y,
			dir.x, dir.y,
			plane.x, plane.y
		);

		mu_begin(ctx);
		if (mu_begin_window_ex(ctx, "Main", mu_rect(200, 100, 160, 60), MU_OPT_NOCLOSE)) {
			mu_text(ctx, Text);

			mu_layout_row(ctx, 2, (int[]) { 54, 100 }, 0);
			mu_text(ctx, "Light:");
			mu_slider(ctx, &dayLight, 1.2, 2.2);

			mu_end_window(ctx);
		}
		mu_end(ctx);

		mouseOldX = win.x;
		mouseOldX = win.y;
		mouseDownOld = win.mouse;

		BlitBuff();
		r_drawAll(ctx, screenBuff, SCREEN_WIDTH, SCREEN_HEIGHT);
		Gfx_FillRect(pixBuff, WIDTH, HEIGHT, (const struct Rect){ 0, 0, WIDTH, HEIGHT / 2 }, COLOR_RGB((120 / dayLight), (176 / dayLight), (240 / dayLight)));
		Gfx_FillRect(pixBuff, WIDTH, HEIGHT, (const struct Rect){ 0, HEIGHT / 2, WIDTH, HEIGHT / 2 }, COLOR_RGB((54 / dayLight), (48 / dayLight), (43 / dayLight)));

		TimeOld = TimeNow;
		TimeNow = fenster_time();
		frameTime = (TimeNow - TimeOld) / 1000.0;
	}

	fenster_close(&win);
	return 0;
}

