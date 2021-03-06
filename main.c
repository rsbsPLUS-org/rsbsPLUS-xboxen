//SPDX-License-Identifier: BSD-3-Clause
//SPDX-FileCopyrightText: 2020 Lorenzo Cauli (lorecast162)

//XBOX Defines
#include <hal/debug.h>
#include <hal/video.h>
#include <hal/xbox.h>
#include <windows.h>
//SDL Defines
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_gamecontroller.h>

static void printSDLErrorAndReboot(void);

static void printIMGErrorAndReboot(void);

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define FPS 60

void game(void){
	//fix to make joystick work
	SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

	//declare SDL window, event and renderer
	SDL_Window* window = NULL;
	SDL_Event event;
	SDL_Renderer* renderer = NULL;

	//init SDL video and game controller
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't init SDL.\n");
		printSDLErrorAndReboot();
	}

	//create window
	window = SDL_CreateWindow("rsbsPLUS-xboxen", 
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
			SCREEN_WIDTH, SCREEN_HEIGHT,
			SDL_WINDOW_SHOWN);

	//throw error if window wasn't created
	if (window == NULL) {
		debugPrint("Window could not be created.\n");
		SDL_VideoQuit();
		printSDLErrorAndReboot();
	}

	//create renderer
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

	//init SDL_image with jpg
	if (!(IMG_Init(IMG_INIT_JPG) & IMG_INIT_JPG)){
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't init SDL_image.\n");
		printSDLErrorAndReboot();
	}

	//load up circle textures
	SDL_Texture* blauTexture = IMG_LoadTexture(renderer, "D:\\res\\blau.bmp");
	SDL_Texture* rotTexture = IMG_LoadTexture(renderer, "D:\\res\\rot.bmp");
	SDL_Texture* gruenTexture = IMG_LoadTexture(renderer, "D:\\res\\gruen.bmp");
	//load bg texture
	SDL_Texture* bgTexture = IMG_LoadTexture(renderer,   "D:\\res\\bg.bmp");

	//create rects for circles and the screen. used in rendering
	SDL_Rect sRect = {256,176,128,128};
	SDL_Rect screenRect = {0,0,SCREEN_WIDTH,SCREEN_HEIGHT};


	//log how many joysticks were found
	debugPrint("%i joysticks were found.\n\n", SDL_NumJoysticks() );

	//create joystick and open joystick 0
	SDL_GameController* joystick;
	SDL_GameControllerEventState(SDL_ENABLE);
	joystick = SDL_GameControllerOpen(0);

	//set current circle to default circle (red)
	SDL_Texture* curCircle = rotTexture;

	//declare game vars
	uint8_t moveDelta = 10;

	//declare variable to stop game loop
	char done = 0;
	while (!done) {
		if (SDL_GetTicks() < 1000 / FPS) {
			SDL_Delay((1000 / FPS) - SDL_GetTicks());
		}
		//wait for vblank
		XVideoWaitForVBlank();
		//event loop
		while (SDL_PollEvent(&event)) {
			//check event type
			switch (event.type) {
				case SDL_QUIT:
					done = 1;
					break;
			}
		}
		
		//if right is pressed and resulting x is not out of the screen make circle go right 
		if (SDL_GameControllerGetButton(joystick, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) && (sRect.x + moveDelta <= SCREEN_WIDTH - 128 ) ) sRect.x += moveDelta;
		
		//if left is set and resulting x is not out of the screen make circle go left
		else if (SDL_GameControllerGetButton(joystick, SDL_CONTROLLER_BUTTON_DPAD_LEFT) && (sRect.x > 0) ) sRect.x -= moveDelta;

		//if up is pressed set to blue
		if (SDL_GameControllerGetButton(joystick, SDL_CONTROLLER_BUTTON_DPAD_UP)) curCircle = blauTexture;
		//otherwise if down is pressed set to green
		else if (SDL_GameControllerGetButton(joystick, SDL_CONTROLLER_BUTTON_DPAD_DOWN)) curCircle = gruenTexture;
		//otherwise set to red
		else curCircle = rotTexture;

		//render out background to clear screen
		SDL_RenderCopy(renderer, bgTexture,    NULL, &screenRect);
		//render circle
		SDL_RenderCopy(renderer, curCircle,    NULL, &sRect);
		//"blit" render to window
		SDL_RenderPresent(renderer);
		SDL_UpdateWindowSurface(window);
	}

	//stop video system
	SDL_VideoQuit();
	//close joystick stream
	SDL_GameControllerClose(joystick);
}

int main() {
	//set video mode 
	XVideoSetMode(SCREEN_WIDTH,SCREEN_HEIGHT,32,REFRESH_DEFAULT);
	game();
	return 0;
}

//functions for error reporting
static void printSDLErrorAndReboot(void) {
	debugPrint("SDL_Error: %s\n", SDL_GetError());
	debugPrint("Rebooting in 5 seconds.\n");
	Sleep(5000);
	XReboot();
}

static void printIMGErrorAndReboot(void) {
	debugPrint("SDL_Image Error: %s\n", IMG_GetError());
	debugPrint("Rebooting in 5 seconds.\n");
	Sleep(5000);
	XReboot();
}
