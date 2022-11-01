#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <unistd.h>

#include "../game_keycodes.hpp"
#include "../game_functions.hpp"
#include "../IGame.hpp"
#include "../HSL_RGB.hpp"

struct SDLColoredSquare
{
	SDL_Rect rect;
	SDL_Color color;
};

class Game : public IGame<SDLColoredSquare>
{
	private:
		SDL_Window *_window;
		SDL_Renderer *_renderer;
		int _hslShift;

	public:
		// https://stackoverflow.com/questions/308276/can-i-call-a-constructor-from-another-constructor-do-constructor-chaining-in-c
		Game() : Game(4, 4, 10, "Hmmmmmm")
		{
		}

		~Game()
		{
		}

		Game(const Game &other)
			: IGame(other)
		{
		}

		Game &operator=(const Game &other)
		{
			IGame::operator=(other);
			return *this;
		}

		Game(int width, int height, int squareSizePx, const std::string &windowTitle)
			: IGame(width, height, squareSizePx), _hslShift(0)
		{
			if (SDL_Init(SDL_INIT_VIDEO) != 0)
			{
				std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
				exit(1);
			}

			_window = SDL_CreateWindow(windowTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width * squareSizePx, height * squareSizePx, SDL_WINDOW_SHOWN);
			if (_window == nullptr) // https://stackoverflow.com/questions/20509734/null-vs-nullptr-why-was-it-replaced
			{
				std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
				SDL_Quit();
				exit(1);
			}

			_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (_renderer == nullptr)
			{
				std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
				SDL_DestroyWindow(_window);
				SDL_Quit();
				exit(1);
			}

			SDL_GL_SetSwapInterval(1); // vsync
			for(int y = 0; y < _grid.getHeight(); y++)
			{
				for(int x = 0; x < _grid.getWidth(); x++)
				{
					_grid(x, y).rect.w = squareSizePx;
					_grid(x, y).rect.h = squareSizePx;
					_grid(x, y).rect.x = x * squareSizePx;
					_grid(x, y).rect.y = y * squareSizePx;
				}
			}
		}

		void setSquareSizePx(int px) override
		{
			_squareSizePx = px;
			SDL_SetWindowSize(_window, _width * px, _height * px);
			for (int y = 0; y < _grid.getHeight(); y++)
			{
				for (int x = 0; x < _grid.getWidth(); x++)
				{
					_grid(x, y).rect.w = px;
					_grid(x, y).rect.h = px;
				}
			}
		}

		void setSquareColor(int x, int y, int r, int g, int b) override
		{
			_grid(x, y).color.r = r;
			_grid(x, y).color.g = g;
			_grid(x, y).color.b = b;
			_grid(x, y).color.a = 255;
		}

		void setSquaresBlack() override
		{
			for (int y = 0; y < _grid.getHeight(); y++)
				for (int x = 0; x < _grid.getWidth(); x++)
					setSquareColor(x, y, 0, 0, 0);
		}

		void drawGrid() override
		{
			for (int y = 0; y < _grid.getHeight(); y++)
			{
				for (int x = 0; x < _grid.getWidth(); x++)
				{
					SDLColoredSquare &square = _grid(x, y);
					if (square.color.r == 0 && square.color.g == 0 && square.color.b == 0)
						continue;

					if (square.color.a == 0)
						continue;

					RGB rotated = rotateRGBUsingHSL(square.color.r, square.color.g, square.color.b, _hslShift);

					SDL_SetRenderDrawColor(_renderer, (int) rotated.r, (int) rotated.g, (int) rotated.b, square.color.a);
					SDL_RenderFillRect(_renderer, &square.rect);
				}
			}
		}

