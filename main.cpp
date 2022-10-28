#include <dlfcn.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <chrono>
#include <vector>
#include <algorithm>

#include "./game_keycodes.hpp"

#define REFRESH_FPS 60
#define GAME_FPS 20
#define MAP_WIDTH 60
#define MAP_HEIGHT 40

typedef int (*init_nibbler_t)(int width, int height, int cell_size, const char *name);
typedef int (*get_pressed_keys_t)(int **keys, int *size);
typedef void (*clear_screen_t)(void);
typedef void (*set_square_color_t)(int x, int y, int r, int g, int b, int a);
typedef void (*render_t)(void);

void *loadDynamicSymbol(void *handle, const char *symbol)
{
	void *func = dlsym(handle, symbol);
	if (func == NULL)
	{
		std::cout << "Error while loading symbol " << symbol << " : " << dlerror() << std::endl;
		exit(1);
	}
	return func;
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
	food.first = rand() % MAP_WIDTH;
	food.second = rand() % MAP_HEIGHT;

	// No space left for food
	if (snakeBody.size() == MAP_WIDTH * MAP_HEIGHT)
		return food;

	for (std::pair<int, int> &bodyPart : snakeBody)
	{
		if (bodyPart.first == food.first && bodyPart.second == food.second)
			return generateFood(food, snakeBody);
	}
	return food;
}

int main()
{
	void *handle = dlopen("./sfml/libsfml.so", RTLD_LAZY);
	if (!handle)
	{
		std::cerr << "dlopen failed: " << dlerror() << std::endl;
		exit(EXIT_FAILURE);
	}

	std::cout << "Successfully opened library libsfml.so" << std::endl;

	init_nibbler_t init_nibbler = (init_nibbler_t) loadDynamicSymbol(handle, "init_nibbler");
	get_pressed_keys_t get_pressed_keys = (get_pressed_keys_t) loadDynamicSymbol(handle, "get_pressed_keys");
	clear_screen_t clear_screen = (clear_screen_t) loadDynamicSymbol(handle, "clear_screen");
	set_square_color_t set_square_color = (set_square_color_t) loadDynamicSymbol(handle, "set_square_color");
	render_t render = (render_t) loadDynamicSymbol(handle, "render");

	init_nibbler(MAP_WIDTH, MAP_HEIGHT, 10, "Nibbler");
	std::vector<int> alreadyPressedKeys;


	int currentFrameInSecond = 0;
	Snake snake(MAP_WIDTH / 2 - 2, MAP_WIDTH / 2 + 2, MAP_HEIGHT / 2);
	auto snakeBody = snake.getBody();
	std::pair<int, int> food = generateFood(food, snakeBody);

	int vy = 0;
	int vx = 1;

	while (1)
	{
		auto frameStartTime = std::chrono::high_resolution_clock::now();

		std::vector<int> pressedKeys;
		int *keys = NULL;
		int size = 0;
		get_pressed_keys(&keys, &size);
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
					exit(0);
					break;

				case UP_KEY:
					// Still check so that if snake goes LEFT, user presses UP then RIGHT, it will go right instead of nothing
					if (snake.getVelY() != 1)
					{
						vy = -1;
						vx = 0;
					}
					std::cout << "Go up\n";
					break;

				case DOWN_KEY:
					if (snake.getVelY() != -1)
					{
						vy = 1;
						vx = 0;
					}
					std::cout << "Go down\n";
					break;

				case LEFT_KEY:
					if (snake.getVelX() != 1)
					{
						vy = 0;
						vx = -1;
					}
					std::cout << "Go left\n";
					break;

				case RIGHT_KEY:
					if (snake.getVelX() != -1)
					{
						vy = 0;
						vx = 1;
					}
					std::cout << "Go right\n";
					break;

				case ONE_KEY:
					std::cout << "Switch to lib #1\n";
					break;

				case TWO_KEY:
					std::cout << "Switch to lib #2\n";
					break;

				case THREE_KEY:
					std::cout << "Switch to lib #3\n";
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

		currentFrameInSecond++;
		if (currentFrameInSecond == REFRESH_FPS)
			currentFrameInSecond = 0;

		if (currentFrameInSecond % (REFRESH_FPS / GAME_FPS) == 0)
		{
			// TODO gameTick function
			std::cout << "Game tick" << std::endl;
			clear_screen();

			// us temp variable vx / vy in case the user moves very fast eg. UP, RIGHT (while going left) between
			// game ticks they cannot make a 180° turn
			snake.setDirection(vx, vy);

			snake.move();

			if (snake.getHeadX() >= MAP_WIDTH || snake.getHeadX() < 0 || snake.getHeadY() >= MAP_HEIGHT || snake.getHeadY() < 0 \
				|| snake.isSelfColliding())
			{
				std::cout << "Game over" << std::endl;
				// see https://github.com/microsoft/wslg/issues/445 for segfault, always do LIBGL_ALWAYS_SOFTWARE=1  ./nibbler  on WSL
				exit(0);
			}

			if (snake.getHeadX() == food.first && snake.getHeadY() == food.second)
			{
				snake.grow();
				food = generateFood(food, snakeBody);
			}

			snakeBody = snake.getBody();
			for (std::pair<int, int> &bodyPart : snakeBody)
			{
				set_square_color(bodyPart.first, bodyPart.second, 0, 255, 0, 255);
			}
			set_square_color(food.first, food.second, 255, 0, 0, 255);

			render();
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

	dlclose(handle);
	return 1;
}