#ifndef RENDER_HPP
#define RENDER_HPP

#include <SDL.h>
#include <iostream>
#include "defs.hpp"
#include "board.hpp"

#include <chrono>
#include <thread>

//Screen dimension constants
const int SCREEN_WIDTH = 160;
const int SCREEN_HEIGHT = 144;

class Render {
private:
	static Uint32 pixels[];
	static Uint32 vramTiles[];
	static Uint32 backgroundTiles[];

public:
	Board board;
	SDL_Window* window;
	SDL_Renderer *renderer;
	SDL_Texture *screenTexture;
	SDL_Texture *vramTilesTexture;
	SDL_Texture *backgorundTilesTexture;

	Render();
	Render(std::string filepath);
	~Render();
	void mainLoop();
	void render(const byte[SCREEN_HEIGHT][SCREEN_WIDTH]);
	void renderTiles(const byte[24 * 8][16 * 8]);
	void renderBackground(const byte[32 * 8][32 * 8]);
	bool init();
	void close();
};

#endif
