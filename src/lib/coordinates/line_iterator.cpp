/**
 * @file line_iterator.cpp
 * @author Danny Feldhaus (danny.b.feldhaus@gmail.com)
 * @brief Iterates between two points on a CImg image
 * @version 0.1
 * @date 2022-10-07
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "line_iterator.hpp"

template <class IMG_TYPE>
line_iterator<IMG_TYPE>::line_iterator(tcimg &_image, scoord point_a, scoord point_b, bool _interpolate) 
    :  image(_image)
{
    interpolate = _interpolate;
    //Order start/end by x value. This is to avoid lines with swapped start/end behaving differently.
    start = (point_a.x < point_b.x) ? point_a : point_b;
    end   = (point_a.x < point_b.x) ? point_b : point_a;
    interpolate = _interpolate;
    line_length = distance(start,end);
    //Todo: make a line.hpp function for this
    float angle = atan2(end.y - start.y, end.x - start.x);
    x_step = cos(angle);
    y_step = sin(angle);
    cur_x = start.x;
    cur_y = start.y;
}

//Constructor. Creates some coord objects and calls the above constructor with them.
template <class IMG_TYPE>
line_iterator<IMG_TYPE>::line_iterator(tcimg &_image, short ax, short ay, short bx, short by, bool _interpolate) 
    : line_iterator(_image, coord(ax,ay), coord(bx,by), _interpolate)
{}

//Step by one pixel width.
template<class IMG_TYPE>
bool line_iterator<IMG_TYPE>::step()
{
    //Return false if this would step past the end point
    if(cur_length + 1 > line_length) return false;
    cur_x += x_step;
    cur_y += y_step;
    ++cur_length;
    return true;
}

//Shrink to the given min/max x coordinates.
//  Avoids intermediate steps with two larger jumps
template<class IMG_TYPE>
bool line_iterator<IMG_TYPE>::shrink_around_x(float min_x, float max_x)
{
        int steps_to_min = abs((min_x - start.x) / x_step);
        int steps_to_max = abs((end.x - max_x) / x_step);
        start.x += x_step * steps_to_min;
        start.y += y_step * steps_to_min;
        end.x -= x_step * steps_to_max;
        end.y -= y_step * steps_to_max;
        line_length = distance<float>(start, end);
        cur_x = start.x;
        cur_y = start.y;
        cur_length = 0;
        return true;
}


//Shrink to the given min/max y coordinates.
//  Avoids intermediate steps with two larger jumps
template<class IMG_TYPE>
bool line_iterator<IMG_TYPE>::shrink_around_y(float min_y, float max_y)
{
    float steps_to_min, steps_to_max;
    //start & end are not sorted by y, so some values need to be swapped when the start is below the end.
    if(start.y < end.y)
    {
        steps_to_min = ((min_y-start.y) / y_step);
        steps_to_max = ((end.y - max_y) / y_step);
        start.x  += x_step * steps_to_min;
        start.y += y_step * steps_to_min;
        end.x    -= x_step * steps_to_max;
        end.y   -= y_step * steps_to_max;
    }
    else
    {
        steps_to_min = abs((min_y - end.y) / y_step);
        steps_to_max = abs((start.y-max_y) / y_step);
        start.x  += x_step * steps_to_max;
        start.y += y_step * steps_to_max;
        end.x    -= x_step * steps_to_min;
        end.y   -= y_step * steps_to_min;       
    }

    line_length = distance<float>(start, end);
    cur_x = start.x;
    cur_y = start.y;
    cur_length = 0;
    return true;
}

template<class IMG_TYPE>
bool line_iterator<IMG_TYPE>::step_back()
{
    if(cur_length-- == 0)
    {
        cur_length = 0;
        return false;
    }
    cur_x -= x_step;
    cur_y -= y_step;
    return true;
}

template<class IMG_TYPE>
IMG_TYPE line_iterator<IMG_TYPE>::get()
{
    if(interpolate) return image.linear_atXY(cur_x,cur_y);
    return image(cur_x,cur_y);
}

template<class IMG_TYPE>
IMG_TYPE line_iterator<IMG_TYPE>::set(IMG_TYPE val)
{
    if(interpolate) 
    {
        image.set_linear_atXY(val,cur_x,cur_y);
    }
    else
    {
        image(cur_x,cur_y) = val;
    }
    return val;
}

template<class IMG_TYPE>
coord<short> line_iterator<IMG_TYPE>::cur_coord()
{
    return scoord(cur_x,cur_y);
}

template<class IMG_TYPE>
int line_iterator<IMG_TYPE>::idx()
{
    //Index equation copied right from CImg.h
    return cur_x + cur_y * image.width();
}

template class line_iterator<short>;
