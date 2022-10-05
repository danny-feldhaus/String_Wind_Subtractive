#ifndef LINE_MAP_H
#define LINE_MAP_H

#include "CImg.h"
#include <map>
#include <vector>
#include <math.h>
#define PAIR_CONTAINS(p, val) p.first == val || p.second == val

using std::make_pair;
using std::map;
using std::max;
using std::min;
using std::pair;
using std::vector;
typedef pair<short, short> coord;

template <typename IMG_TYPE>
class line_iterator
{
private:
    int radius;
    cimg_library::CImg<IMG_TYPE> &image;
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
    using value_type = IMG_TYPE;
    using pointer = IMG_TYPE *;
    using reference = IMG_TYPE &;
    int x, y;
    bool done = false;

    int thickness;
    int steps;
    line_iterator(cimg_library::CImg<IMG_TYPE> &image, int _start_x, int _start_y, int _end_x, int _end_y, int _thickness) : image(image),
                                                                                                                             start_x(_start_x), start_y(_start_y), end_x(_end_x), end_y(_end_y),
                                                                                                                             dy(end_y - start_y), dx(end_x - start_x),
                                                                                                                             y_major(abs(dy) > abs(dx)),
                                                                                                                             d_maj(y_major ? dy : dx), d_min(y_major ? dx : dy), d_minmaj(d_min / d_maj),
                                                                                                                             maj_ax(y_major ? y : x), min_ax(y_major ? x : y),
                                                                                                                             maj_start(y_major ? start_y : start_x), maj_end(y_major ? end_y : end_x),
                                                                                                                             min_start(y_major ? start_x : start_y), min_end(y_major ? end_x : end_y),
                                                                                                                             maj_dir(maj_end > maj_start ? 1 : -1),
                                                                                                                             maj_dim(y_major ? image.height() : image.width()),
                                                                                                                             min_dim(y_major ? image.width() : image.height())
    {
        float angle = atan2(d_min, d_maj);
        thickness = max(_thickness / abs(sin(M_PI / 2 - angle)), 1.0);
        radius = thickness / 2;
        min_ax = max(min_start - radius, 0);
        maj_ax = maj_start;
        max_minor = min(min_ax + thickness - 1, min_dim - 1);
        steps = thickness * (abs(d_maj) + 1);
    };
    int idx()
    {
        return x + y * image.width();
    }

    // Reference to the pixel value at the current position
    reference operator*() const { return image(x, y); }

    pointer operator->() { return &image.at(x, y); }

