#include "line_iterator.hpp"

template <class IMG_TYPE>
line_iterator<IMG_TYPE>::line_iterator(tcimg &_image, scoord _start, scoord _end, bool _interpolate) 
    :  image(_image)
{
    interpolate = _interpolate;
    start = (_start.x < _end.x) ? _start : _end;
    end   = (_start.x < _end.x) ? _end : _start;
    interpolate = _interpolate;
    line_length = distance(start,end);
    float angle = atan2(end.y - start.y, end.x - start.x);
    x_step = cos(angle);
    y_step = sin(angle);
    cur_x = start.x;
    cur_y = start.y;
}

template <class IMG_TYPE>
line_iterator<IMG_TYPE>::line_iterator(tcimg &_image, short _start_x, short _start_y, short _end_x, short _end_y, bool _interpolate) 
    : line_iterator(_image, coord(_start_x,_start_y), coord(_end_x,_end_y), _interpolate)
{}

template<class IMG_TYPE>
bool line_iterator<IMG_TYPE>::step()
{
    if(cur_length + 1 > line_length) return false;
    cur_x += x_step;
    cur_y += y_step;
    ++cur_length;
    return true;
}

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

template<class IMG_TYPE>
bool line_iterator<IMG_TYPE>::shrink_around_y(float min_y, float max_y)
{
    fcoord temp;
    float steps_to_min, steps_to_max;
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
    return cur_x + cur_y * image.width();
}

template class line_iterator<short>;
