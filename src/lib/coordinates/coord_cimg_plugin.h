#include "coord.hpp"

T& operator()(const coordinates::coord<short>& c) {
    return _data[c.x + c.y*_width];
}

const T& operator()(const coordinates::coord<short>& c) const {
    return _data[c.x + c.y*_width];
}