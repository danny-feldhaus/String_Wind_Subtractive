#include "string_art.hpp"

template <class IMG_TYPE>
string_art<IMG_TYPE>::string_art(const char* _image_file, const short _resolution,const short _pin_count, float _pin_radius, short _min_separation, u_char _score_method, float _score_modifier) 
:   
    rgb_image(make_rgb_image(_image_file, _resolution)), 
    lab_image(rgb_image.get_shared_channels(0,2).get_RGBtoLab()),
    darkness_image(make_darkness_image(rgb_image)),
    pin_count(_pin_count), 
    pins(circular_pins(rgb_image, _pin_radius, _pin_count)),
    score_method(_score_method),
    score_modifier(_score_modifier),
    line_count(calculate_line_count(_pin_count, _min_separation))
{   
    #ifdef DEBUG
        #ifdef DEBUG_PATH_STEPS
        po.add_int("Step",1,0);
        #endif
        #ifdef DEBUG_SCORING
            po.add_flt("Score", 0, 2, 1);
        #endif
        #ifdef DEBUG_TIMING
            po.add_flt("Cur Steps Per Second", 0, 1, 2);
            po.add_flt("Avg Steps Per Second", 0, 1, 2);
            po.add_flt("Local Avg Steps Per Second", 0, 1, 2);
                po.add("\% Updating", "0%", 2);
                po.add("\% Getting Score", "0%", 2);
                po.add("\% Other", "0%", 2);

        #endif
    #endif
    build_lines(_min_separation);
    string_image = tcimg(rgb_image.width(), rgb_image.height(), 1, 1, 0);
    score_all_lines();
    return;
}

template <class IMG_TYPE>
string_art<IMG_TYPE>::~string_art()
{
    delete[] pins;
    delete[] line_scores;
    delete[] line_pairs;
}

template <class IMG_TYPE>
short* string_art<IMG_TYPE>::generate(const short path_steps)
{
    if(path_steps < 2)throw std::domain_error("Number of steps is out of range (" + std::to_string(path_steps) + ")");
    #if defined(DEBUG)
        po.reset_progress(path_steps);
        #if defined(DEBUG_TIMING)
            std::deque<float> last_10_sps;
            float runtime_seconds = 0.f;
        #endif
        #if defined(DEBUG_OVERLAP)
            cimg_library::CImg_Display cd;
        #endif
    #endif
    short* path = new short[path_steps];
    IMG_TYPE score = 0;

    path[0] = best_pin();

    for(int i = 1; i < path_steps; i++)
    {
        #if defined(DEBUG) && defined(DEBUG_TIMING)
        auto start = high_resolution_clock::now();
        #endif

        path[i] = best_pin_for(path[i-1], score, DEPTH);
        update_scores(path[i], path[i-1]);
        #if defined (DEBUG) && defined(DEBUG_PATH_STEPS)
            po.update_progress(i+1);
            po.set_int("Step", i);
            po.set_flt("Score", score, 2);
            #ifdef DEBUG_TIMING
                auto stop = high_resolution_clock::now();
                float step_time = duration_cast<microseconds>(stop - start).count() / 1000000.f;
                runtime_seconds += step_time;
                last_10_sps.push_front(1.0f / step_time);
                if(i > 10) last_10_sps.pop_back();
                po.set_flt("Cur Steps Per Second",  last_10_sps.back(), 2);
                po.set_flt("Avg Steps Per Second", (i / runtime_seconds));
                po.set_flt("Local Avg Steps Per Second", std::accumulate(last_10_sps.begin(), last_10_sps.end(),0.0f) / last_10_sps.size(), 2);
                po.set_percent("\% Time Updating",  update_time / step_time, 1, true);
                po.set_percent("\% Time Getting Score", getscore_time / step_time, 1, true);
                po.set_percent("\% Other", 1.f - (getscore_time + update_time)/step_time, 1, true);
            #endif
            std::cout << po.to_string();
        #endif
    }
    #ifdef DEBUG_PATH_STEPS
    std::cout << po.clear();
    #endif
    return path;
}

template <class IMG_TYPE>
bool string_art<IMG_TYPE>::save_string_image(const char* image_file, const bool append_debug_info)
{
    tcimg save_img = (255 - string_image.get_normalize(0,255));
    if(append_debug_info) save_img.append((SCORE_RESOLUTION - darkness_image)/((float)SCORE_RESOLUTION / 255));
    save_img.save(image_file);
    return true;
}

