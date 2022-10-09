#ifndef LINE_ITERATOR_H
#define LINE_ITERATOR_H
#include "CImg.h"
#include "coord.hpp"
#include "line.hpp"

#include <iostream>
#include <vector>
#include <math.h>
#include <line.hpp>
using std::vector;
using std::min;
using std::max;
using coordinates::coord;
using coordinates::distance;
using coordinates::dot;
using coordinates::line;

//An iterator for a line between two points. 
//  Uses basic stepped interpolation / floored values for intermediate coordinates
//  Todo: Add bresenham support / thickness
//  Todo: Add weighted values based on covered pixel area
//  Todo: Convert to an iterator sub-class for the line.hpp object
template <typename IMG_TYPE>
class line_iterator
{
    typedef cimg_library::CImg<IMG_TYPE> tcimg;
    typedef coord<short> scoord;
    typedef coord<float> fcoord;
    public:
        //Distance from start to end
        int line_length;
        //Distance from (cur_x, cur_y) to start
        int cur_length = 0;
        //Start and end coordinates of line
        fcoord start, end;

        /**
         * @brief Construct a new line iterator\<IMG TYPE\>::line iterator object from two coord objects
         * 
         * @param _image Image to iterate over
         * @param point_a One end of the line  
         * @param point_b Other end of the line
         * @param _interpolate
         * - true: Pixel values are read / written using floating point coordinates and interpolation.
         * - false: Pixel values are read / written using floored whole-number coordinates.
         */
        line_iterator(tcimg &_image, scoord point_a, scoord point_b, bool _interpolate = false);
        
        /**
         * @brief Construct a new line iterator object
         * 
         * @param _image Image to iterate over
         * @param ax X coordinate of first point
         * @param ay Y coordinate of first point
         * @param bx X coordinate of second point
         * @param by Y coordinate of second point
         * @param _interpolate 
         * - true: Pixel values are read / written using floating point coordinates and interpolation.
         * - false: Pixel values are read / written using floored whole-number coordinates.
         */
        line_iterator(tcimg &_image, short ax, short ay, short bx, short by, bool _interpolate = false);
        
        /**
         * @brief Step one pixel width forward
         * 
         * @return true Step successful
         * @return false Step unsuccessful, at end of line
         */
        bool step();
        
        /**
         * @brief Step one pixel width backward
         * @details Similar to step(), except it moves in the opposite direction.
         * @return true Step successful
         * @return false Step unsuccessful, at beginning of line
         */
        bool step_back();

        /**
         * @brief Shrink line to the given range of x values
         * @details Shrinks to the smallest range that contains min_x and max_x that can be reached with step() and step_back()
         * @param min_x Minimum x value
         * @param max_x Maximum x value
         * @return true Shrink successful
         * @return false Shrink unsuccessful, bounds not in range
         */
        bool shrink_around_x(float min_x, float max_x);
        
        /**
         * @brief  Shrink line to the given range of y values
         * @details Shrinks to the smallest range that contains min_y and max_y that can be reached with step() and step_back()
         * @param min_y Minimum y
         * @param max_y Maximum y
         * @return true Shrink successful
         * @return false Shrink unsuccessful, bounds not in range
         */
        bool shrink_around_y(float min_y, float max_y);



        /**
         * @brief Get image value at current position
         * @details
         * - _interpolation == true: Value is interpolated using floating point coordinate (see CImg documentation on linear_atXY() for more detail)
         * - _interpolation ==fsv false: Value is found using whole-number coordinates.
         * @return IMG_TYPE image value
         */
        IMG_TYPE get() const;
        
        /**
         * @brief Set image value at current position
         * @details
         * - _interpolation == true: Value is set at interpolated position using floating point coordinate (see CImg documentation on set_linear_atXY() for more detail)
         * - _interpolation == false: Value is set using whole-number coordinates.
         * @param val value to set
         * @return IMG_TYPE equal to val
         */
        IMG_TYPE set(IMG_TYPE val);

        /**
         * @brief A new-instance whole-number coord of the current position
        */
        scoord cur_coord();

