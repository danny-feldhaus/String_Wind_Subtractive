#ifndef IMAGE_EDITING_HPP
#define IMAGE_EDITING_HPP
#include "CImg.h"
//#include "image_analysis.hpp"
#include "line_iterator.hpp"
#include "coord.hpp"
#include <math.h>
#include <map>
#include <vector>

using namespace cimg_library;
using std::min;
using std::max;

namespace image_editing
{
    template<typename T> 
    void draw_points(CImg<T>& img, vector<int>& idxs, T color = 255)
    {
        for(int idx : idxs)
        {
            img[idx] = color;
        }
    }

    template <class IMG_TYPE>
    void draw_line(CImg<IMG_TYPE> &image, const coord<short> line_a, const coord<short> line_b, const IMG_TYPE color, const short buffer = 0)
    {
        line_iterator<IMG_TYPE> li(image, line_a.x, line_a.y, line_b.x, line_b.y, false, 0);
        for(int i=0; i < buffer; i++)
        {
            li.step();
        }
        do
        {
            li.set(color);
        } while (li.step() &&( li.cur_length <= li.line_length - buffer));
        return;
    }

    /*
    template<typename T>
    void draw_all_lines(CImg<T>& img, linemap& string_art)
    {
        for(auto a : string_art)
        {
            for(auto b : a.second)
            {
                draw_points<T>(img, b.second, 255);
            }
        }
    }
    */
    template<typename T>
    void color_difference(CImg<T>& image_a, CImg<T>& image_b, CImg<T>& out)
    {
        CImg<float> diff = image_a.get_RGBtoLab();
        diff -= image_b.get_RGBtoLab();
        diff.sqr();
        diff = diff.get_channel(0) + diff.get_channel(1) + diff.get_channel(2);
        diff.sqrt();
        out = diff;
        return;
    }
}


#endif
