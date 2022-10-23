#include <line.hpp>
using namespace coordinates;

template <class T>
line<T>::line(const fcoord _a, const fcoord _b, tcimg* _image, const short _z, const short _c)
:
a((_a.x < _b.x) ? _a : _b),
b((_a.x < _b.x) ? _b : _a),
length(distance(a,b)),
d((b - a) / length),
z(_z),
c(_c)
{
    image = _image;
    if(image && !contained_in(*image)) throw CImgArgumentException("Line does not fit within image.\n");
}

template <class T>
line<T>::line(const short ax, const short ay, const short bx, const short by, tcimg* _image, const short z, const short c)
    : line(fcoord(ax,ay),fcoord(bx,by), _image, z, c) {};

template <class T>
bool line<T>::contained_in(const tcimg& img) const
{ 
    return(img.containsXYZC(a.x, a.y, 0, 0) && img.containsXYZC(b.x, b.y, 0, 0));
}

template <class T>
bool line<T>::is_empty() const
{
    return size() == 1;
}

template <class T>
line<T> line<T>::operator&(const line<T>& other) const
{
    if(a == other.a && b == other.b)
    {
        return line<T>(*this);
    }
    float a1 = b.y - a.y;
    float b1 = a.x - b.x;
    float c1 = a1*a.x + b1*a.y;

    float a2 = other.b.y - other.a.y;
    float b2 = other.a.x - other.b.x;
    float c2 = a2*other.a.x + b2*other.a.y;
    
    float det = a1 * b2 - a2 * b1;
    //Lines are parallel if determinant is zero
    if(det == 0)
        return line<T>(a, b, image, z, c);
    
    fcoord center((b2 * c1 - b1 * c2) / det, (a1 * c2 - a2 * c1) / det);
    float angle = acos(dot<float>(b - a,other.b-other.a) / (length * other.length));
    float overlap_width = 1.f / sin(angle);
    fcoord actual_start = center - d * overlap_width / 2;
    fcoord actual_end = center + d * overlap_width / 2;
    fcoord start_offset = actual_start - a;
    fcoord end_offset = b - actual_end;
    short start_steps, end_steps;
    if(abs(d.x) > abs(d.y))
    {
        start_steps = (short) start_offset.x / d.x;
        end_steps = 1 + (short) end_offset.x / d.x;
    }
    else
    {
        start_steps = (short) start_offset.y / d.y;
        end_steps = 1 + (short) end_offset.y / d.y;
    }
    fcoord start_coord = (begin() + start_steps).get_pos();
    fcoord end_coord = (end() - end_steps).get_pos();
    return line<T>(start_coord, end_coord, image, z, c);
}

template <class T>
bool line<T>::coord_at(const double d_from_start, fcoord& result) const
{
    if((d_from_start < 0) || (d_from_start > length))
        return false;
    result = a + d * d_from_start;
    return true;
}

template <class T>
T& line<T>::at(const double d_from_start, const short c, const short z) const
{
    fcoord pos(0,0);
    if(!image || !coord_at(d_from_start, pos)) throw std::runtime_error("Image required for at()");
    return image->atXYZC(pos.x, pos.y, z, c);
}

template <class T>
typename line<T>::line_iterator line<T>::begin() const
{
    if(!image) throw std::runtime_error("Image required for line iterator");
    return line_iterator(*image, a, d, 0, z, c);
}

template <class T>
typename line<T>::line_iterator line<T>::end() const
{
    if(!image) throw std::runtime_error("Image required for line iterator");
    return line_iterator(*image, a + d*size(), d, size(), z, c);
}
        
template <class T>
line<T>::line_iterator::line_iterator(tcimg &_image, const fcoord _pos, const fcoord _d, const short _index, const short _z, const short _c)
    :image(_image),
    pos(_pos),
    index(_index),
    d(_d),
    z(_z),
    c(_c)
{
}

template <class T>
typename line<T>::line_iterator& line<T>::line_iterator::operator++()
{
    index++;
    pos += d;
    return *this;
}

template <class T>
typename line<T>::line_iterator& line<T>::line_iterator::operator--()
{
    index--;
    pos -= d;
    return *this;
}

template <class T>
bool line<T>::line_iterator::operator==(const line_iterator& other) const
{
    return index == other.index;
}

template <class T>
bool line<T>::line_iterator::operator!=(const line_iterator& other) const
{
    return index != other.index;
}

template <class T>
bool line<T>::line_iterator::operator<(const line_iterator& other) const
{
    return index < other.index;
}

template <class T>
bool line<T>::line_iterator::operator>(const line_iterator& other) const
{
    return index > other.index;
}

template <class T>
bool line<T>::line_iterator::operator<=(const line_iterator& other) const
{
    return index <= other.index;
}

template <class T>
bool line<T>::line_iterator::operator>=(const line_iterator& other) const
{
    return index >= other.index;
}

template <class T>
typename line<T>::line_iterator line<T>::line_iterator::operator++(int)
{
    line_iterator old(image, pos, d, index, z, c);
    pos += d;
    index++;
    return old;
}

template <class T>
typename line<T>::line_iterator line<T>::line_iterator::operator--(int)
{
    line_iterator old(image, pos, d, index, z, c);
    pos -= d;
    index--;
    return old;
}

template <class T>
typename line<T>::line_iterator line<T>::line_iterator::operator+(const difference_type& n) const
{
    return line_iterator(image, pos + d*n, d, index+n, z, c);
}

template <class T>
typename line<T>::line_iterator line<T>::line_iterator::operator-(const difference_type& n) const
{
    return line_iterator(image, pos - d*n, d, index-n, z, c);
}

template <class T>
typename line<T>::line_iterator& line<T>::line_iterator::operator+=(const difference_type& n)
{
    pos += d*n;
    index += n;
    return *this;
}

template <class T>
typename line<T>::line_iterator& line<T>::line_iterator::operator-=(const difference_type& n)
{
    pos -= d*n;
    index -= n;
    return *this;
}

template <class T>
typename line<T>::line_iterator line<T>::line_iterator::left() const
{
    const fcoord offset(-d.y, d.x);
    fcoord c_left = pos + offset;
    if(scoord(c_left) == scoord(pos))
    {
        c_left += offset;
    }
    return line_iterator(image, c_left, offset, index+1, z, c);
}

template <class T>
typename line<T>::line_iterator line<T>::line_iterator::right() const
{
    const fcoord offset(d.y, -d.x);
    fcoord c_right = pos + offset;
    if(scoord(c_right) == scoord(pos))
    {
        c_right += offset;
    }
    return line_iterator(image, c_right, offset, index+1, z, c);
}

template <class T>
const coord<float>& line<T>::line_iterator::get_pos() const
{
    return pos;
}



template class coordinates::line<short>;
template class coordinates::line<float>;
template class coordinates::line<u_short>;
template class coordinates::line<int>;
template class coordinates::line<u_char>;