        /**
         * @brief The current pixel's CImg index
        */
        int idx();


    private:
        cimg_library::CImg<IMG_TYPE> &image;
        bool interpolate;
        fcoord cur_pos;
        fcoord step_size;/**x and y step size*/
};

/**
 * @brief An iterator version of std::set_intersection()
 * @warning Depreciated
 */
class set_intersection_iterator
{
public:
    set_intersection_iterator(vector<int> &_set_a, vector<int> &_set_b, int _max_separation = 1, int max_step_power = 9) : set_a(_set_a), set_b(_set_b)
    {
        max_separation = _max_separation;
        max_step_size = pow(2, max_step_power);
        itr_a = set_a.begin();
        itr_b = set_b.begin();
    }
    bool get_next_match(int &match)
    {
        /*
        while(*itr_a < *itr_b && itr_a != set_a.end()) ++itr_a;
        while(*itr_b < *itr_a && itr_b != set_b.end()) ++itr_b;
        if(itr_a == set_a.end() || itr_b == set_b.end()) return false;
        match = *itr_a;
        ++itr_a;
        ++itr_b;
        return true;
        */
        int step_size = max_step_size;
        int min_val_diff;
        while (step_size)
        {
            min_val_diff = max_separation * step_size;
            while (*itr_b - *itr_a > min_val_diff && set_a.end() - itr_a > step_size)
            {
                itr_a += step_size;
            }
            while (*itr_a - *itr_b > min_val_diff && set_b.end() - itr_b > step_size)
            {
                itr_b += step_size;
            }
            step_size /= 2;
        }
        while (*itr_a != *itr_b && itr_a != set_a.end() && itr_b != set_b.end())
        {
            while (*itr_a < *itr_b && itr_a != set_a.end())
                itr_a++;
            while (*itr_b < *itr_a && itr_b != set_b.end())
                itr_b++;
        }
        if (itr_a == set_a.end() || itr_b == set_b.end())
            return false;
        match = *itr_a;
        itr_a++;
        itr_b++;
        return true;
        
    }

private:
    vector<int> &set_a;
    vector<int> &set_b;
    vector<int>::iterator itr_a;
    vector<int>::iterator itr_b;
    int max_separation;
    int max_step_size;
};

//Iterates over the region of a line that intersects with another line
//Todo: transition from coords to line objects.
template <typename IMG_TYPE>
class line_intersection_iterator
{
    typedef cimg_library::CImg<IMG_TYPE> tcimg;
    typedef coord<short> scoord;
    private:
    tcimg& image;
    scoord a_start;
    scoord a_end;
    scoord b_start;
    scoord b_end;
    scoord center;
    scoord a_diff, b_diff;
    float a_mag, b_mag;
    float angle;
    float overlap_width;
    line_iterator<IMG_TYPE> li;
    int steps_taken = 0;

    //Find the center intersection coordinate of the two lines
    //Source: https://www.topcoder.com/thrive/articles/Geometry%20Concepts%20part%202:%20%20Line%20Intersection%20and%20its%20Applications
    scoord get_intersection()
    {
        float a1 = a_end.y - a_start.y;
        float b1 = a_start.x - a_end.x;
        float c1 = a1*a_start.x + b1*a_start.y;

        float a2 = b_end.y - b_start.y;
        float b2 = b_start.x - b_end.x;
        float c2 = a2*b_start.x + b2*b_start.y;
        
        float det = a1 * b2 - a2 * b1;
        //If the lines are parallel, just return the mid-point of a. It's wrong, but it feels right, you know?
        if(det == 0) return (a_start + a_end)/2;
        return scoord((b2 * c1 - b1 * c2) / det, (a1 * c2 - a2 * c1) / det);
    }

    //Uses some linear algebra to get the angle 
    //Todo: move this to line.hpp
    float get_angle()
    {
        a_diff = a_end - a_start;
        b_diff = b_end - b_start;
        float d = dot(a_diff,b_diff);
        a_mag = distance(a_start, a_end);
        b_mag = distance(b_start, b_end);
        float cosine = d / (a_mag * b_mag);
        return std::acos(cosine);
    }

