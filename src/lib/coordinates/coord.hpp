/**
 * @file coord.hpp
 * @author Danny Feldhaus
 * @brief A basic 2D coordinate object
 * @version 0.1
 * @date 2022-10-07
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef COORD_H 
#define COORD_H
#define cimg_plugin "../coordinates/coord_cimg_plugin.h"
#include <math.h>
#include <iostream>
using std::sqrt;
using std::pow;


namespace coordinates
{
    /**
     * @brief Container for a 2D coordiante
     * 
     * @tparam T the type of x and y
     */
    template <class T = short>
    struct coord
    {
        T x; 
        T y;
        /**
         * @brief Construct a new coord object
         * @param x 
         * @param y 
         */
        coord(T x = 0, T y = 0) : x(x), y(y){}

        /**
         * @brief Subtraction operator
         * @details Similar to operator-=(const coord& other), except that it returns a new coord instance.
         * @param other Coord object to subtract
         * @return coord A new instance of the result
         */
        coord operator-(const coord& other) const
        {
            return {(T)(x-other.x),(T)(y-other.y)};
        }

        /**
         * @brief Addition operator
         * @details Similar to operator+=(const coord& other), except that it returns a new coord instance.
         * @param other Coord object to add
         * @return coord A new instance of the result
         */
        coord operator+(const coord& other) const
        {
            return {(T)(x+other.x),(T)(y+other.y)};
        }

        /**
         * @brief Division operator
         * @details Similar to operator/=(const coord& other), except that it returns a new coord instance.
         * @param div Value to divide by
         * @return coord New instance of the result
         */
        coord operator/(const float div) const
        {
            return {(T)(x/div),(T)(y/div)};
        }
        /**
         * @brief Multiplication operator
         * @details Similar to operator*=(const coord& other), except that it returns a new coord instance.
         * @param mult Value to multiply by
         * @return coord New instance of the result
         */
        coord operator*(const float mult) const
        {
            return {(T)(x*mult),(T)(y*mult)};
        }
        /**
         * @brief In-place subtraction operator
         * @details Subtracts the coordinates of other from the local coordinates
         * @param other Coord object to subtract
         * @return coord& Reference to this
         */
        coord& operator-=(const coord& other)
        {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        /**
         * @brief In-place addition operator
         * @details Adds the coordinates of other to the local coordinates
         * @param other coord object to add
         * @return coord& Reference to this
         */
        coord& operator+=(const coord& other)
        {
            x += other.x;
            y += other.y;
            return *this;
        }
        
        /**
         * @brief Test if two coords share the same parameters
         * @param other coord object to compare
         * @return bool Result of comparison
         */
        bool operator==(const coord& other) const
        {
            return((x == other.x )&& (y == other.y));
        }

        /**
         * @brief Test that two coords are not equal.
         * @details Similar to operator==(const coord& other), except that the return value is opposite.
         * @param other coord object to compare
         * @return result of comparison
         */
        bool operator!=(const coord& other) const
        {
            return((x != other.x) || (y != other.y));
        }


        /**
         * @brief Implicit conversion to a new instance of type <int>
         * @warning May result in a loss of precision
         * @return coord<int> Converted result
         */
        operator coord<int>() const 
        {
            return coord<int>((int)x,(int)y);
        }

        /**
         * @brief Implicit conversion to a new instance of type <float>
         * @warning May result in a loss of precision
         * @return coord<float> Converted result
         */
        operator coord<float>() const
        {
            return coord<float>((float)x,(float)y);
        }

        /**
         * @brief Implicit conversion to a new instance of type <short>
         * @warning May result in a loss of precision
         * @return coord<short> Converted result
         */
        operator coord<short>() const
        {
            return coord<short>((short)x,(short)y);
        }
        template <typename U>
        friend std::ostream& operator<<(std::ostream& os, const coord<U>& c); 
    };

    /**
     * @brief Calculate length of the line between two coords.
     * 
     * @tparam T Coordinate type
     * @param a First coordinate
     * @param b Second coordinate
     * @return double length
     */
    template <class T = short>
    static double distance(const coord<T>& a,const coord<T>& b) 
    {
        return sqrt(pow(b.x-a.x,2) + pow(b.y-a.y,2));
    }

    /**
     * @brief Calculate the dot product of two coords.
     * @tparam T Coordinate type
     * @param a First coordinate
     * @param b Second coordinate
     * @return double dot product
     */
    template <class T = short>
    static double dot(const coord<T>& a, const coord<T>& b)
    {
        return a.x * b.x + a.y * b.y;
    }

    template <typename T>
    std::ostream& operator<<(std::ostream& os, const coord<T>& c)
    {
        os << '(' << c.x << ',' << c.y << ')';
        return os;
    }

}


#endif