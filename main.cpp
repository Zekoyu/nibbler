#include <dlfcn.h>
#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <chrono>

#include "./game_keycodes.hpp"

#define FPS 60

typedef int (*init_nibbler_t)(int width, int height, int cell_size, const char *name);
typedef int (*get_pressed_keys_t)(int **keys, int *size);

void *thread_main(void *arg)
{
	std::cout << "Hello from thread" << std::endl;
	init_nibbler_t init_nibbler = (init_nibbler_t) arg;
	init_nibbler(60, 40, 10, "Nibbler");

	return NULL;
}

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

	// run init_nibbler in another thread
	// pthread_t thread;
	// if (pthread_create(&thread, NULL, thread_main, (void *) init_nibbler) != 0)
	// {
	// 	std::cerr << "pthread_create failed" << std::endl;
	// 	exit(EXIT_FAILURE);
	// }

	init_nibbler(60, 40, 10, "Nibbler");

	while (1)
	{
		auto frameStartTime = std::chrono::high_resolution_clock::now();

		int *keys = NULL;
		int size = 0;
		get_pressed_keys(&keys, &size);
		for (int i = 0; i < size; i++)
		{
			if (keys[i] == NONE_KEY)
				continue;

			std::cout << keys[i] << std::endl;
		}
		free(keys);


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