template <class IMG_TYPE>
short string_art<IMG_TYPE>::best_pin()
{
    IMG_TYPE best_score = 0;
    short best_pin_a = -1;
    short best_pin_b = -1;
    IMG_TYPE cur_score = 0;
    IMG_TYPE avg_a = 0, avg_b = 0;
    for(int i=0; i < line_count; i++)
    {
        if(line_pairs[i] != -1)
        {
            cur_score = *line_scores_by_pin[line_pairs[i].x][line_pairs[i].y];
            if(cur_score > best_score)
            {
                best_score = cur_score;
                best_pin_a = line_pairs[i].x;
                best_pin_b = line_pairs[i].y;
            }
        }
    }
    assert(best_pin_a != -1);

    best_pin_for(best_pin_a, avg_a, DEPTH);
    best_pin_for(best_pin_b, avg_b, DEPTH);
    if(avg_a > avg_b) 
        return best_pin_a;
    else 
        return best_pin_b;
}

template <class IMG_TYPE>
short string_art<IMG_TYPE>::best_pin_for(short from_pin, IMG_TYPE& score, short depth)
{
    #if defined(DEBUG) && defined(DEBUG_TIMING)
        auto start = high_resolution_clock::now();
    #endif
    short to_pin = 0;
    vector<short> visited{from_pin};
    score = best_score_for(from_pin, depth, to_pin, visited);
    #if defined(DEBUG) && defined(DEBUG_TIMING)
        auto stop = high_resolution_clock::now();
        getscore_time = duration_cast<microseconds>(stop - start).count() / 1000000.f;
    #endif
    return to_pin;
}

template <class IMG_TYPE>    
void string_art<IMG_TYPE>::score_all_lines()
{
    vector<int> to_cull;
    for(int i=0; i < line_count; i++)
    {
        *line_scores_by_pin[line_pairs[i].x][line_pairs[i].y] = initial_score(line_pairs[i].x,line_pairs[i].y);
        if(*line_scores_by_pin[line_pairs[i].x][line_pairs[i].y] < SCORE_RESOLUTION * 0.01f)
        {
            to_cull.push_back(i);
        }
    }
    for(int c : to_cull)
    {
        cull_line(c);
    }
    return;
}

template <class IMG_TYPE>
void string_art<IMG_TYPE>::update_scores(short pin_a, short pin_b)
{
    #if defined(DEBUG) && defined(DEBUG_TIMING)
        auto start = high_resolution_clock::now();
    #endif
    scoord cur_coord;
    old_string_image.assign(string_image);
    draw_line(string_image, pin_a, pin_b, SCORE_RESOLUTION);
    const short min_pin = min(pin_a,pin_b);
    const short max_pin = max(pin_a,pin_b);
    #pragma omp parallel for
    for(int i=0; i < line_count; i++)
    {
        if(line_pairs[i].x != -1)
        {
            if((line_pairs[i].x < min_pin && line_pairs[i].y > min_pin && line_pairs[i].y < max_pin) ||
            (line_pairs[i].x > min_pin && line_pairs[i].x < max_pin && line_pairs[i].y > max_pin))
            {
                update_score(line_pairs[i].x, line_pairs[i].y, pin_a, pin_b, old_string_image, string_image);
            }
        }
    }
    if((score_method == 0) || (score_method == 2))
    {
        darken_line(darkness_image, pin_a, pin_b);
    }
    *line_scores_by_pin[pin_a][pin_b] = 0;
    #if defined(DEBUG) && defined(DEBUG_TIMING)
        auto stop = high_resolution_clock::now();
        update_time = duration_cast<microseconds>(stop - start).count() / 1000000.f;
    #endif
}

