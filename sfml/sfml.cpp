#include <SFML/Graphics.hpp>
#include <iostream>
#include <stdio.h>

#include "../game_keycodes.hpp"


class Game
{
	private:
		sf::RenderWindow window;
		int width, height;

	public:
		Game(int width, int height, int squareSizePx, const std::string& windowTitle)
			: width(width), height(height)
		{
			this->width = width;
			this->height = height;
			window.create(sf::VideoMode(width * squareSizePx, height * squareSizePx), windowTitle);
			window.setVerticalSyncEnabled(true);

			// center window on screen
			sf::VideoMode screenDimensionsMode = sf::VideoMode::getDesktopMode();
			sf::Vector2i windowPosition = sf::Vector2i(
				(screenDimensionsMode.width - window.getSize().x) / 2,
				(screenDimensionsMode.height - window.getSize().y) / 2
			);
			window.setPosition(windowPosition);

		}

		void setSquareSizePx(int px)
		{
			window.setSize(sf::Vector2u(width * px, height * px));
		}

		void run()
		{
			sf::CircleShape shape(100.f);
			shape.setFillColor(sf::Color::Green);
			shape.setPosition(10, 10);
			sf::Clock clock;

			while (window.isOpen())
			{
				clock.restart();
				sf::Event event;
				while (window.pollEvent(event))
				{
					if (event.type == sf::Event::KeyPressed)
					{
						// if (event.key.code == sf::Keyboard::Escape)
						// {
						// 	std::cout << "Escape key pressed" << std::endl;
						// 	window.close();
						// }
					}
					if (event.type == sf::Event::Closed)
						window.close();
				}

				window.clear();
				window.draw(shape);
				window.display();
				// std::cout << "Render time: " << clock.getElapsedTime().asMilliseconds() << "ms" << std::endl;
			}
		}

		sf::RenderWindow& getWindow()
		{
			return window;
		}
};

Game *game = NULL;


// https://stackoverflow.com/questions/20737987/extern-c-when-exactly-to-use
#ifdef __cplusplus
extern "C" {
#endif
	int init_nibbler(int width, int height, int cell_size, const char *name)
	{
		if (game != NULL)
			return -1;

		game = new Game(width, height, cell_size, name);

		return 0;
	}

	int get_pressed_keys(int **keys, int *size)
	{
		if (game == NULL)
			return -1;

		// At most 5 keys (esc, up, down, left, right)
		// Just malloc 5 and set size to 5 (called must check NONE_KEY) for simplicity
		int max_keys_count = 8;
		*keys = (int *)calloc(sizeof(int), max_keys_count);
		*size = max_keys_count;

		sf::RenderWindow &window = game->getWindow();
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				(*keys)[0] = EXIT_KEY;
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
			(*keys)[0] = EXIT_KEY;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W))
			(*keys)[1] = UP_KEY;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S))
			(*keys)[2] = DOWN_KEY;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A))
			(*keys)[3] = LEFT_KEY;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D))
			(*keys)[4] = RIGHT_KEY;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1) || sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad1))
			(*keys)[5] = ONE_KEY;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2) || sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad2))
			(*keys)[6] = TWO_KEY;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3) || sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad3))
			(*keys)[7] = THREE_KEY;

		return 0;
	}


#ifdef __cplusplus
}
#endif