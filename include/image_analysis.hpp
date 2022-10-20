#ifndef IMAGE_ANALYSIS_HPP
#define IMAGE_ANALYSIS_HPP
#define cimg_use_png 1
#include "CImg.h"
#include <iterator>
#include <math.h>
#include <map>
#include <vector>
#include <algorithm>
#include <limits>
#include <error.h>

#define MINMAX(a,b) int temp = a; a = std::min(a,b); b = std::max(temp,b);

using namespace cimg_library;
using std::min;
using std::max;
using std::vector;
using std::map;

namespace image_analysis
{
    template <typename T>
    static CImg<int> sized_dark_regions(const CImg<T>& input_image,const T threshold, bool ignore_zeros = true)
    {
        CImg<int> regions = input_image.get_threshold(threshold).label(false, 0, true);        
        map<int, int> region_sizes;
        region_sizes[-1] = 0;
        cimg_forXY(regions, x, y)
        {
            if((input_image(x,y) > threshold )|| (ignore_zeros && input_image(x,y) == 0))
            {
                regions(x,y) = -1;
            }
            else
            {
                int cur_region = regions(x,y);
                if(region_sizes.find(cur_region) != region_sizes.end())
                {
                    ++region_sizes[cur_region];
                }
                else
                {
                    region_sizes[cur_region] = 1;
                    std::cout << "adding region label " << cur_region << '\n';
                }
            }
        }
        cimg_forXY(regions, x, y)
        {
            regions(x,y) = region_sizes[regions(x,y)];
        }
        return regions;
    };
    
    template <typename T>
    static CImg<int> sized_light_regions(const CImg<T>& input_image,const T threshold, bool ignore_zeros = true)
    {
        CImg<int> regions = CImg<int>(input_image.get_threshold(threshold)).label(false, 0, true);        
        map<int, int> region_sizes;
        region_sizes[-1] = 0;
        cimg_forXY(regions, x, y)
        {
            int cur_region = regions(x,y);
            if((cur_region < threshold ) || (ignore_zeros && (cur_region == 0)))
            {
                regions(x,y) = -1;
            }
            else if(region_sizes.find(cur_region) != region_sizes.end())
            {
                ++region_sizes[cur_region];
            }
            else
            {
                region_sizes[cur_region] = 1;
            }
        }
        cimg_forXY(regions, x, y)
        {   

            regions(x,y) = region_sizes[regions(x,y)];
        }
        return regions;
    };
    
    template <typename T>
    static CImg<T> color_difference(const CImg<T>& rgb, const T* color)
    {
        CImg<T> rgb_Lab = rgb.get_RGBtoLab();
        CImg<T> color_Lab = CImg<T>(color, 1, 1, 1, 3).RGBtoLab();
        CImg<T> diff(rgb.width(), rgb.height(), 1, 1);
        
        cimg_forC(rgb_Lab, c)
        {
            diff += (rgb_Lab.get_shared_channel(c) - color_Lab[c]).sqr();
        }
        diff.sqrt();
        return diff;
    }
}


#endif
