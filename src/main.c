#define FENSTER_HEADER 1
#include "fenster.h"

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>

#include "gfx.h"
#include "vector.h"

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

int main(void) {
	struct fenster win = {
		.title = "raycaster",
		.width = SCREEN_WIDTH,
		.height = SCREEN_HEIGHT,
		.buf = screenBuff
	};
	fenster_open(&win);
	double maxFrameTime = 1000.0f / 30.0f;

	struct DVec2 player = { 22, 12 };
	struct DVec2 dir = { -1, 0 };
	struct DVec2 camPlane = { 0, 0.66 }; // 2 * atan(0.66/1.0)=66°

	const struct Rect topHalf = { 0, 0, WIDTH, HEIGHT / 2 };
	const struct Rect bottomHalf = { 0, HEIGHT / 2, WIDTH, HEIGHT / 2 };

	int64_t timeFrameStart = fenster_time();
	double frameTime = 0;
	while (fenster_loop(&win) == 0) {

		Gfx_FillRect(pixBuff, WIDTH, HEIGHT, topHalf, COLOR_RGB(34, 77, EB));
		Gfx_FillRect(pixBuff, WIDTH, HEIGHT, bottomHalf, COLOR_RGB(69, 41, 17));

		for (int x = 0; x < WIDTH; x++) {
			double camX = (2 * x) / (double)(WIDTH - 1);
			struct DVec2 rayDir = {
				(dir.x + camPlane.x) * camX,
				(dir.y + camPlane.y) * camX,
			};
			struct IVec2 map = { player.x, player.y };
			struct DVec2 sideDist;
			struct DVec2 deltaDist = {
				(rayDir.x == 0) ? 1e30 : fabs(1 / rayDir.x),
				(rayDir.y == 0) ? 1e30 : fabs(1 / rayDir.y)
			};
			double perpWallDist;
			struct IVec2 step;
			int didHit = 0;
			int side; // was a NS or a EW wall hit?

			if (rayDir.x < 0) {
				step.x = -1;
				sideDist.x = (player.x - map.x) * deltaDist.x;
			} else {
				step.x = 1;
				sideDist.x = (map.x + 1.0 - player.x) * deltaDist.x;
			}
			if (rayDir.x < 0) {
				step.y = -1;
				sideDist.y = (player.y - map.y) * deltaDist.y;
			} else {
				step.y = 1;
				sideDist.y = (map.y + 1.0 - player.y) * deltaDist.y;
			}

			while (didHit == 0) {
				// jump to next map square, either in x-direction, or in y-direction
				if (sideDist.x < sideDist.y) {
					sideDist.x += deltaDist.x;
					map.x += step.x;
					side = 0;
				} else if (sideDist.x > sideDist.y) {
					sideDist.y += deltaDist.y;
					map.y += step.y;
					side = 1;
				}

				// Check if ray has hit a wall
				if (worldMap[(map.y * mapWidth) + map.x] > 0) didHit = 1;
			}

			// Calculate distance projected on camera direction (Euclidean distance would give fisheye effect!)
			if (side == 0) perpWallDist = (sideDist.x - deltaDist.x);
			else           perpWallDist = (sideDist.y - deltaDist.y);

			// Calculate height of line to draw on screen
			int lineHeight = (int)(HEIGHT / perpWallDist);

			// Calculate lowest and highest pixel to fill in current stripe
			int drawStart = (-lineHeight / 2) + (HEIGHT / 2);
			if (drawStart < 0) drawStart = 0;

			int drawEnd = (lineHeight / 2) + (HEIGHT / 2);
			if (drawEnd >= HEIGHT) drawEnd = HEIGHT - 1;

			// choose wall color
			Color color;
			switch(worldMap[map.x][map.y]) {
				case 1:  color = COLOR_RGB(FF, 00, 00); break; // red
				case 2:  color = COLOR_RGB(00, FF, 00); break; // green
				case 3:  color = COLOR_RGB(00, 00, FF); break; // blue
				case 4:  color = COLOR_RGB(FF, FF, FF); break; // white
				default: color = COLOR_RGB(FF, FF, 00); break; // yellow
			}
			// give x and y sides different brightness
			if (side == 1) color = color / 2;

			// draw the pixels of the stripe as a vertical line
			Gfx_VertLine(pixBuff, WIDTH, HEIGHT, x, drawStart, drawEnd, color);
			// printf("%f, %f = %d,%d -> %d,%d ->>> %d\n", player.x, player.y, x, drawStart, x, drawEnd, lineHeight);
		}
		printf("FrameTime - %f\n", frameTime);

		double moveSpeed = (frameTime / 1000.0) * 5.0;
		double rotSpeed = (frameTime / 1000.0) * 3.0;

		for (int i = 0; i < 128; i++) {
			if (win.keys[i]) {
				switch (i) {
					case 'W': {
						struct DVec2 newPos = {
							(player.x + dir.x) * moveSpeed,
							(player.y + dir.y) * moveSpeed
						};
						if (worldMap[(int)newPos.x][(int)player.y] == 0) player.x += dir.x * moveSpeed;
						if (worldMap[(int)player.x][(int)newPos.y] == 0) player.y += dir.y * moveSpeed;

						break;
					}
					case 'S': {
						struct DVec2 newPos = {
							player.x - dir.x * moveSpeed,
							player.y - dir.y * moveSpeed
						};
						if (worldMap[(int)newPos.x][(int)player.y] == 0) player.x -= dir.x * moveSpeed;
						if (worldMap[(int)player.x][(int)newPos.y] == 0) player.y -= dir.y * moveSpeed;

						break;
					}
					case 'A': {
						// Both camera direction and camera plane must be rotated
						double oldDirX = dir.x;
						dir.x = dir.x * cos(rotSpeed) - dir.y * sin(rotSpeed);
						dir.y = oldDirX * sin(rotSpeed) + dir.y * cos(rotSpeed);
						double oldPlaneX = camPlane.x;
						camPlane.x = camPlane.x * cos(rotSpeed) - camPlane.y * sin(rotSpeed);
						camPlane.y = oldPlaneX * sin(rotSpeed) + camPlane.y * cos(rotSpeed);
						break;
					}
					case 'D': {
						// Both camera direction and camera plane must be rotated
						double oldDirX = dir.x;
						dir.x = dir.x * cos(-rotSpeed) - dir.y * sin(-rotSpeed);
						dir.y = oldDirX * sin(-rotSpeed) + dir.y * cos(-rotSpeed);
						double oldPlaneX = camPlane.x;
						camPlane.x = camPlane.x * cos(-rotSpeed) - camPlane.y * sin(-rotSpeed);
						camPlane.y = oldPlaneX * sin(-rotSpeed) + camPlane.y * cos(-rotSpeed);
						break;
					}
					default: break;
				}
			}
		}

		BlitBuff();

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

