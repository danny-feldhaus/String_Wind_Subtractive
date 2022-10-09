#include "string_art.hpp"

template <class IMG_TYPE>
string_art<IMG_TYPE>::string_art(const char* _image_file, const short _resolution,const short _pin_count, float _pin_radius, short _min_separation, u_char _score_method, float _score_modifier) 
:   rgb_image(make_rgb_image(_image_file, _resolution)), 
    lab_image(rgb_image.get_RGBtoLab()),
    darkness_image(make_darkness_image(rgb_image)),
    pin_count(_pin_count), 
    pins(circular_pins(rgb_image, _pin_radius, _pin_count)),
    score_method(_score_method),
    score_modifier(_score_modifier),
    line_count(calculate_line_count(_pin_count, _min_separation))
{
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

    short* path = new short[path_steps];
    IMG_TYPE score = 0;
    #ifdef DEBUG_PATH_STEPS
        for(int i = 0; i < 2; i++) std::cout << '\n';
    #endif
    path[0] = best_pin();
    for(int i = 1; i < path_steps; i++)
    {
        path[i] = best_pin_for(path[i-1], score, DEPTH);
        update_scores(path[i], path[i-1]);
        #ifdef DEBUG_PATH_STEPS
            std::cout << "\033[" << 2 << "A";
            std::cout << "Step " << i << ": " << path[i] << "\tscore: " << score << '\n';
            float percent_done = 100 * (float)i / path_steps;
            std::cout << '|' << std::fixed << std::setprecision(1);
            for(int i = 0; i < percent_done; i++)
            {
                std::cout << '=';
            }
            for(int i = percent_done; i < 100; i++)
            {
                std::cout << '-';
            }
            std::cout << "| " << percent_done << "\% done\n";
            /*
            if(cd.is_keySPACE())
            {
                cd = (SCORE_RESOLUTION - string_image.get_resize(512,512)).append(darkness_image.get_resize(512,512));
            }
            cd.show();
            */
            //std::cout << "OPS: " << cd.frames_per_second() << " operations per-second\n";
            //std::cout << ASCII_line(path[i],path[i-1], 40);
            //estd::cout  << "\033[" << 2  << "A";
        #endif
    }
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
    short best_score = 0;
    short best_pin_a = -1;
    short best_pin_b = -1;
    short cur_score = 0;
    IMG_TYPE avg_a = 0, avg_b = 0;
    for(int i=0; i < line_count; i++)
    {
        cur_score = *line_scores_by_pin[line_pairs[i].x][line_pairs[i].y];
        if(cur_score > best_score)
        {
            best_score = cur_score;
            best_pin_a = line_pairs[i].x;
            best_pin_b = line_pairs[i].y;
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
    short to_pin = 0;
    vector<short> visited{from_pin};
    score = best_score_for(from_pin, depth, to_pin, visited);
    return to_pin;
}

template <class IMG_TYPE>    
void string_art<IMG_TYPE>::score_all_lines()
{
    for(int i=0; i < line_count; i++)
    {
        *line_scores_by_pin[line_pairs[i].x][line_pairs[i].y] = initial_score(line_pairs[i].x,line_pairs[i].y);
    }
    return;
}

template <class IMG_TYPE>
void string_art<IMG_TYPE>::update_scores(short pin_a, short pin_b)
{
    vector<scoord> lines_to_update = overlapping_lines(pin_a, pin_b);
    scoord cur_coord;
    old_string_image.assign(string_image);
    draw_line(string_image, pin_a, pin_b, SCORE_RESOLUTION);
    for (scoord p : lines_to_update)
    {
        update_score(p.x, p.y, pin_a, pin_b, old_string_image, string_image);
    }
    if((score_method == 0) || (score_method == 2))
    {
        darken_line(darkness_image, pin_a, pin_b);
    }
    *line_scores_by_pin[pin_a][pin_b] = 0;
}

template <class IMG_TYPE>
IMG_TYPE string_art<IMG_TYPE>::update_score(const short scored_a, const short scored_b, const short overlap_a, const short overlap_b, tcimg& old_string_image, tcimg& new_string_image)
{
    long line_length = distance(pins[scored_a],pins[scored_b]);
    long new_score;
    long cur_score;
    switch(score_method)
    {
        case 0: default:
        {
            line_intersection_iterator<IMG_TYPE> lii(darkness_image, pins[scored_a], pins[scored_b], pins[overlap_a], pins[overlap_b], 3);

            new_score = ((long)*line_scores_by_pin[scored_a][scored_b]) * line_length;
            do
            {
                cur_score = lii.get();
                new_score -= cur_score;
                new_score += cur_score * score_modifier;                
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
                //Update the un-rolled average by removing the old score, and adding the new one.
                new_score -= score_square(old_string_image, line_coords);
                new_score += score_square(new_string_image, line_coords);              
                line_coords.pop_front();
                line_coords.push_back(lii.cur_coord());
            }while(lii.step());
            break;
        }
        case 2: //Root mean square error
            line_intersection_iterator<IMG_TYPE> lii(darkness_image, pins[scored_a], pins[scored_b], pins[overlap_a], pins[overlap_b], 3);

            new_score = pow(*line_scores_by_pin[scored_a][scored_b],2) *  line_length;
            do
            {
                cur_score = lii.get();
                new_score -= pow(cur_score,2);
                new_score += pow(cur_score * score_modifier,2);
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
        if((line_pairs[i].x < min_pin && line_pairs[i].y > min_pin && line_pairs[i].y < max_pin) ||
           (line_pairs[i].x > min_pin && line_pairs[i].x < max_pin && line_pairs[i].y > max_pin))
        {
                lines.push_back(line_pairs[i]);
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
    long score = 0;
    line_iterator<IMG_TYPE> li(darkness_image, pins[pin_a].x, pins[pin_a].y, pins[pin_b].x, pins[pin_b].y, false);

    switch(score_method)
    {
        case 0: default:
        {
            do
            {
                score += li.get();
            } while (li.step());
            score /= li.line_length;
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
                line_coords.pop_front();
                line_coords.push_back(li.cur_coord());
            } 
            score /= li.line_length;
            break;
        }
        case 2: //RMSE
        {
            do
            {
                score += pow(li.get(),2);
            } while (li.step());
            score = sqrt(score / li.line_length);
            break;
        }
            
    }
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
    //std::cout << "Scoring square " << bot_left.first << ',' <<bot_left.second << "->" << top_right.first << ',' << top_right.second << '\n';
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
        visited.push_back(b.first);
        if(*b.second > 0)
        {
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
    rgb_image.resize(resolution, resolution * rgb_image.width() / rgb_image.height(), 1, 3);
    return rgb_image;
}

template <typename IMG_TYPE>
cimg_library::CImg<IMG_TYPE> string_art<IMG_TYPE>::make_darkness_image(const tcimg& rgb_image)
{
    tcimg b_w = 0.299*rgb_image.get_shared_channel(0) + 
                0.587*rgb_image.get_shared_channel(1) + 
                0.114*rgb_image.get_shared_channel(2);
    return (255 - b_w).normalize(0,SCORE_RESOLUTION);
}

template class string_art<short>;