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

typedef map<int,   map<int, short>> linescore;
typedef map<pair<short, short>, vector<int>> line_map;
namespace image_analysis
{
    class set_intersection_iterator
    {
        public:
        set_intersection_iterator(vector<int>& _set_a, vector<int>& _set_b, int _max_separation = 1, int max_step_power = 9) : set_a(_set_a), set_b(_set_b)
        {
            max_separation = _max_separation;
            max_step_size = pow(2, max_step_power);
            itr_a = set_a.begin();
            itr_b = set_b.begin();
        }
        bool get_next_match(int& match)
        {
            int step_size = max_step_size;
            int min_val_diff;
            while(step_size)
            {
                min_val_diff = max_separation * step_size;
                while(*itr_b - *itr_a > min_val_diff && set_a.end() - itr_a > step_size)
                {
                    itr_a += step_size;
                }
                while(*itr_a - *itr_b > min_val_diff && set_b.end() - itr_b > step_size)
                {
                    itr_b += step_size;
                }
                step_size /= 2;
            }
            while(*itr_a != *itr_b && itr_a != set_a.end() && itr_b != set_b.end())
            {
                while(*itr_a < *itr_b && itr_a != set_a.end()) itr_a++;
                while(*itr_b < *itr_a && itr_b != set_b.end()) itr_b++;
            }
            if(itr_a == set_a.end() || itr_b == set_b.end()) return false;
            match = *itr_a;
            itr_a++;
            itr_b++;
            return true;
        }
        private:
        vector<int> & set_a;
        vector<int> & set_b;
        vector<int>::iterator itr_a;
        vector<int>::iterator itr_b;
        int max_separation;
        int max_step_size;
    };

    struct score_update
    {
        int scored_a;
        int scored_b;
        int overlap_a;
        int overlap_b;
    };

    void add_to_updates(int newline_a, int newline_b, linescore& line_scores, map<int, vector<score_update>>& to_update)
    {
        for(auto a = line_scores.begin(); a != line_scores.end(); a++)
        {
            for(auto b = a->second.begin(); b != a->second.end(); b++)
            {
                if ((a->first < newline_a && b->first < newline_b) || (a->first > newline_a && b->first > newline_b))
                {
                    to_update[a->first].push_back({a->first, b->first, newline_a, newline_b});
                }
            }
        }
    }

    template <typename T>
    void score_pin_from_updates(CImg<T>& img, vector<score_update>& to_update, linemap& line_map, linescore& line_scores, float multiplier)
    {
        int cur_idx;
        long new_score;
        for(score_update su : to_update)
        {
            new_score = line_scores[su.scored_a][su.scored_b] * line_map[su.scored_a][su.scored_b].size();
            set_intersection_iterator sti(line_map[su.scored_a][su.scored_b], line_map[su.overlap_a][su.overlap_b], img.width());
            std::cout << "\tScoring line " << su.scored_a << ',' << su.scored_b << ", overlapped by " << su.overlap_a << ',' << su.overlap_b << '\n';
            std::cout << "\tSoring line size: " << line_map[su.scored_a][su.scored_b].size() << ", overlap size: " << line_map[su.overlap_a][su.overlap_b].size() << '\n';
            while(sti.get_next_match(cur_idx))
            {
                new_score += img[cur_idx] * multiplier - img[cur_idx];
            }
            new_score /= line_map[su.scored_a][su.scored_b].size();
            line_scores[su.scored_a][su.scored_b] = new_score;
        }
    }

    void make_line_map(int pins, int thickness, int resolution, int minimum_separation, linemap& out_map)
    {
        CImg<u_char> img(resolution, resolution, 1,1);
        vector<int>* cur_line;
        int x_a, y_a, x_b, y_b, i;
        float angle_a, angle_b;
        out_map.clear();
        for(int a = 0; a < pins; a++)
        {
            angle_a = (a * 2 * M_PI / pins);
            x_a = std::cos(angle_a) * (resolution/2 - 5) + resolution/2;
            y_a = std::sin(angle_a) * (resolution/2 - 5) + resolution/2;
            for(int b = a+minimum_separation; b < pins; b++)
            {
                if((b + minimum_separation < pins) || (b + minimum_separation - pins < a))
                {
                    angle_b = b  *2 * M_PI / pins;
                    x_b = std::cos(angle_b) * (resolution/2 - 5) + resolution/2;
                    y_b = std::sin(angle_b) * (resolution/2 - 5) + resolution/2;
                    image_analysis::line_iterator<u_char> li(img,x_a,y_a,x_b,y_b,thickness);
                    out_map[a][b] = vector<int>(li.steps);
                    cur_line = &out_map[a][b];
                    i = 0;
                    while(!li.done)
                    {
                        (*cur_line)[i] = li.x + li.y * resolution;
                        ++li;
                        ++i;
                    }
                    std::sort(cur_line->begin(), cur_line->end());
                }
            }
        }
    };