template <class IMG_TYPE>
IMG_TYPE string_art<IMG_TYPE>::update_score(const short scored_a, const short scored_b, const short overlap_a, const short overlap_b, tcimg& old_string_image, tcimg& new_string_image)
{
    float line_length = line_lengths[scored_a][scored_b];
    float new_score;
    float cur_score;
    float local_length = 0;
    switch(score_method)
    {
        case 0: default:
        {
            line_intersection_iterator<IMG_TYPE> lii(darkness_image, pins[scored_a], pins[scored_b], pins[overlap_a], pins[overlap_b], 3);

            new_score = ((long)*line_scores_by_pin[scored_a][scored_b]) * line_length;
            do
            {
                cur_score = lii.get();
                if(cur_score > 0.01f)
                {
                    new_score -= cur_score;
                    new_score += cur_score * score_modifier;   
                    local_length ++;     
                }        
            }while(lii.step());
            new_score /= line_length;
            break;
        }
        case 1: 
        {
            line_intersection_iterator<IMG_TYPE> lii(darkness_image, pins[scored_a], pins[scored_b], pins[overlap_a], pins[overlap_b], 3);

            std::deque<scoord> line_coords(3);
            new_score = ((long)*line_scores_by_pin[scored_a][scored_b]) * line_length;
            //Assign the first three pixels in the line
            line_coords[0] = lii.cur_coord();
            lii.step();
            line_coords[1] = lii.cur_coord();
            lii.step();
            line_coords[2] = lii.cur_coord();
            //Iterate through the rest of the line
            do
            {
                new_score -= score_square(old_string_image, line_coords);
                new_score += score_square(new_string_image, line_coords);
                line_coords.pop_front();
                line_coords.push_back(lii.cur_coord());
            }while(lii.step());
            new_score /= line_length;
            break;
        }
        case 2: //Root mean square error
            line_intersection_iterator<IMG_TYPE> lii(darkness_image, pins[scored_a], pins[scored_b], pins[overlap_a], pins[overlap_b], 3);

            new_score = pow(*line_scores_by_pin[scored_a][scored_b],2) *  line_length;
            do
            {
                cur_score = lii.get();
                if(cur_score != 0)
                {
                    local_length++;
                    new_score -= pow(cur_score,2);
                    new_score += pow(cur_score * score_modifier,2);
                }
            }while(lii.step());
            new_score = sqrt(new_score / line_length);
            break;
    }
    *line_scores_by_pin[scored_a][scored_b] = (IMG_TYPE)new_score;
    return new_score;
}

template <class IMG_TYPE>
void string_art<IMG_TYPE>::build_lines(short min_separation)
{
    int b_start = 0, b_end = 0;
    line_scores = new IMG_TYPE[line_count];
    line_pairs = new scoord[line_count];
    int i = 0;
    for (short a = 0; a < pin_count; a++)
    {
        b_start = (a + min_separation);
        b_end = min(pin_count, (short)((pin_count + a - min_separation)+1));
        for (short b = b_start; b < b_end; b++)
        {
            line_scores[i] = 0;
            line_scores_by_pin[a][b] = &(line_scores[i]);
            line_scores_by_pin[b][a] = line_scores_by_pin[a][b];
            
            line_pairs[i].x = a;
            line_pairs[i].y = b;
            i++;
        }
    }
}

template <class IMG_TYPE>
vector<coord<short>> string_art<IMG_TYPE>::overlapping_lines(short pin_a, short pin_b)
{
    vector<scoord> lines;
    short min_pin = min(pin_a,pin_b);
    short max_pin = max(pin_a,pin_b);
    for(int i=0; i < line_count; i++)
    {
        if(line_pairs[i].x != -1)
        {
            if((line_pairs[i].x < min_pin && line_pairs[i].y > min_pin && line_pairs[i].y < max_pin) ||
            (line_pairs[i].x > min_pin && line_pairs[i].x < max_pin && line_pairs[i].y > max_pin))
            {
                    lines.push_back(line_pairs[i]);
            }
        }
    }
    return lines;
}

template <class IMG_TYPE>
std::string string_art<IMG_TYPE>::ASCII_line(const short from_pin,const short to_pin,const short resolution)
{
    std::string text = "";
    cimg_library::CImg<u_char> line_map(resolution, (darkness_image.height() * resolution) / darkness_image.width(), 1, 1, false);
    scoord scaled_from_pin = ((coord<int>)pins[from_pin] * resolution) / darkness_image.width();
    scoord scaled_to_pin   = ((coord<int>)pins[to_pin]   * resolution) / darkness_image.width();
    line_iterator<u_char> li(line_map, scaled_from_pin, scaled_to_pin, false);
    scoord cur_coord;
    const std::string red = "\033[31m";
    const std::string blue = "\033[34m";
    const std::string green = "\033[32m";
    const std::string cclose = "\033[0m";
    float r = (float)(pins[0].x - darkness_image.width()/2) / (darkness_image.width()/2);
    float max_da = abs(atan2(resolution/2-1,resolution/2) - atan2(resolution/2,resolution/2-1));
    int x, y;
    for(float a = 0; a < 2 * M_PI; a += max_da)
    {
        x = cos(a) * r * line_map.width()/2  + line_map.width()/2;
        y = sin(a) * r * line_map.height()/2 + line_map.height()/2;
        line_map(x,y) = 1;
    }
    
    do
    {
        li.set(2);
    }while(li.step());
    line_map(scaled_from_pin.x,scaled_from_pin.y) = 3;
    line_map(scaled_to_pin.x  ,scaled_to_pin.y) = 4;
    for(short y = line_map.height()-1; y >= 0; y--)
    {
        for(short x = 0; x < line_map.width(); x++)
        {
            switch(line_map(x,y))
            {
                case 0:
                    text += "  ";
                    break;
                case 1:
                    text += "* ";
                    break;
                case 2:
                    text += blue + "* " + cclose;
                    break;
                case 3:
                    text += green + "S " + cclose;
                    break;
                case 4:
                    text += red + "E " + cclose;
                    break;
            }
        }
        text += '\n';
    }
    return text;
}

