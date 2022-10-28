#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>

#include "../game_keycodes.hpp"
#include "../game_functions.hpp"
#include "../Grid.hpp"

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
	int init_nibbler(int width, int height, int cell_size, const char *window_name)
	{
		if (game != NULL)
			return -1;

		std::cout << "Initializing Nibbler" << std::endl;

		game = new Game(width, height, cell_size, window_name);

		// sf::SoundBuffer buffer;
		// if (buffer.loadFromFile("./test.wav"))
		// {
		// 	std::cout << "Loaded sound" << std::endl;
		// 	sf::Sound sound;
		// 	sound.setBuffer(buffer);
		// 	std::cout << "Sound duration: " << sound.getPlayingOffset().asSeconds() << std::endl;
		// 	sound.play();
		// }
		// else
		// {
		// 	std::cout << "Failed to load sound" << std::endl;
		// }

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

	void set_square_color(int x, int y, int r, int g, int b)
	{
		if (game == NULL)
			return;

		game->setSquareColor(x, y, sf::Color(r, g, b));
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

	void show_game_over()
	{
		if (game == NULL)
			return;

		sf::RenderWindow &window = game->getWindow();
		sf::Font font;
		if (font.loadFromFile("./ComicNeueSansID.ttf"))
		{
			sf::Text text;
			sf::Text subtext;
			text.setFont(font);
			text.setString("Game Over");
			text.setCharacterSize(42);
			// Ugly pink
			text.setFillColor(sf::Color(255, 0, 255));
			text.setStyle(sf::Text::Bold);

			subtext.setFont(font);
			subtext.setString("Press any movement key to retry");
			subtext.setCharacterSize(24);
			subtext.setFillColor(sf::Color(255, 0, 255));

			// center on screen
			sf::FloatRect textRect = text.getLocalBounds();
			text.setOrigin(textRect.left + textRect.width / 2.0f,
						   textRect.top + textRect.height / 2.0f);
			text.setPosition(sf::Vector2f(window.getSize().x / 2.0f, window.getSize().y / 2.0f));

			sf::FloatRect subtextRect = subtext.getLocalBounds();
			subtext.setOrigin(subtextRect.left + subtextRect.width / 2.0f,
							  subtextRect.top + subtextRect.height / 2.0f);

			subtext.setPosition(sf::Vector2f(window.getSize().x / 2.0f, window.getSize().y / 2.0f + textRect.height));

			window.draw(text);
			window.draw(subtext);
			window.display();
		}
	}


#ifdef __cplusplus
}
#endif