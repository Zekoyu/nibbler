#ifndef IGAME_HPP
#define IGAME_HPP

#include "./Grid.hpp"

template <typename SquareType>
class IGame
{
	protected:
		int _width, _height;
		int _squareSizePx;
		Grid<SquareType> _grid;
		bool _background;

	protected:
		IGame(int width, int height, int squareSizePx)
			: _width(width), _height(height), _squareSizePx(squareSizePx), _grid(Grid<SquareType>(width, height)), _background(false)
		{
		}

		// https://stackoverflow.com/questions/308276/can-i-call-a-constructor-from-another-constructor-do-constructor-chaining-in-c
		IGame() : IGame(4, 4, 10, "Hmmmmmm")
		{
		}

	public:

		virtual ~IGame()
		{
		}

		IGame(const IGame &other)
			: _width(other._width), _height(other._height), _squareSizePx(other._squareSizePx), _grid(other._grid)
		{
		}

		virtual IGame &operator=(const IGame &other)
		{
			_width = other._width;
			_height = other._height;
			_grid = other._grid;
			return *this;
		}

		virtual void setSquareSizePx(int px)
		{
			_squareSizePx = px;
		}

		virtual void setSquareColor(int x, int y, int r, int g, int b) = 0;
		virtual void setSquaresBlack() = 0;
		virtual void drawGrid() = 0;
		virtual void render() = 0;
		virtual void showGameOver() = 0;
		virtual void close() = 0;
		virtual void setBackground(const char *path) = 0;
};

#endif