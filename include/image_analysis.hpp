#ifndef IMAGE_ANALYSIS_HPP
#define IMAGE_ANALYSIS_HPP
#define cimg_use_png 1
#include <CImg/CImg.h>
#include <coord.hpp>
#include <line.hpp>
#include <math.h>
#include <map>
#include <vector>
#include <algorithm>
#include <limits>
#include <error.h>

using namespace cimg_library;
using namespace coordinates;
using std::min;
using std::max;
using std::vector;
using std::map;
typedef coord<short> scoord;

template<typename T>
class image_analysis
{
    public:
    /**
     * @brief Map the size of all thresholded regions of the input image
     * 
     * @param input_image Image to analyze
     * @param threshold Threshold of calculated regions (pixels with higher values are not in a region)
     * @param ignore_zeros Zero values are considered outside of the threshold when \c true
     * @return Region size map
     */
    static CImg<int> sized_dark_regions(const CImg<T>& input_image,const T threshold, bool ignore_zeros = true);
    
    static CImg<int> sized_light_regions(const CImg<T>& input_image,const T threshold, bool ignore_zeros = true);
    
    static CImg<T> color_difference(const CImg<T>& rgb, const T* color);

    static float color_difference3x3(const CImg<T>& image_a, const CImg<T>& image_b, const scoord center);

    static float line_color_difference3x3(const CImg<T>& image_a, const CImg<T>& image_b, const scoord point_a, const scoord point_b);
};


#endif
