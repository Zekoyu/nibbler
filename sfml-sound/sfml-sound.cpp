#include <SFML/Audio.hpp>

sf::Sound sound;
sf::SoundBuffer soundBuffer;

sf::Music music;


#ifdef __cplusplus
extern "C" {
#endif
	int play_sound()
	{
		float randBetweenZeroAndOne = (float) rand() / (float) RAND_MAX;
		sound.setPitch(1.0f + (randBetweenZeroAndOne - 0.5f));
		sound.play();
		return 0;
	}

	int load_sound(const char *path)
	{
		if (!soundBuffer.loadFromFile(path))
			return -1;

		srand(time(NULL));

		sound.setBuffer(soundBuffer);
		return 0;
	}

	int play_music(const char *path, float volume)
	{
		if (!music.openFromFile(path))
			return -1;

		music.setLoop(true);
		music.setVolume(volume);
		music.play();
		return 0;
	}

	void stop_music()
	{
		music.stop();
	}

#ifdef __cplusplus
}
#endif