    template<typename T>
    T average_along_line(CImg<T>& image, int start_x, int start_y, int end_x, int end_y)
    {
        line_iterator li(image, start_x, start_y, end_x, end_y);
        long sum = 0;
        while(!li.done)
        {
            sum += *li;
            ++li;
        }
        return (T)(sum / li.steps);
    };

    template<typename T>
    T average_along_line(CImg<T>& image, vector<int>& line)
    {
        long sum = 0;
        for(int idx : line)
        {
            sum += image[idx];
        }
        return (T)(sum / line.size());
    };

    /*
    template<typename T>
    T update_score(CImg<T>& img, int score_a, int score_b, int overlap_a, int overlap_b, float multiplier, linescore& line_scores, linemap& line_map, T cull_threshold = 0)
    {
        vector<int>& score_line = line_map[score_a][score_b];
        vector<int>& overlap_line = line_map[overlap_a][overlap_b];
        long score_line_size = score_line.size();
        long old_score = line_scores[score_a][score_b];
        long new_score = old_score * score_line_size;
        int cur_idx;

        //This is a modified version of std::set_intersection that immediately uses each element instead of copying it to a vector.
        set_intersection_iterator sti(score_line, overlap_line, img.width());
        //std::cout << score_a << ',' << score_b << "->" << overlap_a << ',' << overlap_b << '\n';
        while(sti.get_next_match(cur_idx))
        {
            new_score += img[cur_idx]*multiplier - img[cur_idx]; 
        }

        new_score /= score_line_size;
        if(new_score > cull_threshold)
        {
            line_scores[score_a][score_b] = (T)new_score;
            return (T)new_score;
        }
        else if(score_b != line_scores[score_a].begin()->first)
        {    
            line_scores[score_a].erase(score_b);
            if(line_scores[score_a].empty()) line_scores.erase(score_a);
        }
        return 0;

    };
    */

    template<typename T>
    void score_all_lines(CImg<T>& image, linemap& line_map, linescore& line_scores, T cull_threshold = 0)
    {
        T score = 0;
        vector<int> to_cull;

        int erase_count = 0;
        int total_count = 0;
        for(auto a : line_map)
        {
            to_cull.clear();
            for(auto b : a.second)
            {
                score = average_along_line(image, b.second);
                if(score > cull_threshold)
                {
                    line_scores[a.first][b.first] = score;
                }
                else
                {
                    to_cull.push_back(b.first);
                    erase_count++;
                }
                total_count++;
            }
            for(int c : to_cull)
            {
                std::cout << "Culling " << a.first << ',' << c << '\n';
                line_map[a.first].erase(c);
                line_scores[a.first].erase(c);
            }
            if(line_map[a.first].empty())
            {
                std::cout << "Culling " << a.first << ',' << c << '\n';
                line_map.erase(a.first);
                line_scores.erase(a.first);
            }
        }
        std::cout << "Lines erased: " << erase_count << "(" << 100 * ((float)erase_count / total_count) << "%)" << '\n';
    };

    template<typename T>
    T get_best_score(linescore& line_scores, int& pin_a, int& pin_b)
    {
        T best_score = 0;
        for(auto a : line_scores)
        {
            for(auto b : a.second)
            {
                if(b.second > best_score)
                {
                    best_score = b.second;
                    pin_a = a.first;
                    pin_b = b.first;
                }
            }
        }
        return best_score;
    };

    template<typename T>
    int get_next_pin(linescore& line_scores, int from_pin, T& score)
    {
        score = 0;
        T cur_score = -1;
        int to_pin = -1;
        auto a = line_scores.begin();
        auto b = a -> second.begin();
        while(a -> first < from_pin && a != line_scores.end())
        {
            b = a -> second.find(from_pin);
            if(b != a -> second.end())
            {
                cur_score = b -> second;
                if(cur_score > score)
                {
                    score = cur_score;
                    to_pin = a -> first;
                }
            }
            ++a;
        }
        b = line_scores[from_pin].begin();
        while(b != line_scores[from_pin].end())
        {
            cur_score = b -> second;
            if(cur_score > score)
            {
                score = cur_score;
                to_pin = b -> first;
            }
            ++b;
        }
        return to_pin;
    };
}


#endif
