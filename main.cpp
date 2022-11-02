#include <dlfcn.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <chrono>
#include <vector>
#include <algorithm>
#include <random>
#include <map>

#include <SFML/Window.hpp>

#include "./game_keycodes.hpp"
#include "./game_functions.hpp"

#define REFRESH_FPS 60 // To handle inputs
#define GAME_FPS 20 // Game ticks per second (should not be lower then REFRESH_FPS)
#define SQUARE_SIZE_PX 10

// bonuses: Fast as fuck mode, multiplayer mode, easy mode, music, background image
enum e_gamemode
{
	GAMEMODE_NORMAL,
	GAMEMODE_FASTASFUCK,
	GAMEMODE_MULTIPLAYER,
	GAMEMODE_MUSICAL,
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
	set_background_image_t set_background_image;

} t_nibbler_dynamic_library;

typedef int (*load_sound_t)(const char *path);
typedef int (*play_sound_t)();
// volume range 0-100
typedef int (*play_music_t)(const char *path, float volume);
typedef int (*stop_music_t)();

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
	lib.set_background_image = (set_background_image_t)loadDynamicSymbol(lib.handle, "set_background_image");
}

class Snake
{
	private:
		std::vector<std::pair<int, int>> _body;
		int _vx;
		int _vy;

	public:
		Snake(): _body(), _vx(0), _vy(0)
		{ }

		// xStart = tail xEnd = head position (if == means 1 block)
		Snake(int xStart, int xEnd, int y)
		{
			_vx = xStart < xEnd ? 1 : -1;
			_vy = 0;
			if (xStart < xEnd)
			{
				for (int x = xStart; x <= xEnd; x++)
					_body.insert(_body.begin(), std::make_pair(x, y));
			}
			else
			{
				for (int x = xStart; x >= xEnd; x--)
					_body.insert(_body.begin(), std::make_pair(x, y));
			}
		}

		void grow()
		{
			_body.push_back(std::make_pair(_body.back().first, _body.back().second));
		}

		void move()
		{
			if (_body.size() < 1)
				return;

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
			if (_body.size() < 1)
				return -1;

			return _body[0].first;
		}

		int getHeadY() const
		{
			if (_body.size() < 1)
				return -1;

			return _body[0].second;
		}

		int getTailX() const
		{
			if (_body.size() < 1)
				return -1;

			return _body[_body.size() - 1].first;
		}

		int getTailY() const
		{
			if (_body.size() < 1)
				return -1;

			return _body[_body.size() - 1].second;
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
			// if (vx != -_vx)
				_vx = vx;

			// if (vy != -_vy)
				_vy = vy;
		}

