#ifndef STRING_ART_H
#define STRING_ART_H
#include "coord.hpp"
#include "CImg.h"
#include "line_iterator.hpp"
#include <map>
#include <vector>
#include <deque>
#include <math.h>

using std::make_pair;
using std::map;
using std::max;
using std::min;
using std::pair;
using std::vector;
using coordinates::coord;

static vector<coord<short>> circular_pins(short center_x, short center_y, short radius, short pin_count)
{
    vector<coord<short>> pins;
    float angle;
    int x, y;
    for (int i = 0; i < pin_count; i++)
    {
        angle = (2.f * 3.14159f * i) / pin_count;
        x = center_x + std::cos(angle) * radius;
        y = center_y + std::sin(angle) * radius;
        pins.push_back({(short)x,(short)y});
    }
    return pins;
}


template <typename IMG_TYPE>
class string_art
{
    typedef cimg_library::CImg<IMG_TYPE> tcimg;
    typedef coord<short> scoord;
    typedef map<short, map<short, IMG_TYPE *>> sbp;

public:
    tcimg &image;
    tcimg string_image;
    tcimg string_image_old;
    short pin_count;
    short line_count;
    string_art(tcimg &_image, vector<scoord> &_pins, int min_separation = 1);
    short best_pin_for(short from_pin, IMG_TYPE& score, short depth = 1);


    // Score every line based on its average pixel value
    void score_all_lines();
    
    // Update scores based on an overlapping line
    void update_scores(short pin_a, short pin_b);

private:
    vector<vector<int> *> line_indices;
    vector<IMG_TYPE> line_scores;
    vector<scoord> line_pairs;
    vector<scoord>& pins;
    
    sbp scores_by_pin;
    void make_map(short min_separation);

    vector<scoord> overlapping_lines(short pin_a, short pin_b);
    

    IMG_TYPE calculate_score(short pin_a, short pin_b, u_char method = 0);

    IMG_TYPE score_square(tcimg& string_image_ref, std::deque<scoord>& line_coords);
    
    IMG_TYPE darken_line(short pin_a, short pin_b, float multiplier);

    void draw_line(short pin_a, short pin_b, IMG_TYPE color = 0);
    //Return the best-scoring next pin.
    IMG_TYPE best_score_for(short from_pin, short depth, short& to_pin, vector<short>& visited);
};
#endif
