#ifndef IMAGE_EDITING_HPP
#define IMAGE_EDITING_HPP
#include <CImg/CImg.h>
//#include "image_analysis.hpp"
#include <line.hpp>
#include <coord.hpp>
#include <math.h>
#include <map>
#include <vector>

using namespace cimg_library;
using namespace coordinates;
using std::min;
using std::max;
using std::vector;

namespace image_editing
{
    template<typename T> 
    void draw_points(CImg<T>& img, vector<int>& idxs, T color = 255);

    template <class T>
    void draw_line(CImg<T> &image, const coord<short> line_a, const coord<short> line_b, const T color, const short buffer = 0)
    {
        line<T> l(line_a, line_b, &image);
        for(auto a = l.begin() + buffer; a < l.end() - buffer; a++)
        {
            (*a) = color;
        }
        return;
    }

    /**
     * @brief Multiply pixels along a line by a constant
     * @tparam T Image type
     * @param image Image to modify
     * @param line_a Start coordinate of line
     * @param line_b End coordinate of line
     * @param multiplier Amount to multiply each pixel by
     * @param buffer Steps to skip at start / end of line
     * @param multiply_LR If \c true pixels to the left and right (oriented to the line) are also multiplied. If \c false , only pixels in the line are modified.
     */
    template<typename T>
    void multiply_line(CImg<T>& image, const coord<short> line_a, const coord<short> line_b, const float multiplier, const short buffer = 0, const bool multiply_LR = false)
    {
        line<T> l(line_a, line_b, &image);
        for(auto a = l.begin() + buffer; a < l.end() - buffer; a++)
        {
            (*a) *= multiplier;
            if(multiply_LR)
            {
                *(a.left()) *= multiplier;
                *(a.right()) *= multiplier;
            }
        }
        return;
        return;
    }

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