template <class IMG_TYPE>
IMG_TYPE string_art<IMG_TYPE>::initial_score(const short pin_a, const short pin_b)
{
    float cur_score = 0;
    float score = 0;
    line_iterator<IMG_TYPE> li(darkness_image, pins[pin_a].x, pins[pin_a].y, pins[pin_b].x, pins[pin_b].y, false);
    float masked_length = 0;
    switch(score_method)
    {
        case 0: default:
        {
            do
            {
                cur_score = li.get();
                if(cur_score != 0)
                {
                    score += cur_score;
                    masked_length ++;
                }
            } while (li.step());
            score /= masked_length;
            break;
        }
        case 1:
        {
            std::deque<scoord> line_coords(3);
            coord bot_left, top_right;
            line_coords[0] = li.cur_coord();
            li.step();
            line_coords[1] = li.cur_coord();
            li.step();
            line_coords[2] = li.cur_coord();
            while(li.step())
            {
                score += score_square(string_image, line_coords);
                masked_length ++;
                line_coords.pop_front();
                line_coords.push_back(li.cur_coord());
            } 
            score /= masked_length;
            break;
        }
        case 2: //RMSE
        {
            do
            {
                cur_score = li.get();
                if(cur_score != 0)
                {
                    score += pow(cur_score,2);
                    masked_length ++;
                }
            } while (li.step());
            score = sqrt(score / masked_length);
            break;
        }
    }
    line_lengths[pin_a][pin_b] = masked_length;
    line_lengths[pin_b][pin_a] = masked_length;
    return score;
}

template <class IMG_TYPE>
IMG_TYPE string_art<IMG_TYPE>::score_square(const tcimg& string_image_ref, std::deque<scoord>& line_coords) const
{
    scoord bot_left, top_right, cur_coord;
    int img_sum = 0;
    int line_sum = 0;
    int new_line_sum = 0;
    int square_area = 9;
    bot_left.x = line_coords[1].x - 1;
    bot_left.y = line_coords[1].y - 1;
    top_right.x = line_coords[1].x + 1;
    top_right.y = line_coords[1].y + 1;
    for(cur_coord.x = bot_left.x; cur_coord.x <= top_right.x; cur_coord.x++)
    {
        for(cur_coord.y = bot_left.y; cur_coord.y <=  top_right.y; cur_coord.y++)
        {
            img_sum += darkness_image(cur_coord.x, cur_coord.y);
            //Add to the string score if the point is on the string map, or if it's in the given list of coords.
            if(string_image_ref(cur_coord.x, cur_coord.y))
            {
                line_sum += SCORE_RESOLUTION;
            }
            else
            {
                for(scoord c : line_coords)
                {
                    if(c == cur_coord)
                    {
                        new_line_sum += SCORE_RESOLUTION;
                        break;
                    }
                }
            }
        }
    }
    //The average value of the image in this square
    int image_score = img_sum / square_area;
    //The difference of the string img (before the current line) and the image (small = good)
    int existing_score = abs((line_sum / square_area) - image_score);
    //The difference of the string img (after the current line) and the image (small = good)
    int potential_score = abs((line_sum+new_line_sum)/square_area - image_score);
    //The improvement in similarity after adding a line (big = good)
    int score = existing_score - potential_score;
    return score + SCORE_RESOLUTION; //just avoiding negatives here.
}

template <class IMG_TYPE>
void string_art<IMG_TYPE>::darken_line(tcimg& image, const short pin_a,const short pin_b)
{
    line_iterator<IMG_TYPE> li(image, pins[pin_a].x, pins[pin_a].y, pins[pin_b].x, pins[pin_b].y, true);
    do
    {
        li.set(li.get() * score_modifier);
    } while (li.step());
    return;
}

