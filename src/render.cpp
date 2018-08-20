#include "render.hpp"
#include <ctime>

Uint32 Render::pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
Uint32 Render::vramTiles[16 * 8 * 24 * 8];
Uint32 Render::backgroundTiles[32 * 8 * 32 * 8];

Render::Render() : board("..\\..\\ROM\\Super Mario Land 2 - 6 Golden Coins (UE) (V1.2) [!].gb") {
	if(board.memory != nullptr && init()) mainLoop();
}

Render::Render(std::string filepath) : board(filepath) {
	if(board.memory != nullptr && init()) mainLoop();
}

Render::~Render() {
	close();
}

void Render::mainLoop() {
	SDL_Event e;
	bool shouldQuit = false;
	//board.mbc1.readRom("..\\..\\ROM\\Super Mario Land 2 - 6 Golden Coins (UE) (V1.2) [!].gb");
	//board.memory.readRom("..\\..\\ROM\\Kirby's Dream Land (U) [!].gb");
	//board.mbc1.readRom("..\\..\\ROM\\Tetris (World).gb");
	//board.mbc1.readRom("..\\..\\ROM\\Pokemon Red.gb");
	//board.mbc1.readRom("..\\..\\ROM\\bgbtest.gb");
	//board.mbc1.readRom("..\\..\\ROM\\Tests\\cpu_instrs\\cpu_instrs.gb");
	//board.mbc1.readRom("..\\..\\ROM\\Tests\\cpu_instrs\\individual\\06-ld r,r.gb");

	// Timing
	std::chrono::nanoseconds lcd = std::chrono::nanoseconds(16742706);
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::chrono::system_clock::time_point oldTime = std::chrono::system_clock::now();
	std::chrono::system_clock::time_point newTime = std::chrono::system_clock::now();

	
	while(!shouldQuit) {
		while(SDL_PollEvent(&e) != 0)
			if(e.type == SDL_QUIT) shouldQuit = true;

		const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
		if(currentKeyStates[SDL_SCANCODE_ESCAPE]) shouldQuit = true;

		board.memory->updateJoypad(currentKeyStates);

		newTime = std::chrono::system_clock::now();

		board.step();

		if(board.lcd.screenRedrawn) {
			auto deltaT = std::chrono::duration_cast<std::chrono::nanoseconds>(newTime - oldTime);

			static int i = 0;
			i++;
			if(i > 60) i = 0;

			if(i == 0) {
				std::cout << "Frame duration: " << deltaT.count() << std::endl;
			}

			board.lcd.screenRedrawn = false;
			render(board.lcd.screen);
			std::this_thread::sleep_until(oldTime + lcd);
			oldTime = newTime;


			auto deltaTSleep = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - oldTime);
			if(i == 0) {
				std::cout << "Sleep duration: " << deltaTSleep.count() << std::endl;
				std::cout << "Sleep percentage: " << ((double) deltaTSleep.count() / deltaT.count() * 100) << std::endl << std::endl;
			}
		}
	


	}
}

void Render::render(const byte lcd[SCREEN_HEIGHT][SCREEN_WIDTH]) {
	for(int i = 0; i < SCREEN_HEIGHT; i++) {
		for(int j = 0; j < SCREEN_WIDTH; j++) {
			byte value = lcd[i][j];
			Uint32 color = 0xFFFFFF / 3 * value;
			pixels[i*SCREEN_WIDTH + j] = color;
		}
	}

	SDL_UpdateTexture(screenTexture, NULL, pixels, SCREEN_WIDTH * sizeof(Uint32));
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0x00, 0xFF);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, screenTexture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

void Render::renderTiles(const byte tiles[24 * 8][16 * 8]) {
	for(int i = 0; i < 24 * 8; i++) {
		for(int j = 0; j < 16 * 8; j++) {
			byte value = tiles[i][j];
			Uint32 color = 0xFFFFFF / 3 * value;
			vramTiles[i*16*8 + j] = color;
		}
	}

	SDL_UpdateTexture(vramTilesTexture, NULL, vramTiles, 16 * 8 * sizeof(Uint32));
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0x00, 0xFF);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, vramTilesTexture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

void Render::renderBackground(const byte tiles[32 * 8][32 * 8]) {
	for(int i = 0; i < 32 * 8; i++) {
		for(int j = 0; j < 32 * 8; j++) {
			byte value = tiles[i][j];
			Uint32 color = 0xFFFFFF / 3 * value;
			vramTiles[i * 32 * 8 + j] = color;
		}
	}

	SDL_UpdateTexture(backgorundTilesTexture, NULL, vramTiles, 32 * 8 * sizeof(Uint32));
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0x00, 0xFF);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, backgorundTilesTexture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

bool Render::init() {
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cout << "Couldn't init SDL: " << SDL_GetError() << std::endl;
		return false;
	}
	window = SDL_CreateWindow("Emu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH * 3, SCREEN_HEIGHT * 3, 0);
	if (window == NULL) {
		std::cout << "Couldn't create window: " << SDL_GetError() << std::endl;
		return false;
	} 
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if(renderer == NULL) {
		std::cout << "Couldn't create renderer: " << SDL_GetError() << std::endl;
		return false;
	}
	screenTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, SCREEN_WIDTH, SCREEN_HEIGHT);
	if(screenTexture == NULL) {
		std::cout << "Couldn't create screen texture: " << SDL_GetError() << std::endl;
		return false;
	}
	vramTilesTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 16 * 8, 24 * 8);
	if(vramTilesTexture == NULL) {
		std::cout << "Couldn't create VRAM tiles texture: " << SDL_GetError() << std::endl;
		return false;
	}
	backgorundTilesTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 32 * 8, 32 * 8);
	if(backgorundTilesTexture == NULL) {
		std::cout << "Couldn't create backgorund tiles texture: " << SDL_GetError() << std::endl;
		return false;
	}
	SDL_SetWindowResizable(window, SDL_TRUE);
	return true;
}



void Render::close() {
	SDL_DestroyTexture(backgorundTilesTexture);
	SDL_DestroyTexture(vramTilesTexture);
	SDL_DestroyTexture(screenTexture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
