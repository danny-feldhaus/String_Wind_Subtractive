//A struct for 2D coordinates, with CImg integration.

#ifndef LINE_H
#define LINE_H
#include <math.h>
using std::sqrt;
using std::pow;
namespace coordinates
{
    template <class T = short>
    struct line
    {
        typedef coord<T> tcoord;
        tcoord a;
        tcoord b;
        //Constructor
        line(const tcoord& _a, const tcoord& _b, bool sorted = true)
        {
            if(sorted && _a.x > _b.x)
            {
                a = _b;
                b = _a;
            }
            else
            {
                a = _a;
                b = _b;
            }
        }
        line(const T ax, const T ay, const T bx, const T by) : line(tcoord(ax,ay),tcoord(bx,by)) {};
        double length() const
        {
            return distance(a,b);
        }
    };

    template <class T = short>
    static double dot(const line<T> &line_a, const line<T> &line_b)
    {
        return dot(line_a.b - line_a.a,line_b.b-line_b.a);
    }
}


#endif