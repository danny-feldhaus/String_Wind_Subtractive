//A struct for 2D coordinates, with CImg integration.

#ifndef COORD_H 
#define COORD_H
#define cimg_plugin "../coordinates/coord_cimg_plugin.h"
#include <math.h>
using std::sqrt;
using std::pow;
namespace coordinates
{
    template <class T = short>
    struct coord
    {
        T x;
        T y;
        //Constructor
        coord(T x = 0, T y = 0) : x(x), y(y){}
        coord operator-(const coord& other) const
        {
            return {(T)(x-other.x),(T)(y-other.y)};
        }
        coord operator+(const coord& other) const
        {
            return {(T)(x+other.x),(T)(y+other.y)};
        }
        coord operator/(const float div) const
        {
            return {(T)(x/div),(T)(y/div)};
        }
        coord& operator-=(const coord& other)
        {
            x -= other.x;
            y -= other.y;
            return *this;
        }
        coord& operator+=(const coord& other)
        {
            x += other.x;
            y += other.y;
            return *this;
        }
        
        bool operator==(const coord& other) const
        {
            return((x == other.x )&& (y == other.y));
        }
        bool operator!=(const coord& other) const
        {
            return((x != other.x) || (y != other.y));
        }

        operator coord<int>() const 
        {
            return coord<int>((int)x,(int)y);
        }

        operator coord<float>() const
        {
            return coord<float>((float)x,(float)y);
        }

        operator coord<short>() const
        {
            return coord<short>((short)x,(short)y);
        }
    };
    template <class T = short>
    static double distance(const coord<T>& a,const coord<T>& b) 
    {
        return sqrt(pow(b.x-a.x,2) + pow(b.y-a.y,2));
    }

    template <class T = short>
    static double dot(const coord<T>& a, const coord<T>& b)
    {
        return a.x * b.x + a.y * b.y;
    }
}


#endif