    //Uses some very cool math to find the width of the intersection (equal to the length of the iterator)
    //Todo: make a readme to explain the math
    float get_overlap_width(float width_buffer)
    {
        float w =(1.0f / sin(angle)) + width_buffer;
        if(w > min(a_mag,b_mag)) w = min(a_mag,b_mag);
        return w + width_buffer;
    }
  
    //Shrinks the line iterator to the overlapping region
    void shrink_iterator()
    {
        //std::cout << "Start / end before: " << li.start.x << ',' << li.start.y << "->" << li.end.x << ',' << li.end.y << "\n";
        if(abs(a_end.x - a_start.x) > abs(a_end.y - a_start.x))
        {
            //std::cout << "\tShrinking x to " << center.x - cos(angle)*overlap_width/2 << "->" << center.x + cos(angle)*overlap_width/2 << '\n';
            li.shrink_around_x(center.x - abs(cos(angle)*overlap_width/2), center.x + abs(cos(angle)*overlap_width/2));
        }
        else
        {
            //std::cout << "\tShrinking y to " << center.y - sin(angle)*overlap_width/2 << "->" << center.y + sin(angle)*overlap_width/2 << '\n';

            li.shrink_around_y(center.y - abs(sin(angle)*overlap_width/2), center.y + abs(sin(angle)*overlap_width/2));
        }
        if(li.cur_coord().x > image.width())
        {
            std::cerr << "ERROR: SHRUNK REGION OUT OF BOUNDS.\n"
                << "\ta start: \t" << a_start.x << ',' << a_start.y << '\n'
                << "\ta end: \t" << a_end.x << ',' << a_end.y << '\n'
                << "\tb start: \t" << b_start.x << ',' << b_start.y << '\n'
                << "\tb end: \t" << b_end.x << ',' << b_end.y << '\n'
                << "\tcenter: \t" << center.x << ',' << center.y << '\n'
                << "\tShrunk start: \t" << li.start.x << ',' << li.start.y << '\n'
                << "\tShrunk end: \t"  <<  li.end.x << ',' << li.end.y << '\n'
                << "\tShrunk around " << ((abs(a_end.x - a_start.x) > abs(a_end.y - a_start.x)) ? 'x' : 'y') << '\n'
                << "\tA diff: " << abs(a_end.x - a_start.x) << ',' << abs(a_end.y - a_start.x);
        }
       // std::cout << "Start / end after: " << li.start.x << ',' << li.start.y << "->" << li.end.x << ',' << li.end.y << "\n";

       // std::cout << "Overlap width: " << overlap_width << ", Line length: " << li.line_length << '\n';

        return;
    }

    public:
    line_intersection_iterator(tcimg &_image, scoord line_a_start, scoord line_a_end, scoord line_b_start, scoord line_b_end, float width_buffer = 0) : 
        image(_image),
        //Swap based on the x values, so that intersections are calculated the same regardless of what order the lines are given.
        a_start((line_a_start.x <= line_a_end.x) ? line_a_start : line_a_end),
        a_end((line_a_start.x <= line_a_end.x) ? line_a_end : line_a_start),
        b_start((line_b_start.x <= line_b_end.x) ? line_b_start : line_b_end),
        b_end((line_b_start.x <= line_b_end.x) ? line_b_end : line_b_start),
        center(get_intersection()),
        angle(get_angle()),
        overlap_width(get_overlap_width(width_buffer)),
        li(line_iterator<IMG_TYPE>(image, a_start, a_end))
    {
        shrink_iterator();
    }

    //Wrapper for contained iterator. 
    //Get current whole-number coordinate
    scoord cur_coord()
    {
        return li.cur_coord();
    }

    //Wrapper for contained iterator. 
    //Step coordinate forward by 1 pixel width
    //Returns:
    //   false: Reached end of overlap
    //   true: Stepped forward
    bool step()
    {
        return li.step();
    }

    IMG_TYPE get() const
    {
        return li.get();
    }
};
#endif
