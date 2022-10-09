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
    float angle;
    interpolate = _interpolate;
    //Order start/end by x value. This is to avoid lines with swapped start/end behaving differently.
    start = (point_a.x < point_b.x) ? point_a : point_b;
    end   = (point_a.x < point_b.x) ? point_b : point_a;
    angle = atan2(end.y - start.y, end.x - start.x);

    interpolate = _interpolate;
    line_length = distance(start,end);
    step_size = {cos(angle),sin(angle)};
    cur_pos = start;
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
    cur_pos.x += step_size.x;
    cur_pos.y += step_size.y;
    ++cur_length;
    return true;
}

//Shrink to the given min/max x coordinates.
//  Avoids intermediate steps with two larger jumps
template<class IMG_TYPE>
bool line_iterator<IMG_TYPE>::shrink_around_x(float min_x, float max_x)
{
        int steps_to_min = abs((min_x - start.x) / step_size.x);
        int steps_to_max = abs((end.x - max_x) / step_size.x);
        start.x += step_size.x * steps_to_min;
        start.y += step_size.y * steps_to_min;
        end.x -= step_size.x * steps_to_max;
        end.y -= step_size.y * steps_to_max;
        line_length = distance<float>(start, end);
        cur_pos.x = start.x;
        cur_pos.y = start.y;
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
        steps_to_min = ((min_y-start.y) / step_size.y);
        steps_to_max = ((end.y - max_y) / step_size.y);
        start.x  += step_size.x * steps_to_min;
        start.y += step_size.y * steps_to_min;
        end.x    -= step_size.x * steps_to_max;
        end.y   -= step_size.y * steps_to_max;
    }
    else
    {
        steps_to_min = abs((min_y - end.y) / step_size.y);
        steps_to_max = abs((start.y-max_y) / step_size.y);
        start.x  += step_size.x * steps_to_max;
        start.y += step_size.y * steps_to_max;
        end.x    -= step_size.x * steps_to_min;
        end.y   -= step_size.y * steps_to_min;       
    }

    line_length = distance<float>(start, end);
    cur_pos.x = start.x;
    cur_pos.y = start.y;
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
    cur_pos.x -= step_size.x;
    cur_pos.y -= step_size.y;
    return true;
}

template<class IMG_TYPE>
IMG_TYPE line_iterator<IMG_TYPE>::get() const
{
    if(interpolate) return image.linear_atXY(cur_pos.x,cur_pos.y);
    return image(cur_pos.x,cur_pos.y);
}

template<class IMG_TYPE>
IMG_TYPE line_iterator<IMG_TYPE>::set(IMG_TYPE val)
{
    if(interpolate) 
    {
        image.set_linear_atXY(val,cur_pos.x,cur_pos.y);
    }
    else
    {
        image(cur_pos.x,cur_pos.y) = val;
    }
    return val;
}

template<class IMG_TYPE>
coord<short> line_iterator<IMG_TYPE>::cur_coord()
{
    return scoord(cur_pos.x,cur_pos.y);
}

template<class IMG_TYPE>
int line_iterator<IMG_TYPE>::idx()
{
    //Index equation copied right from CImg.h
    return cur_pos.x + cur_pos.y * image.width();
}

template class line_iterator<short>;
template class line_iterator<bool>;
template class line_iterator<u_char>;