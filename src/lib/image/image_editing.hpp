#ifndef IMAGE_EDITING_HPP
#define IMAGE_EDITING_HPP
#include "CImg.h"
//#include "image_analysis.hpp"
#include <iterator>
#include <math.h>
#include <map>
#include <vector>

using namespace cimg_library;
using std::min;
using std::max;

namespace image_editing
{

    template<typename T> 
    void mult_points(CImg<T>& img, vector<int>& idxs, float multiplier)
    {
        for(int idx : idxs)
        {
            img[idx] *= multiplier;
        }
    }
    /*
    template<typename T>
    void score_and_multiply(CImg<T>& img, int pin_a, int pin_b, linemap& string_art, map<int, vector<vector<int>>>& to_update, float multiplier)
    {
        for(auto other_a = string_art.lower_bound(pin_a+1); other_a != string_art.end() && other_a->first < pin_b; ++other_a)
        {
            for(auto other_b = other_a->second.begin(); other_b != other_a->second.end(); other_b++)
            {
                if(other_b -> first < pin_a || other_b -> first > pin_b)
                {
                    to_update[]
                    image_analysis::update_score(img, other_a->first, other_b->first, pin_a, pin_b, multiplier, line_scores, string_art);
                }
            }       
        }
        mult_points(img, string_art[pin_a][pin_b], multiplier);
        line_scores[pin_a][pin_b] = image_analysis::average_along_line(img, string_art[pin_a][pin_b]);
        return;
    }   */


    template<typename T> 
    void draw_points(CImg<T>& img, vector<int>& idxs, T color = 255)
    {
        for(int idx : idxs)
        {
            img[idx] = color;
        }
    }

    template<typename T> 
    void draw_line(CImg<T>& img, int ax, int ay, int bx, int by, T color = 255)
    {
        img.draw_line(ax,ay,bx,by,&color);
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
