#include "video.h"

#include <SDL.h>
#include "debug.h"
#include "input.h"
#include "scaler.h"

#ifdef _TINSPIRE
#include "graphics.h"
#endif

SDL_Surface *screen;
SDL_Surface *screenScaled;
int screenScale;
int fullscreen;
Uint32 curTicks;
Uint32 lastTicks = 0;

SDL_Surface* back[2];

int initSDL()
{
#ifdef JOYSTICK
	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK))
#else
	if(SDL_Init(SDL_INIT_VIDEO))
#endif
	{
		return -1;
	}

	SDL_WM_SetCaption("Homing Fever", NULL);
	SDL_ShowCursor(SDL_DISABLE);
	
	back[0] = loadImage("./data/gfx/blueback.bmp");
	back[1] = loadImage("./data/gfx/redback.bmp");
	
	updateScale();

	if(screen == NULL)
	{
		return -1;
	}

#ifdef JOYSTICK
	if(SDL_NumJoysticks() > joyNum)
	{
		joyDevice = SDL_JoystickOpen(joyNum);
	}
#endif

	return 0;
}

void deinitSDL()
{
#ifdef JOYSTICK
	if(joyDevice)
	{
		SDL_JoystickClose(joyDevice);
	}
#endif
	if (screenScale > 1)
	{
		SDL_FreeSurface(screen);
	}

	SDL_Quit();
}

void updateScale()
{
#ifdef _TINSPIRE
	int flags = SDL_SWSURFACE;
#else
	int flags = SDL_HWSURFACE | SDL_DOUBLEBUF | (fullscreen ? SDL_FULLSCREEN : 0);
#endif
	if (screen != screenScaled)
	{
		SDL_FreeSurface(screen);
	}

	screenScaled = SDL_SetVideoMode(SCREEN_W * screenScale, SCREEN_H * screenScale, SCREEN_BPP, flags);
	screen = screenScale > 1 ? SDL_CreateRGBSurface(SDL_SWSURFACE, SCREEN_W, SCREEN_H, SCREEN_BPP, 0, 0, 0, 0) : screenScaled;
}

Uint32 getColor(Uint8 r, Uint8 g, Uint8 b)
{
	return SDL_MapRGB(screen->format, r, g, b);
}

SDL_Surface *loadImage(char *fileName)
{
	SDL_Surface *loadedImage;
	SDL_Surface *optimizedImage;
	Uint32 colorKey;

	if (!fileName)
	{
		fprintf(stderr, "ERROR: Filename is empty.");
		return NULL;
	}

#ifdef _TINSPIRE
	if (strstr(fileName, "data/gfx/missileBlue.bmp"))
	{
		loadedImage = nSDL_LoadImage(img_missileBlue);
	}
	else if (strstr(fileName, "data/gfx/missileYellow.bmp"))
	{
		loadedImage = nSDL_LoadImage(img_missileYellow);
	}
	else if (strstr(fileName, "data/gfx/missileRed.bmp"))
	{
		loadedImage = nSDL_LoadImage(img_missileRed);
	}
	else if (strstr(fileName, "data/gfx/player.bmp"))
	{
		loadedImage = nSDL_LoadImage(img_player);
	}
	else if (strstr(fileName, "data/gfx/smoke.bmp"))
	{
		loadedImage = nSDL_LoadImage(img_smoke);
	}
	else if (strstr(fileName, "data/gfx/redback.bmp"))
	{
		loadedImage = nSDL_LoadImage(img_redback);
	}
	else if (strstr(fileName, "data/gfx/blueback.bmp"))
	{
		loadedImage = nSDL_LoadImage(img_blueback);
	}
	else if (strstr(fileName, "data/gfx/marker.bmp"))
	{
		loadedImage = nSDL_LoadImage(img_marker);
	}
	else if (strstr(fileName, "data/gfx/font.bmp"))
	{
		loadedImage = nSDL_LoadImage(img_font);
	}
	else if (strstr(fileName, "data/gfx/fontBlack.bmp"))
	{
		loadedImage = nSDL_LoadImage(img_fontBlack);
	}
#else
	loadedImage = SDL_LoadBMP(fileName);
#endif

	if (!loadedImage)
	{
		fprintf(stderr, "ERROR: Failed to load image: %s\n", fileName);
		return NULL;
	}

	optimizedImage = SDL_CreateRGBSurface(SDL_SWSURFACE, loadedImage->w, loadedImage->h, SCREEN_BPP, 0, 0, 0, 0);
	SDL_BlitSurface(loadedImage, NULL, optimizedImage, NULL);
	SDL_FreeSurface(loadedImage);

	if (!optimizedImage)
	{
		fprintf(stderr, "ERROR: Failed to optimize image: %s\n", fileName);
		return NULL;
	}

	colorKey = SDL_MapRGB(optimizedImage->format, 255, 0, 255); /* Set transparency to magenta. */
	SDL_SetColorKey(optimizedImage, SDL_SRCCOLORKEY, colorKey);

	return optimizedImage;
}

void clipImage(SDL_Rect *source, int tileWidth, int tileHeight, int rowLength, int numOfTiles)
{
	int i;
	int j;
	int k;
	int l;

	for(i = 0, k = 0; k < numOfTiles; i+= tileHeight)
	{
		for(j = 0, l = 0; l < rowLength; j+= tileWidth)
		{
			source[k].x = j;
			source[k].y = i;
			source[k].w = tileWidth;
			source[k].h = tileHeight;
			++k;
			++l;
		}
		l = 0;
	}
}

void drawImage(SDL_Surface *source, SDL_Rect *clip, SDL_Surface *destination, int x, int y)
{
	SDL_UnlockSurface(screen);
	SDL_Rect offset;
	offset.x = x;
	offset.y = y;

	SDL_BlitSurface(source, clip, destination, &offset);
}

void drawBackground(SDL_Surface *destination, Uint32 color)
{
	SDL_Rect r;

	r.x = 0;
	r.y = 0;
	r.w = 320;
	r.h = 240;
	//printf("Color back : %d\n", color);
	switch(color)
	{
		case 16:
		SDL_BlitSurface(back[0], &r, destination, NULL);
		break;
		case 32768:
		SDL_BlitSurface(back[1], &r, destination, NULL);
		break;
	}
	//SDL_FillRect(destination, NULL, color);
}

void drawPoint(SDL_Surface *destination, int x, int y, Uint32 color)
{
	SDL_Rect r;

	r.x = x;
	r.y = y;
	r.w = 1;
	r.h = 1;

	SDL_FillRect(destination, &r, color);
}

int frameLimiter()
{
#if defined(NO_FRAMELIMIT)
	return 0;
#else
	int t;

	curTicks = SDL_GetTicks();
	t = curTicks - lastTicks;

	if(t >= 1000/FPS)
	{
		lastTicks = curTicks;
		return 0;
	}

	SDL_Delay(1);
	return 1;
#endif
}

void flipScreen()
{
#if defined(SCREEN_SCALE) != 1
	switch (screenScale)
	{
		case 1:
		break;
		case 2:
			upscale2((uint32_t *)screenScaled->pixels, (uint32_t *)screen->pixels);
		break;
	}
#endif

	SDL_Flip(screenScaled);

#ifndef DEBUG
	if (debugSlowMotion)
	{
		SDL_Delay(250);
	}
#endif
}

void clearScreen()
{
	//SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
}
