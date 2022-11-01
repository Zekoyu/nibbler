#include <SFML/Graphics.hpp>
#include <iostream>
#include <unistd.h>

#include "../game_keycodes.hpp"
#include "../game_functions.hpp"
#include "../IGame.hpp"

class Game : public IGame<sf::RectangleShape>
{
	private:
		sf::RenderWindow _window;

	public:
		Game(int width, int height, int squareSizePx, const std::string &windowTitle)
			: IGame(width, height, squareSizePx)
		{
			_window.create(sf::VideoMode(width * squareSizePx, height * squareSizePx), windowTitle);
			_window.setVerticalSyncEnabled(true);
			for(int y = 0; y < _grid.getHeight(); y++)
			{
				for(int x = 0; x < _grid.getWidth(); x++)
				{
					_grid(x, y).setSize(sf::Vector2f(squareSizePx, squareSizePx));
					_grid(x, y).setPosition(x * squareSizePx, y * squareSizePx);
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

		// https://stackoverflow.com/questions/308276/can-i-call-a-constructor-from-another-constructor-do-constructor-chaining-in-c
		Game() : Game(4, 4, 10, "Hmmmmmm")
		{
		}

		~Game()
		{
		}

		Game(const Game &other)
			: IGame(other)
		{
		}

		Game &operator=(const Game &other)
		{
			IGame::operator=(other);
			return *this;
		}

		void setSquareSizePx(int px) override
		{
			_squareSizePx = px;
			_window.setSize(sf::Vector2u(_width * px, _height * px));
			for (int y = 0; y < _grid.getHeight(); y++)
				for (int x = 0; x < _grid.getWidth(); x++)
					_grid(x, y).setSize(sf::Vector2f(px, px));
		}

		void setSquareColor(int x, int y, int r, int g, int b) override
		{
			_grid(x, y).setFillColor(sf::Color(r, g, b));
		}

		void setSquaresBlack() override
		{
			for (int y = 0; y < _grid.getHeight(); y++)
				for (int x = 0; x < _grid.getWidth(); x++)
					setSquareColor(x, y, 0, 0, 0);
		}

		void drawGrid() override
		{
			for (int y = 0; y < _grid.getHeight(); y++)
			{
				for (int x = 0; x < _grid.getWidth(); x++)
				{
					sf::RectangleShape& square = _grid(x, y);
					if (square.getFillColor() != sf::Color::Black && square.getFillColor().a != 0)
					{
						sf::Color save = square.getFillColor();
						sf::Color grayscale = square.getFillColor();
						sf::Uint8 col = (grayscale.r + grayscale.g + grayscale.b) / 3;
						grayscale.r = grayscale.g = grayscale.b = col;
						square.setFillColor(grayscale);
						_window.draw(square);
						square.setFillColor(save);
					}
				}
			}
		}

		void render() override
		{
			_window.clear();
			drawGrid();
			_window.display();
		}

		void showGameOver() override
		{
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
				subtext.setString("Press the space key to retry");
				subtext.setCharacterSize(24);
				subtext.setFillColor(sf::Color(255, 0, 255));

				// center on screen
				sf::FloatRect textRect = text.getLocalBounds();
				text.setOrigin(textRect.left + textRect.width / 2.0f,
							textRect.top + textRect.height / 2.0f);
				text.setPosition(sf::Vector2f(_window.getSize().x / 2.0f, _window.getSize().y / 2.0f));

				sf::FloatRect subtextRect = subtext.getLocalBounds();
				subtext.setOrigin(subtextRect.left + subtextRect.width / 2.0f,
								subtextRect.top + subtextRect.height / 2.0f);

				subtext.setPosition(sf::Vector2f(_window.getSize().x / 2.0f, _window.getSize().y / 2.0f + textRect.height));

				_window.draw(text);
				_window.draw(subtext);
				_window.display();
			}
		}

		void close() override
		{
			_window.close();
		}

		bool pollEvent(sf::Event &event)
		{
			return _window.pollEvent(event);
		}

		bool isKeyPressed(sf::Keyboard::Key key)
		{
			return sf::Keyboard::isKeyPressed(key);
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

		std::cout << "Successfully initialized Nibbler" << std::endl;

		return 0;
	}

	int get_pressed_keys(int **keys, int *size)
	{
		if (game == NULL)
			return -1;

		// At most 5 keys (esc, up, down, left, right)
		// Just malloc 5 and set size to 5 (called must check NONE_KEY) for simplicity
		int max_keys_count = 9;
		*keys = (int *)calloc(sizeof(int), max_keys_count);
		*size = max_keys_count;

		sf::Event event;
		while (game->pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				(*keys)[0] = EXIT_KEY;
		}

		if (game->isKeyPressed(sf::Keyboard::Escape))
			(*keys)[0] = EXIT_KEY;

		if (game->isKeyPressed(sf::Keyboard::Up) || game->isKeyPressed(sf::Keyboard::W))
			(*keys)[1] = UP_KEY;

		if (game->isKeyPressed(sf::Keyboard::Down) || game->isKeyPressed(sf::Keyboard::S))
			(*keys)[2] = DOWN_KEY;

		if (game->isKeyPressed(sf::Keyboard::Left) || game->isKeyPressed(sf::Keyboard::A))
			(*keys)[3] = LEFT_KEY;

		if (game->isKeyPressed(sf::Keyboard::Right) || game->isKeyPressed(sf::Keyboard::D))
			(*keys)[4] = RIGHT_KEY;

		if (game->isKeyPressed(sf::Keyboard::Num1) || game->isKeyPressed(sf::Keyboard::Numpad1))
			(*keys)[5] = ONE_KEY;

		if (game->isKeyPressed(sf::Keyboard::Num2) || game->isKeyPressed(sf::Keyboard::Numpad2))
			(*keys)[6] = TWO_KEY;

		if (game->isKeyPressed(sf::Keyboard::Num3) || game->isKeyPressed(sf::Keyboard::Numpad3))
			(*keys)[7] = THREE_KEY;

		if (game->isKeyPressed(sf::Keyboard::Space))
			(*keys)[8] = SPACE_KEY;

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

		game->setSquareColor(x, y, r, g, b);
	}

	void render()
	{
		if (game == NULL)
			return;

		game->render();
	}

	void show_game_over()
	{
		if (game == NULL)
			return;

		game->showGameOver();
	}

	void exit_nibbler()
	{
		if (game == NULL)
			return;

		game->close();
		delete game;
		game = NULL;
	}


#ifdef __cplusplus
}
#endif