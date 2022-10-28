#include <dlfcn.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <chrono>
#include <vector>
#include <algorithm>

#include "./game_keycodes.hpp"

#define FPS 60

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

	init_nibbler(60, 40, 10, "Nibbler");
	std::vector<int> alreadyPressedKeys;

	int x = 0;
	int y = 0;

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
					std::cout << "Go up\n";
					break;

				case DOWN_KEY:
					std::cout << "Go down\n";
					break;

				case LEFT_KEY:
					std::cout << "Go left\n";
					break;

				case RIGHT_KEY:
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

		(void)clear_screen;
		(void)set_square_color;
		(void)render;
		clear_screen();
		set_square_color(x++, y, 255, 0, 0, 255);
		if (x > 60)
		{
			x = 0;
			y++;
		}
		if (y > 40)
			y = 0;

		render();

		std::chrono::microseconds frameDuration;
		do
		{
			auto now = std::chrono::high_resolution_clock::now();
			frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(now - frameStartTime);
			usleep(500);
		} while(frameDuration.count() < 1000 * 1000 / FPS);

		std::cout << "Frame took " << (frameDuration.count() / 1000) << "ms to run" << std::endl;
	}


	// pthread_join(thread, NULL);

	dlclose(handle);
	return 1;
}