#include <raylib.h>
#include <iostream>
#include <unistd.h>

#include "../game_keycodes.hpp"
#include "../game_functions.hpp"
#include "../IGame.hpp"

struct RaylibColoredSquare
{
	Rectangle rect;
	Color color;
};

class Game : public IGame<RaylibColoredSquare>
{
	private:


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
			: IGame(width, height, squareSizePx)
		{
			SetTraceLogLevel(LOG_WARNING); // disables all INFO messages
			InitWindow(width * squareSizePx, height * squareSizePx, windowTitle.c_str());
			SetTargetFPS(60);

			// move window to center of the screen
			int currentMonitor = GetCurrentMonitor();
			int monitorWidth = GetMonitorWidth(currentMonitor);
			int monitorHeight = GetMonitorHeight(currentMonitor);
			int windowWidth = GetScreenWidth();
			int windowHeight = GetScreenHeight();

			SetWindowPosition((monitorWidth - windowWidth) / 2, (monitorHeight - windowHeight) / 2);

			for(int y = 0; y < _grid.getHeight(); y++)
			{
				for(int x = 0; x < _grid.getWidth(); x++)
				{
					_grid(x, y).rect.width = squareSizePx;
					_grid(x, y).rect.height = squareSizePx;
					_grid(x, y).rect.x = x * squareSizePx;
					_grid(x, y).rect.y = y * squareSizePx;
				}
			}
		}

		void setSquareSizePx(int px) override
		{
			_squareSizePx = px;
			SetWindowSize(_width * px, _height * px);
			for (int y = 0; y < _grid.getHeight(); y++)
			{
				for (int x = 0; x < _grid.getWidth(); x++)
				{
					_grid(x, y).rect.width = px;
					_grid(x, y).rect.height = px;
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
					RaylibColoredSquare &square = _grid(x, y);

					if (square.color.r == 0 && square.color.g == 0 && square.color.b == 0)
						continue;

					if (square.color.a == 0)
						continue;

					DrawRectangleRec(square.rect, square.color);
				}
			}
		}

		void render() override
		{
			BeginDrawing();
			ClearBackground(Color{0, 0, 0, 255});
			drawGrid();
			EndDrawing();
		}

		void showGameOver() override
		{
			// https://github.com/raysan5/raylib/issues/2355
			Font font = LoadFontEx("./ComicNeueSansID.ttf", 100, NULL, 0);
			Vector2 textSize = MeasureTextEx(font, "Game Over", 42, 0);
			Vector2 subtextSize = MeasureTextEx(font, "Press the space key to retry", 24, 0);

			int windowWidth = GetScreenWidth();
			int windowHeight = GetScreenHeight();

			float textX = (windowWidth - textSize.x) / 2.0;
			float textY = (windowHeight - textSize.y) / 2.0;
			float subtextX = (windowWidth - subtextSize.x) / 2.0;
			float subtextY = textY + textSize.y + 10;

			BeginDrawing();

			DrawTextEx(font, "Game Over", Vector2{textX, textY}, 42, 0, Color{255, 0, 255, 255});
			DrawTextEx(font, "Press the space key to retry", Vector2{subtextX, subtextY}, 24, 0, Color{255, 0, 255, 255});

			EndDrawing();

			UnloadFont(font);
		}

		void close() override
		{
			CloseWindow();
		}

		bool isCloseButtonClicked()
		{
			return WindowShouldClose() && !isKeyPressed(KEY_ESCAPE);
		}

		bool isKeyPressed(int key)
		{
			return IsKeyDown(key);
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

		// from https://www.raylib.com/cheatsheet/cheatsheet.html :
		// By default EndDrawing updates everything, but since we might not be
		// looping (game over screen) do it manually
		PollInputEvents();

		*keys = (int *)calloc(sizeof(int), KEY_TOTAL_COUNT + 1);
		*size = KEY_TOTAL_COUNT + 1;

		if (game->isCloseButtonClicked())
			(*keys)[EXIT_KEY] = EXIT_KEY;

		if (game->isKeyPressed(KEY_ESCAPE))
			(*keys)[EXIT_KEY] = EXIT_KEY;


		if (game->isKeyPressed(KEY_UP))
			(*keys)[UP_KEY] = UP_KEY;

		if (game->isKeyPressed(KEY_W))
			(*keys)[W_KEY] = W_KEY;


		if (game->isKeyPressed(KEY_DOWN))
			(*keys)[DOWN_KEY] = DOWN_KEY;

		if (game->isKeyPressed(KEY_S))
			(*keys)[S_KEY] = S_KEY;


		if (game->isKeyPressed(KEY_LEFT))
			(*keys)[LEFT_KEY] = LEFT_KEY;

		if (game->isKeyPressed(KEY_A))
			(*keys)[A_KEY] = A_KEY;


		if (game->isKeyPressed(KEY_RIGHT))
			(*keys)[RIGHT_KEY] = RIGHT_KEY;

		if (game->isKeyPressed(KEY_D))
			(*keys)[D_KEY] = D_KEY;


		if (game->isKeyPressed(KEY_ONE) || game->isKeyPressed(KEY_KP_1))
			(*keys)[ONE_KEY] = ONE_KEY;

		if (game->isKeyPressed(KEY_TWO) || game->isKeyPressed(KEY_KP_2))
			(*keys)[TWO_KEY] = TWO_KEY;

		if (game->isKeyPressed(KEY_THREE) || game->isKeyPressed(KEY_KP_3))
			(*keys)[THREE_KEY] = THREE_KEY;

		if (game->isKeyPressed(KEY_SPACE))
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