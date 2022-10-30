#include <dlfcn.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <chrono>
#include <vector>
#include <algorithm>
#include <random>
#include <map>
#include <X11/Xlib.h>

#include "./game_keycodes.hpp"
#include "./game_functions.hpp"

#define REFRESH_FPS 60 // To handle inputs
#define GAME_FPS 20 // Game ticks per second (should not be lower then REFRESH_FPS)
#define SQUARE_SIZE_PX 10

enum e_gamemode
{
	GAMEMODE_NORMAL,
	GAMEMODE_FASTASFUCK,
};

int g_mapWidth;
int g_mapHeight;

typedef struct s_nibbler_dynamic_library
{
	void *handle;
	init_nibbler_t init_nibbler;
	get_pressed_keys_t get_pressed_keys;
	clear_screen_t clear_screen;
	set_square_color_t set_square_color;
	render_t render;
	show_game_over_t show_game_over;
	exit_nibbler_t exit_nibbler;

} t_nibbler_dynamic_library;

void *loadDynamicSymbol(void *handle, const char *symbol)
{
	void *func = dlsym(handle, symbol);
	if (func == NULL)
	{
		std::cout << "Error while loading symbol " << symbol << " : " << dlerror() << std::endl;
		exit(EXIT_FAILURE);
	}
	return func;
}

void *dlOpenOrExit(const char *path)
{
	void *handle = dlopen(path, RTLD_LAZY);
	if (handle == NULL)
	{
		std::cout << "Error while loading " << path << " : " << dlerror() << std::endl;
		exit(EXIT_FAILURE);
	}

	std::cout << "Successfully loaded " << path << std::endl;
	return handle;
}

void openNibblerDynamicLibraryOrExit(const char *path, t_nibbler_dynamic_library &lib)
{
	lib.handle = dlOpenOrExit(path);
	lib.init_nibbler = (init_nibbler_t)loadDynamicSymbol(lib.handle, "init_nibbler");
	lib.get_pressed_keys = (get_pressed_keys_t)loadDynamicSymbol(lib.handle, "get_pressed_keys");
	lib.clear_screen = (clear_screen_t)loadDynamicSymbol(lib.handle, "clear_screen");
	lib.set_square_color = (set_square_color_t)loadDynamicSymbol(lib.handle, "set_square_color");
	lib.render = (render_t)loadDynamicSymbol(lib.handle, "render");
	lib.show_game_over = (show_game_over_t)loadDynamicSymbol(lib.handle, "show_game_over");
	lib.exit_nibbler = (exit_nibbler_t)loadDynamicSymbol(lib.handle, "exit_nibbler");
}

class Snake
{
	private:
		std::vector<std::pair<int, int>> _body;
		int _vx;
		int _vy;

	public:
		// xStart = tail xEnd = head position (if == means 1 block)
		Snake(int xStart, int xEnd, int y)
		{
			_vx = 1;
			_vy = 0;
			for (int x = xStart; x <= xEnd; x++)
				_body.insert(_body.begin(), std::make_pair(x, y));
		}

		void grow()
		{
			_body.push_back(std::make_pair(_body.back().first, _body.back().second));
		}

		void move()
		{
			for (int i = _body.size() - 1; i > 0; i--)
			{
				_body[i].first = _body[i - 1].first;
				_body[i].second = _body[i - 1].second;
			}
			_body[0].first += _vx;
			_body[0].second += _vy;
		}

		int getHeadX() const
		{
			return _body[0].first;
		}

		int getHeadY() const
		{
			return _body[0].second;
		}

		int getVelX() const
		{
			return _vx;
		}

		int getVelY() const
		{
			return _vy;
		}

		void setDirection(int vx, int vy)
		{
			if (vx != -_vx)
				_vx = vx;

			if (vy != -_vy)
				_vy = vy;
		}

		bool isSelfColliding() const
		{
			for (auto it = _body.begin() + 1; it != _body.end(); it++)
			{
				if (it->first == _body[0].first && it->second == _body[0].second)
					return true;
			}
			return false;
		}

		std::vector<std::pair<int, int>> getBody()
		{
			return _body;
		}
};

std::pair<int, int> &generateFood(std::pair<int, int> &food, std::vector<std::pair<int, int>> &snakeBody)
{
	food.first = rand() % g_mapWidth;
	food.second = rand() % g_mapHeight;

	// No space left for food
	if (snakeBody.size() >= (unsigned int) (g_mapWidth * g_mapHeight))
		return food;

	for (std::pair<int, int> &bodyPart : snakeBody)
	{
		if (bodyPart.first == food.first && bodyPart.second == food.second)
			return generateFood(food, snakeBody);
	}
	return food;
}

