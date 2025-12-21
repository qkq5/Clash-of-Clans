#ifndef __GRID_H__
#define __GRID_H__

#include <vector>
#include <stdexcept>
#include "core/GameException.h"

namespace core {

template <typename T>
class Grid {
public:
    Grid(int width, int height) : _width(width), _height(height) {
        if (width <= 0 || height <= 0) {
            throw GameException("Grid dimensions must be positive");
        }
        _data.resize(width * height);
    }

    T& at(int x, int y) {
        if (x < 0 || x >= _width || y < 0 || y >= _height) {
            throw GameException("Grid index out of bounds");
        }
        return _data[y * _width + x];
    }

    const T& at(int x, int y) const {
        if (x < 0 || x >= _width || y < 0 || y >= _height) {
            throw GameException("Grid index out of bounds");
        }
        return _data[y * _width + x];
    }

    int getWidth() const { return _width; }
    int getHeight() const { return _height; }

    // Iterator support
    typename std::vector<T>::iterator begin() { return _data.begin(); }
    typename std::vector<T>::iterator end() { return _data.end(); }
    typename std::vector<T>::const_iterator begin() const { return _data.begin(); }
    typename std::vector<T>::const_iterator end() const { return _data.end(); }

private:
    int _width;
    int _height;
    std::vector<T> _data;
};

} // namespace core

#endif // __GRID_H__
