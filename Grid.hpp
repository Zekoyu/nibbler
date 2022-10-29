#ifndef GRID_HPP
#define GRID_HPP

#include <vector>

template <typename T>
class Grid
{
	private:
		int _width, _height;
		std::vector<T> _grid;

	public:
		Grid(int width, int height)
			: _width(width), _height(height), _grid(std::vector<T>(width * height))
		{
		}

		Grid()
			: _width(0), _height(0), _grid(std::vector<T>())
		{
		}

		Grid(const Grid& other)
			: _width(other._width), _height(other._height), _grid(other._grid)
		{
		}

		~Grid()
		{
		}

		Grid &operator=(const Grid &other)
		{
			_width = other._width;
			_height = other._height;
			_grid = other._grid;
			return *this;
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

#endif