void switchLibrary(const char *newLibPath, t_nibbler_dynamic_library &lib)
{
	lib.exit_nibbler();
	dlclose(lib.handle);
	openNibblerDynamicLibraryOrExit(newLibPath, lib);
}

void usage(char *executableName)
{
	std::cout << "Usage: " << executableName << " <Map Width> <Map Height> <Mode>" << std::endl;
	std::cout << "Mode can be 'classic' or 'faf' (fast as fuck)" << std::endl;
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	if (argc < 3)
		usage(argv[0]);

	Display *display = XOpenDisplay(NULL);
	if (!display)
	{
		std::cerr << "Cannot get primary monitor" << std::endl;
		return(EXIT_FAILURE);
	}
	Screen *screen = DefaultScreenOfDisplay(display);

	int screenWidth = screen->width;
	int screenHeight = screen->height;

	int mapWidth;
	int mapHeight;

	try
	{
		mapWidth = std::stoi(argv[1]);
		mapHeight = std::stoi(argv[2]);
	}
	catch (const std::exception &e)
	{
		usage(argv[0]);
	}

	if (mapWidth * SQUARE_SIZE_PX > screenWidth || mapHeight * SQUARE_SIZE_PX > screenHeight)
	{
		std::cerr << "Map is too big\n";
		usage(argv[0]);
	}

	if (mapWidth < 6 || mapHeight < 4)
	{
		std::cerr << "Map is too small\n";
		usage(argv[0]);
	}

	g_mapWidth = (unsigned int) mapWidth;
	g_mapHeight = (unsigned int) mapHeight;

	int gamemode = GAMEMODE_NORMAL;

	if (argc >= 4)
	{
		std::string mode = argv[3];
		if (mode == "normal")
			gamemode = GAMEMODE_NORMAL;
		else if (mode == "faf" || mode == "fastasfuck")
			gamemode = GAMEMODE_FASTASFUCK;
		else
		{
			std::cerr << "Wrong gamemode\n";
			usage(argv[0]);
		}
	}

	int gameFps = GAME_FPS;


	std::vector<std::pair<const char *, const char *>> libs = { {"./sdl/libnibbler_sdl.so", "SDL2"},
									  							{"./sfml/libnibbler_sfml.so", "SFML"},
																{"./raylib/libnibbler_raylib.so", "Raylib"} };

	t_nibbler_dynamic_library lib;
	srand(time(NULL));

	int libIndex = rand() % libs.size();

	openNibblerDynamicLibraryOrExit(libs[libIndex].first, lib);
	lib.init_nibbler(g_mapWidth, g_mapHeight, SQUARE_SIZE_PX, (std::string("Nibbler - ") + libs[libIndex].second).c_str());

	std::vector<int> alreadyPressedKeys;
	int currentFrame = 0;
	Snake snake(g_mapWidth / 2 - 2, g_mapWidth / 2 + 2, g_mapHeight / 2);
	auto snakeBody = snake.getBody();
	std::pair<int, int> food = generateFood(food, snakeBody);

	int vy = 0;
	int vx = 1;
	bool gameOver = false;

	game_loop:
	while (1)
	{
		auto frameStartTime = std::chrono::high_resolution_clock::now();

		std::vector<int> pressedKeys;
		int *keys = NULL;
		int size = 0;
		lib.get_pressed_keys(&keys, &size);
		for (int i = 0; i < size; i++)
		{
			if (keys[i] == NONE_KEY)
				continue;

			pressedKeys.push_back(keys[i]);
		}
		free(keys);

		for (int key : pressedKeys)
		{
			if (std::find(alreadyPressedKeys.begin(), alreadyPressedKeys.end(), key) != alreadyPressedKeys.end())
				continue;

			alreadyPressedKeys.push_back(key);

			// Manage newly pressed key logic
			switch(key)
			{
				case EXIT_KEY:
					std::cout << "Exit\n";
					lib.exit_nibbler();
					exit(EXIT_SUCCESS);
					break;

				case UP_KEY:
					// Still check so that if snake goes LEFT, user presses UP then RIGHT, it will go right instead of nothing
					if (snake.getVelY() != 1)
					{
						vy = -1;
						vx = 0;
					}
					// std::cout << "Go up\n";
					break;

				case DOWN_KEY:
					if (snake.getVelY() != -1)
					{
						vy = 1;
						vx = 0;
					}
					// std::cout << "Go down\n";
					break;

				case LEFT_KEY:
					if (snake.getVelX() != 1)
					{
						vy = 0;
						vx = -1;
					}
					// std::cout << "Go left\n";
					break;

				case RIGHT_KEY:
					if (snake.getVelX() != -1)
					{
						vy = 0;
						vx = 1;
					}
					// std::cout << "Go right\n";
					break;

				case ONE_KEY:
					std::cout << "Switch to lib #1\n";
					if (libs.size() < 1 || libIndex == 0)
						break;

					libIndex = 0;
					switchLibrary(libs[0].first, lib);
					gameOver = false;
					lib.init_nibbler(g_mapWidth, g_mapHeight, SQUARE_SIZE_PX, (std::string("Nibbler - ") + libs[0].second).c_str());
					goto game_loop;

					break;

				case TWO_KEY:
					std::cout << "Switch to lib #2\n";
					if (libs.size() < 2 || libIndex == 1)
						break;

					libIndex = 1;
					switchLibrary(libs[1].first, lib);
					gameOver = false;
					lib.init_nibbler(g_mapWidth, g_mapHeight, SQUARE_SIZE_PX, (std::string("Nibbler - ") + libs[1].second).c_str());
					goto game_loop;

					break;

				case THREE_KEY:
					std::cout << "Switch to lib #3\n";
					if (libs.size() < 3 || libIndex == 2)
						break;

					libIndex = 2;
					switchLibrary(libs[2].first, lib);
					gameOver = false;
					lib.init_nibbler(g_mapWidth, g_mapHeight, SQUARE_SIZE_PX, (std::string("Nibbler - ") + libs[2].second).c_str());
					goto game_loop;

					break;

				case SPACE_KEY:
					if (gameOver)
					{
						std::cout << "Restart\n";
						gameOver = false;
						snake = Snake(g_mapWidth / 2 - 2, g_mapWidth / 2 + 2, g_mapHeight / 2);
						vx = 1;
						vy = 0;
						snake.setDirection(1, 0);
						snakeBody = snake.getBody();
						food = generateFood(food, snakeBody);
					}
					break;
			}


		}

		// Remove keys that were released
		for (auto keyIt = alreadyPressedKeys.begin(); keyIt != alreadyPressedKeys.end(); )
		{
			auto keyInKeys = std::find(pressedKeys.begin(), pressedKeys.end(), *keyIt);

			if (keyInKeys != pressedKeys.end())
			{
				++keyIt;
				continue;
			}

			keyIt = alreadyPressedKeys.erase(keyIt);
		}

		currentFrame++;
		if (currentFrame > 2000000000) // to avoid overflow
			currentFrame = 0;

		if (currentFrame % (REFRESH_FPS / gameFps) == 0)
		{
			if (gameOver)
				continue;
			// TODO gameTick function
			// std::cout << "Game tick" << std::endl;
			lib.clear_screen();

			// us temp variable vx / vy in case the user moves very fast eg. UP, RIGHT (while going left) between
			// game ticks they cannot make a 180° turn
			snake.setDirection(vx, vy);

			snake.move();

			if (snake.getHeadX() >= g_mapWidth || snake.getHeadX() < 0 || snake.getHeadY() >= g_mapHeight || snake.getHeadY() < 0 \
				|| snake.isSelfColliding())
			{
				std::cout << "Game over" << std::endl;
				gameOver = true;
				gameFps = GAME_FPS;
				lib.show_game_over();
				continue;
				// see https://github.com/microsoft/wslg/issues/445 for segfault, always do LIBGL_ALWAYS_SOFTWARE=1  ./nibbler  on WSL
				// exit(0);
			}

			if (snake.getHeadX() == food.first && snake.getHeadY() == food.second)
			{
				if (gamemode == GAMEMODE_FASTASFUCK)
				{
					gameFps++;
					if (gameFps > REFRESH_FPS)
						gameFps = REFRESH_FPS;
				}
				snake.grow();
				food = generateFood(food, snakeBody);
			}

			snakeBody = snake.getBody();
			for (std::pair<int, int> &bodyPart : snakeBody)
				lib.set_square_color(bodyPart.first, bodyPart.second, 0, 255, 0);


			lib.set_square_color(snakeBody.back().first, snakeBody.back().second, 128, 255, 0);
			lib.set_square_color(snake.getHeadX(), snake.getHeadY(), 0, 128, 0);

			lib.set_square_color(food.first, food.second, 255, 0, 0);

			lib.render();
		}

		std::chrono::microseconds frameDuration;
		do
		{
			auto now = std::chrono::high_resolution_clock::now();
			frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(now - frameStartTime);
			usleep(500);
		} while(frameDuration.count() < 1000 * 1000 / REFRESH_FPS);

		// std::cout << "Frame took " << (frameDuration.count() / 1000) << "ms to run" << std::endl;
	}


	// pthread_join(thread, NULL);
	dlclose(lib.handle);
	return 1;
}