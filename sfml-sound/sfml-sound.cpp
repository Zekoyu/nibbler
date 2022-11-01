#include <SFML/Audio.hpp>
#include <iostream>
#include <unistd.h>

sf::Sound sound;
sf::SoundBuffer buffer;

#ifdef __cplusplus
extern "C" {
#endif
	int play_sound()
	{
		sound.play();
		return 0;
	}

	int load_sound(const char *path)
	{
		if (!buffer.loadFromFile(path))
			return -1;

		sound.setBuffer(buffer);
		return 0;
	}

#ifdef __cplusplus
}
#endif