		bool isSelfColliding() const
		{
			if (_body.size() < 1)
				return false;

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

		Snake &operator=(const Snake &other)
		{
			this->_body = other._body;
			this->_vx = other._vx;
			this->_vy = other._vy;
			return *this;
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
		if (bodyPart.first % g_mapWidth == food.first && bodyPart.second % g_mapHeight == food.second)
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
	std::cout << "Usage: " << executableName << " <Map Width> <Map Height> <Mode> -b <Background Image>" << std::endl;
	std::cout << "Mode can be 'classic', 'faf' (fast as fuck), 'easy' (or musical) or 'multiplayer'" << std::endl;
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	if (argc < 3)
		usage(argv[0]);

	int screenWidth = sf::VideoMode::getDesktopMode().width;
	int screenHeight = sf::VideoMode::getDesktopMode().height;

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

	int gamemode = GAMEMODE_NORMAL;
	char *backgroundImage = NULL;

	if (argc >= 4)
	{
		for (int i = 3; i < argc; i++)
		{
			std::string mode = argv[i];
			if (mode == "normal")
				gamemode = GAMEMODE_NORMAL;
			else if (mode == "faf" || mode == "fastasfuck")
				gamemode = GAMEMODE_FASTASFUCK;
			else if (mode == "multiplayer" || mode == "battle" || mode == "versus")
				gamemode = GAMEMODE_MULTIPLAYER;
			else if (mode == "music" || mode == "musical" || mode == "easy" || mode == "peaceful")
				gamemode = GAMEMODE_MUSICAL;
			else if (mode == "-b" && i + 1 < argc)
			{
				backgroundImage = argv[i + 1];
				i++;
				continue;
			}
			else
			{
				std::cerr << "Wrong argument: '" << mode << "'\n";
				usage(argv[0]);
			}
		}
	}

	if (mapWidth * SQUARE_SIZE_PX > screenWidth || mapHeight * SQUARE_SIZE_PX > screenHeight)
	{
		std::cerr << "Map is too big\n";
		usage(argv[0]);
	}

	if (mapWidth < 6 || mapHeight < 4 || (gamemode == GAMEMODE_MULTIPLAYER && mapWidth < 10))
	{
		std::cerr << "Map is too small\n";
		usage(argv[0]);
	}

	g_mapWidth = (unsigned int) mapWidth;
	g_mapHeight = (unsigned int) mapHeight;

	int gameFps = GAME_FPS;


	std::vector<std::pair<const char *, const char *>> libs = { {"./sdl/libnibbler_sdl.so", "SDL2"},
									  							{"./sfml/libnibbler_sfml.so", "SFML"},
																{"./raylib/libnibbler_raylib.so", "Raylib"} };

	std::vector<std::string> gameOverSounds = { "./sounds/game_over1.wav", "./sounds/game_over2.wav", "./sounds/game_over3.wav" };

	const char *soundLibPath = "./sfml-sound/libnibbler_sfml-sound.so";
	void *soundLibHandle = dlOpenOrExit(soundLibPath);

	load_sound_t load_sound = (load_sound_t)loadDynamicSymbol(soundLibHandle, "load_sound");
	play_sound_t play_sound = (play_sound_t)loadDynamicSymbol(soundLibHandle, "play_sound");
	play_music_t play_music = (play_music_t)loadDynamicSymbol(soundLibHandle, "play_music");
	stop_music_t stop_music = (stop_music_t)loadDynamicSymbol(soundLibHandle, "stop_music");

	(void) stop_music;

	if (load_sound("./sounds/grow.wav") != 0)
		std::cerr << "Cannot open sound grow.wav" << std::endl;

	if (play_music("./sounds/music.wav", 100) != 0)
		std::cerr << "Cannot play music music.wav" << std::endl;

	t_nibbler_dynamic_library lib;
	srand(time(NULL));

	int libIndex = rand() % libs.size();

	openNibblerDynamicLibraryOrExit(libs[libIndex].first, lib);
	lib.init_nibbler(g_mapWidth, g_mapHeight, SQUARE_SIZE_PX, (std::string("Nibbler - ") + libs[libIndex].second).c_str());

	if (backgroundImage)
	{
		std::cout << "Setting background image to " << backgroundImage << std::endl;
		lib.set_background_image(backgroundImage);
	}

	std::vector<int> alreadyPressedKeys;
	int currentFrame = 0;

	Snake snake;
	Snake snake2;

	if (gamemode != GAMEMODE_MULTIPLAYER)
	{
		snake = Snake(g_mapWidth / 2 - 2, g_mapWidth / 2 + 2, g_mapHeight / 2);
	}
	else
	{
		snake = Snake(g_mapWidth / 2, g_mapWidth / 2 + 4,g_mapHeight / 2);
		snake2 = Snake(g_mapWidth / 2, g_mapWidth / 2 - 4, g_mapHeight / 2);
	}

	auto snakeBody = snake.getBody();
	if (gamemode == GAMEMODE_MULTIPLAYER)
	{
		auto snakeBody2 = snake2.getBody();
		snakeBody.insert(snakeBody.end(), snakeBody2.begin(), snakeBody2.end());
	}
	std::pair<int, int> food = generateFood(food, snakeBody);

	int vy = 0;
	int vx = 1;
	int vy2 = 0;
	int vx2 = -1;
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

				case W_KEY:
					if (gamemode == GAMEMODE_MULTIPLAYER)
					{
						if (snake2.getVelY() != 1)
						{
							vy2 = -1;
							vx2 = 0;
						}
						break;
					}
				case UP_KEY:
					// Still check so that if snake goes LEFT, user presses UP then RIGHT, it will go right instead of nothing
					if (snake.getVelY() != 1)
					{
						vy = -1;
						vx = 0;
					}
					// std::cout << "Go up\n";
					break;


				case S_KEY:
					if (gamemode == GAMEMODE_MULTIPLAYER)
					{
						if (snake2.getVelY() != -1)
						{
							vy2 = 1;
							vx2 = 0;
						}
						break;
					}
				case DOWN_KEY:
					if (snake.getVelY() != -1)
					{
						vy = 1;
						vx = 0;
					}
					// std::cout << "Go down\n";
					break;


				case A_KEY:
					if (gamemode == GAMEMODE_MULTIPLAYER)
					{
						if (snake2.getVelX() != 1)
						{
							vy2 = 0;
							vx2 = -1;
						}
						break;
					}
				case LEFT_KEY:
					if (snake.getVelX() != 1)
					{
						vy = 0;
						vx = -1;
					}
					// std::cout << "Go left\n";
					break;


				case D_KEY:
					if (gamemode == GAMEMODE_MULTIPLAYER)
					{
						if (snake2.getVelX() != -1)
						{
							vy2 = 0;
							vx2 = 1;
						}
						break;
					}
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

					if (backgroundImage)
						lib.set_background_image(backgroundImage);
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

					if (backgroundImage)
						lib.set_background_image(backgroundImage);
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

					if (backgroundImage)
						lib.set_background_image(backgroundImage);
					goto game_loop;

					break;

				case SPACE_KEY:
					if (gameOver)
					{
						std::cout << "Restart\n";
						play_music("./sounds/music.wav", 100);
						gameOver = false;
						vx = 1;
						vy = 0;
						if (gamemode != GAMEMODE_MULTIPLAYER)
						{
							snake = Snake(g_mapWidth / 2 - 2, g_mapWidth / 2 + 2, g_mapHeight / 2);
						}
						else
						{
							vx2 = -1;
							vy2 = 0;
							snake = Snake(g_mapWidth / 2, g_mapWidth / 2 + 4,g_mapHeight / 2);
							snake2 = Snake(g_mapWidth / 2, g_mapWidth / 2 - 4, g_mapHeight / 2);
						}
						snake.setDirection(1, 0);
						snake2.setDirection(-1, 0);
						snakeBody = snake.getBody();
						if (gamemode == GAMEMODE_MULTIPLAYER)
						{
							auto snakeBody2 = snake2.getBody();
							snakeBody.insert(snakeBody.end(), snakeBody2.begin(), snakeBody2.end());
						}
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
			{
				// stop_music();
				continue;
			}
			// TODO gameTick function
			// std::cout << "Game tick" << std::endl;

			// us temp variable vx / vy in case the user moves very fast eg. UP, RIGHT (while going left) between
			// game ticks they cannot make a 180° turn
			snake.setDirection(vx, vy);

			if (gamemode == GAMEMODE_MUSICAL)
			{
				if (vx < 0 && snake.getHeadX() <= 0)
					vx = 1;
				else if (vx > 0 && snake.getHeadX() >= g_mapWidth - 1)
					vx = -1;
				else if (vy < 0 && snake.getHeadY() <= 0)
					vy = 1;
				else if (vy > 0 && snake.getHeadY() >= g_mapHeight - 1)
					vy = -1;
				snake.setDirection(vx, vy);
			}
			snake.move();

			if (gamemode == GAMEMODE_MULTIPLAYER)
			{
				snake2.setDirection(vx2, vy2);
				snake2.move();
			}

			snakeBody = snake.getBody();
			if (gamemode == GAMEMODE_MULTIPLAYER)
			{
				auto snakeBody2 = snake2.getBody();
				snakeBody.insert(snakeBody.end(), snakeBody2.begin(), snakeBody2.end());
			}

			// std::cout << snake.getHeadX();

			if ((snake.getHeadX() >= g_mapWidth || snake.getHeadX() < 0 || snake.getHeadY() >= g_mapHeight || snake.getHeadY() < 0 \
				|| snake.isSelfColliding()) && gamemode != GAMEMODE_MUSICAL)
			{
				std::cout << "Game over" << std::endl;
				gameOver = true;
				int gameOverSoundIndex = rand() % gameOverSounds.size();;
				play_music(gameOverSounds[gameOverSoundIndex].c_str(), 100.0);
				gameFps = GAME_FPS;
				lib.show_game_over();
				continue;
				// see https://github.com/microsoft/wslg/issues/445 for segfault, always do LIBGL_ALWAYS_SOFTWARE=1  ./nibbler  on WSL
				// exit(0);
			}

			if (gamemode == GAMEMODE_MULTIPLAYER)
			{
				if (snake2.getHeadX() >= g_mapWidth || snake2.getHeadX() < 0 || snake2.getHeadY() >= g_mapHeight || snake2.getHeadY() < 0 \
				|| snake2.isSelfColliding())
				{
					std::cout << "Game over" << std::endl;
					gameOver = true;
					int gameOverSoundIndex = rand() % gameOverSounds.size();;
					play_music(gameOverSounds[gameOverSoundIndex].c_str(), 100.0);
					gameFps = GAME_FPS;
					lib.show_game_over();
					continue;
				}

				auto snake1Body = snake.getBody();
				auto snake2Body = snake2.getBody();
				bool collision = false;
				for (auto &pair1 : snake1Body)
				{
					if (collision)
						break;

					for (auto &pair2 : snake2Body)
					{
						if (pair1.first == pair2.first && pair1.second == pair2.second)
						{
							collision = true;
							break;
						}
					}
				}

				if (collision)
				{
					std::cout << "Game over" << std::endl;
					gameOver = true;
					int gameOverSoundIndex = rand() % gameOverSounds.size();;
					play_music(gameOverSounds[gameOverSoundIndex].c_str(), 100.0);
					gameFps = GAME_FPS;
					lib.show_game_over();
					continue;
				}
			}

			if (snake.getHeadX() == food.first && snake.getHeadY() == food.second)
			{
				if (gamemode == GAMEMODE_FASTASFUCK)
				{
					gameFps++;
					if (gameFps > REFRESH_FPS)
						gameFps = REFRESH_FPS;
				}
				play_sound();
				snake.grow();
				food = generateFood(food, snakeBody);
			}

			if (gamemode == GAMEMODE_MULTIPLAYER && snake2.getHeadX() == food.first && snake2.getHeadY() == food.second)
			{
				play_sound();
				snake2.grow();
				food = generateFood(food, snakeBody);
			}

			lib.clear_screen();

			for (std::pair<int, int> &bodyPart : snakeBody)
			{
				// use modulo in case of peaceful mode just go on the other side of the screen
				lib.set_square_color(bodyPart.first, bodyPart.second, 0, 255, 0);
			}



			lib.set_square_color(snake.getTailX(), snake.getTailY(), 128, 255, 0);
			lib.set_square_color(snake.getHeadX(), snake.getHeadY(), 0, 128, 0);

			if (gamemode == GAMEMODE_MULTIPLAYER)
			{
				lib.set_square_color(snake2.getTailX(), snake2.getTailY(), 128, 255, 0);
				lib.set_square_color(snake2.getHeadX(), snake2.getHeadY(), 0, 128, 0);
			}

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