    // Iterate along the line. After reaching the end, done=IMG_TYPErue and this operator does nothing.
    bool step()
    {
        if (min_ax == max_minor)
        {
            if (maj_ax == maj_end)
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

class set_intersection_iterator
{
public:
    set_intersection_iterator(vector<int> &_set_a, vector<int> &_set_b, int _max_separation = 1, int max_step_power = 9) : set_a(_set_a), set_b(_set_b)
    {
        max_separation = _max_separation;
        max_step_size = pow(2, max_step_power);
        itr_a = set_a.begin();
        itr_b = set_b.begin();
    }
    bool get_next_match(int &match)
    {
        /*
        while(*itr_a < *itr_b && itr_a != set_a.end()) ++itr_a;
        while(*itr_b < *itr_a && itr_b != set_b.end()) ++itr_b;
        if(itr_a == set_a.end() || itr_b == set_b.end()) return false;
        match = *itr_a;
        ++itr_a;
        ++itr_b;
        return true;
        */
        int step_size = max_step_size;
        int min_val_diff;
        while (step_size)
        {
            min_val_diff = max_separation * step_size;
            while (*itr_b - *itr_a > min_val_diff && set_a.end() - itr_a > step_size)
            {
                itr_a += step_size;
            }
            while (*itr_a - *itr_b > min_val_diff && set_b.end() - itr_b > step_size)
            {
                itr_b += step_size;
            }
            step_size /= 2;
        }
        while (*itr_a != *itr_b && itr_a != set_a.end() && itr_b != set_b.end())
        {
            while (*itr_a < *itr_b && itr_a != set_a.end())
                itr_a++;
            while (*itr_b < *itr_a && itr_b != set_b.end())
                itr_b++;
        }
        if (itr_a == set_a.end() || itr_b == set_b.end())
            return false;
        match = *itr_a;
        itr_a++;
        itr_b++;
        return true;
        
    }

private:
    vector<int> &set_a;
    vector<int> &set_b;
    vector<int>::iterator itr_a;
    vector<int>::iterator itr_b;
    int max_separation;
    int max_step_size;
};

class line_intersection_iterator
{
    private:
    coord a_start;
    coord a_end;
    coord b_start;
    coord b_end;

    coord cur_pos;

    //Find the center intersection coordinate of the two lines
    //Source: https://www.topcoder.com/thrive/articles/Geometry%20Concepts%20part%202:%20%20Line%20Intersection%20and%20its%20Applications
    bool intersection(coord& center)
    {
        float a1 = a_end.second - a_start.second;
        float b1 = a_end.first - a_start.first;
        float c1 = a1*a_start.first + b1*a_start.second;
        float a2 = b_end.second - a_start.second;
        float b2 = b_end.first - b_start.first;
        float c2 = a2*b_start.first + b2*b_start.second;
        
        float det = a1 * b2 - a2 * b1;
        if(det == 0) return false;
        center.first = (b2 * c1 - b1 * c2) / det;
        center.second = (a1 * c2 - a2 * c1) / det;
    }

    

    
    public:
    line_intersection_iterator(coord line_a_start, coord line_a_end, coord line_b_start, coord line_b_end)
    {
        a_start = line_a_start;
        a_end = line_a_end;
        b_start = line_b_start;
        b_end = line_b_end;
        cur_pos = a_start;
    }


};

static vector<coord> circular_pins(short center_x, short center_y, short radius, short pin_count)
{
    vector<coord> pins;
    float angle;
    int x, y;
    for (int i = 0; i < pin_count; i++)
    {
        angle = (2.f * 3.14159f * i) / pin_count;
        x = center_x + std::cos(angle) * radius;
        y = center_y + std::sin(angle) * radius;
        pins.push_back(make_pair<short, short>(x, y));
    }
    return pins;
}

template <typename IMG_TYPE>
class line_map
{
    typedef cimg_library::CImg<IMG_TYPE> cimg;
    typedef map<short, map<short, vector<int> *>> lbp;
    typedef map<short, map<short, IMG_TYPE *>> sbp;

public:
    cimg &image;
    int pin_count;
    int line_count;
    line_map(cimg &_image, vector<coord> &pins, int thickness = 1, int min_separation = 1) : image(_image), pin_count(pins.size())
    {
        make_map(pins, thickness, min_separation);
        score_all_lines();
        return;
    }
    ~line_map()
    {
        for (vector<int> *line : line_indices)
        {
            delete line;
        }
    }
    vector<int> &line_between(short pin_a, short pin_b)
    {
        if (lines_by_pin[pin_a].find(pin_b) == lines_by_pin[pin_a].end())
            std::cout << "ERROR: NO LINE BETWEEN " << pin_a << " AND " << pin_b << '\n';
        return *(lines_by_pin[pin_a][pin_b]);
    }

    // Return a list of all lines (as a vector of indices) that end in pin
    map<short, vector<int> *> lines_from_pin(int pin)
    {
        return lines_by_pin[pin];
    }

    // Get the line with the highest score that ends at from_pin
    short best_pin_for(short from_pin, IMG_TYPE &score)
    {
        score = 0;
        short best_pin = -1;
        for (auto b : scores_by_pin[from_pin])
        {
            if (*(b.second) > score)
            {
                score = *(b.second);
                best_pin = b.first;
            }
        }
        return best_pin;
    }
    // Score every line based on its average pixel value
    void score_all_lines()
    {
        for (int i = 0; i < line_count; i++)
        {
            line_scores[i] = calculate_score(i);
        }
        return;
    }
    // Update scores based on an overlapping line
    void update_scores(short pin_a, short pin_b, float multiplier)
    {
        vector<pair<short, short>> lines_to_update = overlapping_lines(pin_a, pin_b);
        vector<int> *line_a = lines_by_pin[pin_a][pin_b];
        vector<int> *line_b = nullptr;
        int idx;
        long new_score;
        //Update the score for each overlapping line
        int update_count = 0;
        for (pair<short, short> p : lines_to_update)
        {
            line_b = lines_by_pin[p.first][p.second];
            set_intersection_iterator sii(*line_a, *line_b, image.width());
            new_score = ((long)*scores_by_pin[p.first][p.second]) * line_b -> size();

            int match_count = 0;
            while (sii.get_next_match(idx))
            {
                new_score += image[idx] * multiplier - image[idx];
                match_count++;
            }
            if(match_count > 0)
            {
                update_count++;
                //std::cout << "\t\t\t" << "Updated " << match_count << " pixels for " << p.first << ',' << p.second << '\n';
            }
            new_score = (new_score > 0) ? new_score / line_b -> size() : 0;
            *scores_by_pin[p.first][p.second] = (IMG_TYPE)new_score;
        }
        std::cout << "\t\t" << update_count << " overlaps updated out of " << lines_to_update.size() << " possible.\n";
        //Update the score for pin_a->pin_b
        new_score = 0;
        for(int idx : *line_a)
        {
            new_score += image[idx] * multiplier;
        }
        new_score = (new_score > 0) ? new_score / line_a -> size() : 0;
        *scores_by_pin[pin_a][pin_b] = (IMG_TYPE)new_score;
        
    }

    bool cull_line(int pin_a, int pin_b)
    {
        if (!(lines_by_pin.find(pin_a) != lines_by_pin.end() && lines_by_pin[pin_a].find(pin_b) != lines_by_pin[pin_a].end()))
            return false;
        std::remove(line_indices.begin(), line_indices.end(), *lines_by_pin[pin_a][pin_b]);
        lines_by_pin[pin_a].erase(pin_b);
        lines_by_pin[pin_b].erase(pin_a);
        line_scores[pin_a].erase(pin_b);
        line_scores[pin_b].erase(pin_a);
        line_count--;
        return true;
    }

private:
    vector<vector<int> *> line_indices;
    vector<IMG_TYPE> line_scores;
    lbp lines_by_pin;
    sbp scores_by_pin;
    void make_map(vector<coord> &pins, int thickness, int min_separation)
    {
        int b_start = 0, b_end = 0;
        int p = pins.size();
        //Some math I made up, calculates the total number of connections.
        line_count = (p*p - 2*p*min_separation + p)/2;
        line_indices = vector<vector<int>*>(line_count);
        line_scores = vector<IMG_TYPE>(line_count);
        int i = 0;
        for (short a = 0; a < pin_count; a++)
        {
            b_start = (a + min_separation);
            b_end = min(pin_count, (pin_count + a - min_separation)+1);
            for (short b = b_start; b < b_end; b++)
            {
                line_iterator<IMG_TYPE> li(image, pins[a].first, pins[a].second, pins[b].first, pins[b].second, thickness);
                line_indices[i] = new vector<int>();

                while (li.step())
                {
                    line_indices[i]->push_back(li.idx());
                }
                std::sort(line_indices[i]->begin(), line_indices[i]->end());
                line_scores[i] = 0;
                lines_by_pin[a][b] = line_indices[i];
                lines_by_pin[b][a] = lines_by_pin[a][b];
                scores_by_pin[a][b] = &(line_scores[i]);
                scores_by_pin[b][a] = scores_by_pin[a][b];

                i++;
            }
        }

    }
    vector<pair<short, short>> overlapping_lines(short pin_a, short pin_b)
    {
        vector<pair<short, short>> lines;
        short min_pin = min(pin_a,pin_b);
        short max_pin = max(pin_a,pin_b);
        for(auto a : lines_by_pin)
        {
            for(auto b : a.second)
            {
                if((a.first < min_pin && b.first > min_pin && b.first < max_pin) ||
                   (a.first > min_pin && a.first < max_pin && b.first > max_pin))
                {
                    lines.push_back(make_pair<short,short>((short)a.first,(short)b.first));
                }
            }
        }

        return lines;
    }
    IMG_TYPE calculate_score(short pin_a, short pin_b)
    {
        return calculate_score(*lines_by_pin[pin_a][pin_b]);
    }
    IMG_TYPE calculate_score(int line_index)
    {
        return calculate_score(*line_indices[line_index]);
    }
    IMG_TYPE calculate_score(vector<int>& indices)
    {
        long sum = 0;
        for (int idx : indices)
        {
            sum += image[idx];
        }
        return (IMG_TYPE)(sum / indices.size());
    }
};
#endif