template <class IMG_TYPE>
void string_art<IMG_TYPE>::draw_line(tcimg& image, short pin_a, short pin_b, IMG_TYPE color)
{
    line_iterator<IMG_TYPE> li(image, pins[pin_a].x, pins[pin_a].y, pins[pin_b].x, pins[pin_b].y, false);
    do{
        li.set(color);
    } while (li.step());
    return;
}

template <class IMG_TYPE>
void string_art<IMG_TYPE>::cull_line(const int line_index)
{
    line_scores_by_pin[line_pairs[line_index].x].erase(line_pairs[line_index].y);
    line_scores_by_pin[line_pairs[line_index].y].erase(line_pairs[line_index].x);
    line_pairs[line_index].y = -1;

}


//Return the best-scoring next pin.
template <class IMG_TYPE>
IMG_TYPE string_art<IMG_TYPE>::best_score_for(short from_pin, short depth, short& to_pin, vector<short>& visited)
{
    if(depth == 0) return 0;
    short best_pin = -1;
    IMG_TYPE best_score = 0;
    IMG_TYPE cur_score  = 0;
    
    for(auto b : line_scores_by_pin[from_pin])
    {   
        for(short v : visited)
        {
            if(b.first == v) continue;
        }
        if(*b.second > 0)
        {
            visited.push_back(b.first);
            cur_score = *(b.second) + best_score_for(b.first, depth-1, to_pin, visited);
            visited.pop_back();
            if(cur_score > best_score)
            {
                best_pin = b.first;
                best_score = cur_score;
            }
        }
    }
    //If no suitable neighbor was found, return the nearest neighbor
    if(best_pin == -1)
    {
        auto b = line_scores_by_pin[from_pin].begin();
        while(b -> first < from_pin && b != line_scores_by_pin[from_pin].end()) ++b;
        if(b == line_scores_by_pin[from_pin].end()) --b;
        best_pin = b -> first;
    }
    to_pin = best_pin;
    return best_score;
}

template <typename IMG_TYPE>
coord<short>* string_art<IMG_TYPE>::circular_pins(const tcimg& image, float radius, short pin_count)
{
    float angle;
    scoord *pins = new scoord[pin_count];
    scoord center(image.width()/2, image.height()/2);
    for (int i = 0; i < pin_count; i++)
    {
        angle = (2.f * 3.14159f * i) / pin_count;
        pins[i].x = center.x + std::cos(angle) * radius * image.width() /2;
        pins[i].y = center.y + std::sin(angle) * radius * image.height()/2;
    }
    return pins;
}

/**
 * @details  If you map the possible connections on a 2D grid, they can be re-organized into a rectangle and a triangle.
 * This sums the areas of those two shapes. <br>
 * \f{eqnarray*}
 *   & \textrm{Rectangle area:} & (c-2s+1) \times s \\
 *   & \textrm{Triangle area:}  & \frac{ (c-2s) \times(c-2s+1)}{2} \\
 *   & \textrm{Total area:}     & \frac{c^2-2cs + c}{2}
 * \f}
 */
template <typename IMG_TYPE>
short string_art<IMG_TYPE>::calculate_line_count(short pin_count, short min_separation)
{
    return (pin_count*pin_count - 2*pin_count*min_separation + pin_count)/2;
}

template <typename IMG_TYPE>
cimg_library::CImg<IMG_TYPE> string_art<IMG_TYPE>::make_rgb_image(const char* image_file, short resolution)
{
    tcimg rgb_image(image_file);
    rgb_image.resize(resolution, resolution * rgb_image.width() / rgb_image.height(), 1, rgb_image.spectrum());    

    return rgb_image;
}

template <typename IMG_TYPE>
cimg_library::CImg<IMG_TYPE> string_art<IMG_TYPE>::make_darkness_image(const tcimg& rgb_image)
{
    tcimg b_w =255 - (0.299*rgb_image.get_shared_channel(0) + 
                      0.587*rgb_image.get_shared_channel(1) + 
                      0.114*rgb_image.get_shared_channel(2));
    if(rgb_image.spectrum() == 4)
    {
        cimg_forXY(b_w,x,y)
        {
            if(rgb_image(x,y,0,3) < 1)
            {
                b_w(x,y) = 0;
            }
        }
    }

    //Set bounds in some un-used pixels, so that normalizing doesn't stretch dark gray to black / light gray to white.
    b_w[0] = 0;
    b_w[1] = 255;
    b_w.normalize(0,SCORE_RESOLUTION);
    return b_w;
}

template class string_art<short>;
template class string_art<int>;
template class string_art<float>;