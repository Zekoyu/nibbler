#include <dlfcn.h>
#include <iostream>
#include <stdlib.h>


int main()
{
	void *handle = dlopen("./sfml/libsfml.so", RTLD_LAZY);
	if (!handle)
	{
		std::cerr << "dlopen failed: " << dlerror() << std::endl;
		exit(EXIT_FAILURE);
	}

	std::cout << "Successfully opened library libsfml.so" << std::endl;

	// dlsym
	int (*init_nibbler)(int, int, int, const char *);
	*(void **)(&init_nibbler) = dlsym(handle, "init_nibbler");

	if (!init_nibbler)
	{
		std::cerr << "dlsym failed: " << dlerror() << std::endl;
		exit(EXIT_FAILURE);
	}

	init_nibbler(10, 10, 10, "test");

	dlclose(handle);
	return 1;
}