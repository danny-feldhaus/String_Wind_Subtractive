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
#include <line_iterator.hpp>

//Shrink to the given min/max x coordinates.
//  Avoids intermediate steps with two larger jumps
template<class T>
bool line_iterator<T>::shrink_around_x(float min_x, float max_x)
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
template<class T>
bool line_iterator<T>::shrink_around_y(float min_y, float max_y)
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

template<class T>
bool line_iterator<T>::step_back()
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

template<class T>
T line_iterator<T>::get() const
{
    if(interpolate) return image.linear_atXY(cur_pos.x,cur_pos.y);
    return image(cur_pos.x,cur_pos.y);
}

template<class T>
T line_iterator<T>::set(T val)
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

template<class T>
coord<short> line_iterator<T>::cur_coord()
{
    return scoord(cur_pos.x,cur_pos.y);
}

template<class T>
coord<short> line_iterator<T>::left()
{
    return scoord(cur_pos.x + step_size.y,cur_pos.y - step_size.x);
}

template<class T>
coord<short> line_iterator<T>::right()
{
    return scoord(cur_pos.x - step_size.y,cur_pos.y + step_size.x);
}

template<class T>
int line_iterator<T>::idx()
{
    //Index equation copied right from CImg.h
    return cur_pos.x + cur_pos.y * image.width();
}

//Private

template<class T>
coord<float> line_iterator<T>::get_left_offset() const
{
    fcoord offset(-d.y, d.x);
    if(scoord(pos + offset) == scoord(pos))
    {
        offset *= 2;
    }
    return offset;
}

template<class T>
coord<float> line_iterator<T>::get_right_offset() const
{
    fcoord offset(d.y, -d.x);
    if(scoord(pos + offset) == scoord(pos))
    {
        offset *= 2;
    }
    return offset;
}

template class line_iterator<short>;
template class line_iterator<bool>;
template class line_iterator<u_char>;
template class line_iterator<int>;
template class line_iterator<float>;
template class line_iterator<u_short>;