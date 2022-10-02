#ifndef LINE_MAP_H
#define LINE_MAP_H

#include "CImg.h"
#include <map>
#include <vector>
#include <math.h>
#define PAIR_CONTAINS(p, val) p.first == val || p.second == val

using std::map;
using std::pair;
using std::make_pair;
using std::vector;
using std::min;
using std::max;
typedef pair<short,short> coord;

template<typename IMG_TYPE>
class line_iterator
{
    private:
        int radius;
        cimg_library::CImg<IMG_TYPE>& image;
        const int start_x, start_y, end_x, end_y;
        const int dy, dx;
        const bool y_major;
        const float d_maj, d_min, d_minmaj;
        int &maj_ax, &min_ax;
        const int &maj_start, &maj_end, &min_start, &min_end;
        const int maj_dir, maj_dim, min_dim;

        int max_minor;

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = IMG_TYPE;
        using pointer           = IMG_TYPE*;
        using reference         = IMG_TYPE&;
        int x, y;
        bool done = false;

        int thickness;
        int steps;
        line_iterator(cimg_library::CImg<IMG_TYPE>& image, int _start_x, int _start_y, int _end_x, int _end_y, int _thickness) : image(image), 
                                                                                                         start_x(_start_x), start_y(_start_y), end_x(_end_x), end_y(_end_y), 
                                                                                                         dy(end_y - start_y), dx(end_x - start_x),
                                                                                                         y_major(abs(dy) > abs(dx)),
                                                                                                         d_maj(y_major ? dy : dx), d_min(y_major ? dx : dy), d_minmaj(d_min/d_maj),
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
            steps = thickness * (abs(d_maj)+1) ;
        };
        int idx()
        {
            return x + y*image.width();
        }

        //Reference to the pixel value at the current position
        reference operator*() const {return image(x,y);}
        
        pointer operator->() {return &image.at(x,y);}

        //Iterate along the line. After reaching the end, done=IMG_TYPErue and this operator does nothing.
        bool step()
        {
            if(min_ax == max_minor)
            {
                if(maj_ax == maj_end)
                {
                    return false;
                }
                maj_ax += maj_dir;
                min_ax = (min_start + (maj_ax - maj_start) * d_minmaj) - radius;
                max_minor = (min_ax + thickness - 1);
                min_ax = max(0, min_ax);
                max_minor = min(max_minor, min_dim - 1);
            }
            else
            {
                min_ax++;
            }
            return true;
        }
};

static vector<coord> circular_pins(short center_x, short center_y, short radius, short pin_count)
{
    vector<coord> pins;
    float angle;
    int x, y;
    for(int i=0; i < pin_count; i++)
    {
        angle = (2.f * 3.14159f * i) / pin_count;
        x = center_x + std::cos(angle) * radius;
        y = center_y + std::sin(angle) * radius;
        pins.push_back(make_pair<short, short>(x,y));
    }
    return pins;
}

template <typename IMG_TYPE>
class line_map
{
    typedef cimg_library::CImg<IMG_TYPE> cimg;
    typedef map<short, map<short, vector<int>*>> lbp;
    typedef map<short, map<short, IMG_TYPE*   >> sbp;
    public:
        cimg& image;
        int pin_count;
        int line_count;
        line_map(cimg& _image, vector<coord>& pins, int thickness = 1, int min_separation = 1) : image(_image), pin_count(pins.size())
        {
            make_map(pins, thickness, min_separation);
            score_all_lines();
            return;
        }
        vector<int>& line_between(short pin_a, short pin_b)
        {
            std::cout << "Line between " << pin_a << " and " << pin_b << " has " << lines_by_pin[pin_a][pin_b] -> size() << " elements.\n";
            if(lines_by_pin[pin_a].find(pin_b) == lines_by_pin[pin_a].end()) std::cout << "ERROR: NO LINE BETWEEN " << pin_a << " AND " << pin_b << '\n';
            return *(lines_by_pin[pin_a][pin_b]);
        }
        map<short, vector<int>*> lines_from_pin(int pin)
        {
            return lines_by_pin[pin];
        }
        short best_pin_for(short from_pin, IMG_TYPE& score)
        {
            score = 0;
            short best_pin = -1;
            for(auto b : scores_by_pin[from_pin])
            {
                if(*(b.second) > score)
                {
                    score = *(b.second);
                    best_pin = b.first;
                }
            }
            return best_pin;
        }
        void score_all_lines()
        {

            for(int i = 0; i < line_count; i++)
            {
                line_scores[i] = calculate_score(line_indices[i]);
            }
            return;
        }
        //NOTE: This doesn't erase the actual data, just removes it from the map.
        //  Currently, the contents of line_indices are only accessed through the map, so this shouldn't be a problem.
        bool cull_line(int pin_a, int pin_b)
        {
            if(!(lines_by_pin.find(pin_a) != lines_by_pin.end() && lines_by_pin[pin_a].find(pin_b) != lines_by_pin[pin_a].end())) return false;
            lines_by_pin[pin_a].erase(pin_b);
            lines_by_pin[pin_b].erase(pin_a);
            return true;
        }

    private:
        vector<vector<int>> line_indices;
        vector<IMG_TYPE> line_scores;
        lbp lines_by_pin;
        sbp scores_by_pin;
        void make_map(vector<coord>& pins, int thickness, int min_separation)
        {
            int b_start = 0, b_end = 0;
            line_count = 0;
            for(short a = 0; a < pin_count; a++)
            {
                b_start = (a + min_separation) % pin_count;
                b_end = (a - min_separation >= -1) ? pin_count : (pin_count + a - min_separation) % pin_count;
                for(short b = b_start; b <= b_end; b++)
                {
                    vector<int> cur_indices();
                     
                    line_indices.push_back(vector<int>());
                    line_scores.push_back(0);
                    lines_by_pin[a][b] = &(line_indices.back());
                    lines_by_pin[b][a] = &(line_indices.back());
                    scores_by_pin[a][b] =&(line_scores.back());
                    scores_by_pin[b][a] =&(line_scores.back());

                    line_iterator<IMG_TYPE> li(image, pins[a].first, pins[a].second, pins[b].first, pins[b].second, thickness);
                    while(li.step())
                    {
                        line_indices.back().push_back(li.idx());
                    }
                    line_count++;
                }
            }
        }
        vector<vector<int>*> overlapping_lines(short pin_a, short pin_b)
        {
            vector<vector<int>*> lines;
            map<short, map<short, vector<int>*>>::iterator a;
            //Add overlaps where a is before pin_a, and b is between pin_a and pin_b
            for(a = lines_by_pin.begin(); a -> first < pin_a; a++)
            {
                auto b = a -> second.begin();
                while(b -> first < pin_a) b++;
                while(b -> first < pin_b)
                {
                    lines.push_back(b->second);
                }
            }
            //a is now between pin_a and pin_b
            //Add overlaps where a is between pin_a and pin_b, and b is greater than pin_b
            for(;a -> first < pin_b; a++)
            {
                for(auto b = --(a->second.end()); b -> first > pin_b; --b)
                {
                    lines.push_back(b->second);
                }
            }
            return lines;
        }
        IMG_TYPE calculate_score(short pin_a, short pin_b)
        {
            return calculate_score(&lines_by_pin[pin_a][pin_b]);
        }
        IMG_TYPE calculate_score(vector<int>& indices)
        {
            long sum = 0;
            for(int idx : indices)
            {
                sum += image[idx];
            }
            return (IMG_TYPE)(sum / indices.size());
        }
};
#endif
