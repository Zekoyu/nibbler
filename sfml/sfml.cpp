#include <SFML/Graphics.hpp>
#include <iostream>
#include <stdio.h>


// https://stackoverflow.com/questions/20737987/extern-c-when-exactly-to-use
#if defined(__cplusplus)
# define EXTERN_C extern "C"
#else
# define EXTERN_C
#endif

EXTERN_C
{
	int init_nibbler(int width, int height, int cell_size, const char *name)
	{
		(void) width;
		(void) height;
		(void) cell_size;
		(void) name;

		printf("Hello from init_nibbler\n");
		return 1;
	}
}

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
			window.setFramerateLimit(60);
		}

		void setSquareSizePx(int px)
		{
			window.setSize(sf::Vector2u(width * px, height * px));
		}

		// void run()
		// {
		// 	while(window.isOpen())
		// 	{
		// 		sf::Event event;
		// 		while(window.pollEvent(event))
		// 		{
		// 			if(event.type == sf::Event::Closed)
		// 				window.close();
		// 		}

		// 		window.clear();
		// 		window.display();
		// 	}
		// }

		sf::RenderWindow& getWindow()
		{
			return window;
		}
};


int main()
{
	Game game(20, 20, 32, "Nibbler");

	sf::RenderWindow& window = game.getWindow();

	sf::CircleShape shape(100.f);
	shape.setFillColor(sf::Color::Green);
	shape.setPosition(10, 10);
	sf::Clock clock;
	window.setFramerateLimit(60);
	// resize window to 200x200
	window.setSize(sf::Vector2u(200, 200));

	while (window.isOpen())
	{
		clock.restart();
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		window.clear();
		window.draw(shape);
		window.display();
		// std::cout << "Render time: " << clock.getElapsedTime().asMilliseconds() << "ms" << std::endl;
	}

	return 0;
}