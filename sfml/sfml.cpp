#include <SFML/Graphics.hpp>
#include <iostream>
#include <stdio.h>
#include <SFML/Audio.hpp>

#include "../game_keycodes.hpp"

template <typename T>
class Grid
{
	private:
		std::vector<T> _grid;
		int _width, _height;

	public:
		Grid(int width, int height)
			: _width(width), _height(height)
		{
			_grid = std::vector<T>(width * height);
		}

		T& operator()(int x, int y)
		{
			return _grid[y * _width + x];
		}

		int getWidth() const
		{
			return _width;
		}

		int getHeight() const
		{
			return _height;
		}

		void setWidth(int width)
		{
			_width = width;
			_grid.resize(_width * _height);
		}

		void setHeight(int height)
		{
			_height = height;
			_grid.resize(_width * _height);
		}

		void resize(int width, int height)
		{
			_width = width;
			_height = height;
			_grid.resize(_width * _height);
		}

		void clear()
		{
			_grid.clear();
		}

		void fill(T value)
		{
			std::fill(_grid.begin(), _grid.end(), value);
		}

};

class Game
{
	private:
		sf::RenderWindow _window;
		int _width, _height;
		int _squareSizePx;
		Grid<sf::RectangleShape> _grid;

	public:
		Game(int width, int height, int squareSizePx, const std::string& windowTitle)
			: _width(width), _height(height), _squareSizePx(squareSizePx), _grid(Grid<sf::RectangleShape>(width, height))
		{
			_window.create(sf::VideoMode(width * squareSizePx, height * squareSizePx), windowTitle);
			_window.setVerticalSyncEnabled(true);
			_grid.fill(sf::RectangleShape(sf::Vector2f(squareSizePx, squareSizePx)));
			for (int y = 0; y < _grid.getHeight(); y++)
			{
				for (int x = 0; x < _grid.getWidth(); x++)
				{
					_grid(x, y).setPosition(x * squareSizePx, y * squareSizePx);
					// std::cout << "Square position: " << _grid(x, y).getPosition().x << ", " << _grid(x, y).getPosition().y << std::endl;
				}
			}

			// center window on screen
			sf::VideoMode screenDimensionsMode = sf::VideoMode::getDesktopMode();
			sf::Vector2i windowPosition = sf::Vector2i(
				(screenDimensionsMode.width - _window.getSize().x) / 2,
				(screenDimensionsMode.height - _window.getSize().y) / 2
			);
			_window.setPosition(windowPosition);
		}

		void setSquareSizePx(int px)
		{
			_squareSizePx = px;
			_window.setSize(sf::Vector2u(_width * px, _height * px));
			for (int y = 0; y < _grid.getHeight(); y++)
			{
				for (int x = 0; x < _grid.getWidth(); x++)
				{
					_grid(x, y).setSize(sf::Vector2f(px, px));
				}
			}
		}

		void setSquareColor(int x, int y, sf::Color color)
		{
			_grid(x, y).setFillColor(color);
		}

		void setSquaresBlack()
		{
			for (int y = 0; y < _grid.getHeight(); y++)
			{
				for (int x = 0; x < _grid.getWidth(); x++)
				{
					_grid(x, y).setFillColor(sf::Color::Black);
				}
			}
		}

		// void run()
		// {
		// 	sf::CircleShape shape(100.f);
		// 	shape.setFillColor(sf::Color::Green);
		// 	shape.setPosition(10, 10);
		// 	sf::Clock clock;

		// 	while (window.isOpen())
		// 	{
		// 		clock.restart();
		// 		sf::Event event;
		// 		while (window.pollEvent(event))
		// 		{
		// 			if (event.type == sf::Event::KeyPressed)
		// 			{
		// 				// if (event.key.code == sf::Keyboard::Escape)
		// 				// {
		// 				// 	std::cout << "Escape key pressed" << std::endl;
		// 				// 	window.close();
		// 				// }
		// 			}
		// 			if (event.type == sf::Event::Closed)
		// 				window.close();
		// 		}

		// 		window.clear();
		// 		window.draw(shape);
		// 		window.display();
		// 		// std::cout << "Render time: " << clock.getElapsedTime().asMilliseconds() << "ms" << std::endl;
		// 	}
		// }

		void drawGrid()
		{
			// _window.draw(_grid(0, 0));
			for (int y = 0; y < _grid.getHeight(); y++)
			{
				for (int x = 0; x < _grid.getWidth(); x++)
				{
					sf::RectangleShape& square = _grid(x, y);
					if (square.getFillColor() != sf::Color::Black && square.getFillColor().a != 0)
					{
						_window.draw(square);
					}
				}
			}
		}

		sf::RenderWindow& getWindow()
		{
			return _window;
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

		std::cout << "Initializing Nibbler" << std::endl;

		game = new Game(width, height, cell_size, name);

		sf::SoundBuffer buffer;
		if (buffer.loadFromFile("./test.wav"))
		{
			std::cout << "Loaded sound" << std::endl;
			sf::Sound sound;
			sound.setBuffer(buffer);
			std::cout << "Sound duration: " << sound.getPlayingOffset().asSeconds() << std::endl;
			sound.play();
		}
		else
		{
			std::cout << "Failed to load sound" << std::endl;
		}

		std::cout << "Successfully initialized Nibbler" << std::endl;

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
		{
			(*keys)[5] = ONE_KEY;
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2) || sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad2))
			(*keys)[6] = TWO_KEY;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3) || sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad3))
			(*keys)[7] = THREE_KEY;

		return 0;
	}


	void clear_screen()
	{
		if (game == NULL)
			return;

		game->setSquaresBlack();
	}

	void set_square_color(int x, int y, int r, int g, int b, int a)
	{
		if (game == NULL)
			return;

		game->setSquareColor(x, y, sf::Color(r, g, b, a));
	}

	void render()
	{
		if (game == NULL)
			return;

		sf::RenderWindow &window = game->getWindow();
		window.clear();
		game->drawGrid();
		window.display();
	}


#ifdef __cplusplus
}
#endif