		void render() override
		{
			SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);
			SDL_RenderClear(_renderer);
			drawGrid();
			_hslShift += 4;
			if (_hslShift > 359)
				_hslShift %= 360;
			SDL_RenderPresent(_renderer);
		}

		void showGameOver() override
		{
			if (TTF_Init() == -1)
			{
				std::cerr << "TTF_Init Error: " << TTF_GetError() << std::endl;
				exit(2);
			}

			// https://stackoverflow.com/questions/22886500/how-to-render-text-in-sdl2
			TTF_Font *font = TTF_OpenFont("./ComicNeueSansID.ttf", 42);
			TTF_Font *subfont = TTF_OpenFont("./ComicNeueSansID.ttf", 24);
			if (font == nullptr || subfont == nullptr)
			{
				std::cerr << "TTF_OpenFont Error: " << TTF_GetError() << std::endl;
				return;
			}

			TTF_SetFontStyle(font, TTF_STYLE_BOLD);
			SDL_Color color = {255, 0, 255, 255};
			SDL_Surface *surfaceText = TTF_RenderText_Blended(font, "Game Over", color);
			SDL_Surface *surfaceSubText = TTF_RenderText_Blended(subfont, "Press the space key to restart", color);
			if (surfaceText == nullptr || surfaceSubText == nullptr)
			{
				std::cerr << "TTF_RenderText_Blended Error: " << TTF_GetError() << std::endl;
				return;
			}

			SDL_Texture *textureText = SDL_CreateTextureFromSurface(_renderer, surfaceText);

			SDL_Rect rectText;
			rectText.x = _width * _squareSizePx / 2 - surfaceText->w / 2;
			rectText.y = _height * _squareSizePx / 2 - surfaceText->h / 2;
			rectText.w = surfaceText->w;
			rectText.h = surfaceText->h;

			SDL_Texture *textureSubText = SDL_CreateTextureFromSurface(_renderer, surfaceSubText);

			SDL_Rect rectSubText;
			rectSubText.x = _width * _squareSizePx / 2 - surfaceSubText->w / 2;
			rectSubText.y = _height * _squareSizePx / 2 - surfaceSubText->h / 2 + surfaceText->h;
			rectSubText.w = surfaceSubText->w;
			rectSubText.h = surfaceSubText->h;

			SDL_RenderCopy(_renderer, textureText, NULL, &rectText);
			SDL_RenderCopy(_renderer, textureSubText, NULL, &rectSubText);

			SDL_FreeSurface(surfaceText);
			SDL_FreeSurface(surfaceSubText);
			SDL_DestroyTexture(textureText);
			SDL_DestroyTexture(textureSubText);
			TTF_CloseFont(font);
			TTF_CloseFont(subfont);

			SDL_RenderPresent(_renderer);
		}

		void close() override
		{
			SDL_DestroyRenderer(_renderer);
			SDL_DestroyWindow(_window);
			SDL_Quit();
		}

		bool pollEvent(SDL_Event &event)
		{
			return SDL_PollEvent(&event);
		}

		bool isKeyPressed(SDL_Keycode key)
		{
			const Uint8 *state = SDL_GetKeyboardState(NULL);
			return state[SDL_GetScancodeFromKey(key)];
		}
};

Game *game = NULL;


// https://www.parallelrealities.co.uk/tutorials/shooter/shooter1.php

#ifdef __cplusplus
extern "C" {
#endif
	int init_nibbler(int width, int height, int cell_size, const char *window_name)
	{
		if (game != NULL)
			return -1;

		std::cout << "Initializing Nibbler" << std::endl;

		game = new Game(width, height, cell_size, window_name);

		std::cout << "Successfully initialized Nibbler" << std::endl;

		return 0;
	}

	int get_pressed_keys(int **keys, int *size)
	{
		if (game == NULL)
			return -1;

		*keys = (int *)calloc(sizeof(int), KEY_TOTAL_COUNT + 1);
		*size = KEY_TOTAL_COUNT + 1;

		SDL_Event event;
		// pollEvents
		while (game->pollEvent(event))
		{
			if (event.type == SDL_QUIT)
				(*keys)[EXIT_KEY] = EXIT_KEY;
		}

		if (game->isKeyPressed(SDLK_ESCAPE))
			(*keys)[EXIT_KEY] = EXIT_KEY;


		if (game->isKeyPressed(SDLK_UP))
			(*keys)[UP_KEY] = UP_KEY;

		if (game->isKeyPressed(SDLK_w))
			(*keys)[W_KEY] = W_KEY;


		if (game->isKeyPressed(SDLK_DOWN))
			(*keys)[DOWN_KEY] = DOWN_KEY;

		if (game->isKeyPressed(SDLK_s))
			(*keys)[S_KEY] = S_KEY;


		if (game->isKeyPressed(SDLK_LEFT))
			(*keys)[LEFT_KEY] = LEFT_KEY;

		if (game->isKeyPressed(SDLK_a))
			(*keys)[A_KEY] = A_KEY;


		if (game->isKeyPressed(SDLK_RIGHT))
			(*keys)[RIGHT_KEY] = RIGHT_KEY;

		if (game->isKeyPressed(SDLK_d))
			(*keys)[D_KEY] = D_KEY;

		if (game->isKeyPressed(SDLK_1) || game->isKeyPressed(SDLK_KP_1))
			(*keys)[ONE_KEY] = ONE_KEY;

		if (game->isKeyPressed(SDLK_2) || game->isKeyPressed(SDLK_KP_2))
			(*keys)[TWO_KEY] = TWO_KEY;

		if (game->isKeyPressed(SDLK_3) || game->isKeyPressed(SDLK_KP_3))
			(*keys)[THREE_KEY] = THREE_KEY;

		if (game->isKeyPressed(SDLK_SPACE))
			(*keys)[SPACE_KEY] = SPACE_KEY;

		return 0;
	}


	void clear_screen()
	{
		if (game == NULL)
			return;

		game->setSquaresBlack();
	}

	void set_square_color(int x, int y, int r, int g, int b)
	{
		if (game == NULL)
			return;

		game->setSquareColor(x, y, r, g, b);
	}

	void render()
	{
		if (game == NULL)
			return;

		game->render();
	}

	void show_game_over()
	{
		if (game == NULL)
			return;

		game->showGameOver();
	}

	void exit_nibbler()
	{
		if (game == NULL)
			return;

		game->close();
		delete game;
		game = NULL;
	}


#ifdef __cplusplus
}
#endif