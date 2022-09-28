#ifndef IMAGE_ANALYSIS_HPP
#define IMAGE_ANALYSIS_HPP
#include "CImg.h"
#include <iterator>
#include <math.h>

using namespace cimg_library;
using std::min;
using std::max;

namespace image_analysis
{
    template<typename T>
    class line_iterator
    {
    private:
        int radius;
        CImg<T>& image;
        const int start_x, start_y, end_x, end_y;
        const float dy, dx;
        const bool y_major;
        const float d_maj, d_min;
        int &maj_ax, &min_ax;
        const int &maj_start, &maj_end, &min_start, &min_end;
        const int maj_dir, maj_dim, min_dim;

        int max_minor;

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = T;
        using pointer           = T*;
        using reference         = T&;
        int x, y;
        bool done = false;

        int thickness;
        int steps;
        line_iterator(CImg<T>& image, int _start_x, int _start_y, int _end_x, int _end_y, int _thickness) : image(image), 
                                                                                                         start_x(_start_x), start_y(_start_y), end_x(_end_x), end_y(_end_y), 
                                                                                                         dy(end_y - start_y), dx(end_x - start_x),
                                                                                                         y_major(abs(dy) > abs(dx)),
                                                                                                         d_maj(y_major ? dy : dx), d_min(y_major ? dx : dy),
                                                                                                         maj_ax(y_major ? y : x), min_ax(y_major ? x : y),
                                                                                                         maj_start(y_major ? start_y : start_x), maj_end(y_major ? end_y : end_x),
                                                                                                         min_start(y_major ? start_x : start_y), min_end(y_major ? end_x : end_y),
                                                                                                         maj_dir(maj_end > maj_start ? 1 : -1),
                                                                                                         maj_dim(y_major ? image.height() : image.width()),
                                                                                                         min_dim(y_major ? image.width() : image.height())
        {
            float angle = atan2(d_min,d_maj);
            thickness = max(_thickness / abs(sin(M_PI/2 - angle)), 1.0);
            radius = thickness / 2;
            min_ax = max(min_start - radius, 0);
            maj_ax = maj_start;
            max_minor = min(min_ax + thickness - 1, min_dim-1);
            steps = thickness * d_maj;
        };

        //Reference to the pixel value at the current position
        reference operator*() const {return image(x,y);}
        
        pointer operator->() {return &image.at(x,y);}

        //Iterate along the line. After reaching the end, done=True and this operator does nothing.
        line_iterator& operator++() 
        {
            if(!done)
            {
                if(min_ax == max_minor)
                {
                    if(maj_ax == maj_end)
                    {
                        done = true;
                        return *this;
                    }
                    maj_ax += maj_dir;
                    min_ax = (min_start + (maj_ax - maj_start) * (d_min / d_maj) - radius);
                    max_minor = (min_ax + thickness - 1);
                    min_ax = max(0, min_ax);
                    max_minor = min(max_minor, min_dim - 1);
                }
                else
                {
                    min_ax++;
                }
            }
            return *this;
        }

        
        

    };

}


#endif
