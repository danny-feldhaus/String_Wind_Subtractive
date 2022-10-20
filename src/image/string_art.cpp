#include "string_art.hpp"

template <class IMG_TYPE>
string_art<IMG_TYPE>::string_art(const char *_image_file, const short _resolution, const short _pin_count, float _pin_radius, short _min_separation, u_char _score_method, float _score_modifier, const short _score_depth, const float accessibility_weight, const float localsize_weight, const float neighbor_weight)
    : 
    #ifdef DEBUG
      dm(display_manager<IMG_TYPE>(1024,1024,"Debug Info")),
    #endif
      rgb_image(make_rgb_image(_image_file, _resolution)),
      lab_image(rgb_image.get_shared_channels(0, 2).get_RGBtoLab()),
      darkness_image(make_darkness_image(rgb_image, _pin_radius)),
      pin_count(_pin_count),
      pins(circular_pins(rgb_image, _pin_radius, _pin_count)),
      score_method(_score_method),
      score_modifier(_score_modifier),
      score_depth(_score_depth),
      line_count(calculate_line_count(_pin_count, _min_separation)),
      wg_accessibility(accessibility_weight),
      wg_localsize(localsize_weight),
      wg_neighbor(neighbor_weight)
{
    build_lines(_min_separation);
    //std::cout << "Mapping lines to slices...\n";
    //CImgList<u_short> slices = map_lines_to_slices();
    std::cout << "Building accessibility map...\n";
    accessibility_map = make_accessibility_map();
    //dm.add_image(&accessibility_map, 2);
    std::cout << "Building region size map...\n";
    region_size_map = make_region_size_map();
    dm.add_image(&region_size_map, 2);
    std::cout << "Weighting darkness image...\n";
    //weight_darkness_image();
    dm.add_image(&darkness_image, 0);
    dm.update();
    string_image = tcimg(rgb_image.width(), rgb_image.height(), 1, 1, 0);
    std::cout << "Scoring all lines...\n";
    score_all_lines();
    dm.add_image(&string_image, 1);
    dm.set_pause(true);
    /*
    #if defined(DEBUG) && defined(DEBUG_SCORING)
        CImg<int> am = accessibility_map.get_normalize(0,255);
        cd -> display(am.crop(am.width()/4,am.height()/4,0,0,am.width()/2,am.height()/2,0,0));
        //std::cout << "Min / Max Accessibility: " << am.min() << ", " << am.max() << '\n';
        while(cd->key() != escape_key)
            cd -> wait();
        //std::cout << "Scoring all lines...\n";
    #endif
    */
    //debug_show_all_connections();
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
short *string_art<IMG_TYPE>::generate(const short path_steps)
{
    if (path_steps < 2)
        throw std::domain_error("Number of steps is out of range (" + std::to_string(path_steps) + ")");
#if defined(DEBUG)
    //cd_image = CImg<IMG_TYPE>(cd_size, cd_size, 1, 3, 255);
        std::deque<float> last_10_sps;
        float runtime_seconds = 0.f;
        float getscore_time = 0.f;
        float update_time = 0.f;
    bool gen_done = false;
    short step = 0;
    std::cout << "Calculating path...\n";
#endif //DEBUG
    short *path = new short[path_steps];
    IMG_TYPE score = 0;

    path[0] = best_pin();
    //Split into two threads for processing and display
    #pragma omp parallel num_threads(2), shared(ai, gen_done, string_image)
    {
    if(omp_get_thread_num() == 0) //Processing
    {
        for (step = 1; step < path_steps; step++)
        {
    #if defined(DEBUG)
            auto start = high_resolution_clock::now();
            auto start_getscore = high_resolution_clock::now();
    #endif //DEBUG && DEBUG_TIMING

            path[step] = best_pin_for(path[step - 1], score, score_depth);

    #if defined(DEBUG)
            auto stop_getscore = high_resolution_clock::now();
            getscore_time = duration_cast<microseconds>(stop_getscore - start_getscore).count() / 1000000.f;
            auto start_update = high_resolution_clock::now();
    #endif //DEBUG && DEBUG_TIMINGs

            update_scores(path[step], path[step - 1]);

    #if defined(DEBUG)
            auto stop_update = high_resolution_clock::now();
            update_time = duration_cast<microseconds>(stop_update - start_update).count() / 1000000.f;
    #endif //DEBUG && DEBUG_TIMING

    #if defined(DEBUG)
            ai.set_flt("Score", score, 2);
            auto stop = high_resolution_clock::now();
            float step_time = duration_cast<microseconds>(stop - start).count() / 1000000.f;
            runtime_seconds += step_time;
            last_10_sps.push_front(1.0f / step_time);
            if (step > 10)
                last_10_sps.pop_back();
            float last_10_avg = std::accumulate(last_10_sps.begin(), last_10_sps.end(), 0.0f) / last_10_sps.size();
            int total_seconds_to = (path_steps - step) / (step/runtime_seconds);
            int hours_to = total_seconds_to / 3600;
            int minutes_to = (total_seconds_to - hours_to * 3600) / 60;
            int seconds_to = total_seconds_to - hours_to * 3600 - minutes_to * 60;
            ai.set_flt("Cur Steps Per Second", last_10_sps.back(), 2);
            ai.set_percent("\% Updating", update_time / step_time, 1, true);
            ai.set_percent("\% Getting Score", getscore_time / step_time, 1, true);
            ai.set_percent("\% Other", 1.f - (getscore_time + update_time) / step_time, 1, true);
            ai.set_flt("Avg Steps Per Second", (step / runtime_seconds), 2);
            ai.set_flt("Local Avg Steps Per Second", last_10_avg, 2);
            ai.set_str("Est. time to completion", (std::to_string(hours_to) + ":" + std::to_string(minutes_to) + ":" + std::to_string(seconds_to)).c_str());
            ai.set_progress("Progress", step + 1, path_steps);
            std::cout << ai.to_string();
    #endif //DEBUG
        }
        gen_done = true;
    }
    else //Display
    {
        while(!gen_done)
        {
            if(!dm.is_paused())
            {
                dm.update(ai.to_string(false).c_str(),true);
                float wait_time = 1.f/30 - dm.spf();
                if(wait_time > 0)
                {
                    dm.wait(wait_time);
                }
            }
            else
            {
                dm.update_input();
                dm.wait(1.f/30);
            }
        }
    }
    } //#PRAGMA
#ifdef DEBUG
    std::cout << ai.end_string();
    ai.clear();
#endif //DEBUG
    return path;
}

template <class IMG_TYPE>
bool string_art<IMG_TYPE>::save_string_image(const char *image_file, const bool append_debug_info)
{
    tcimg save_img = (255 - string_image.get_normalize(0, 255));
    if (append_debug_info)
    {
        tcimg dark_img = (SCORE_RESOLUTION - darkness_image) / ((float)SCORE_RESOLUTION/255.f);
        save_img.append(dark_img).save(image_file);
    }
    else
    {
        save_img.save(image_file);
    }
    return true;
}
#if defined(DEBUG)
template <class IMG_TYPE>
void string_art<IMG_TYPE>::debug_show_all_connections()
{
    /*
    bool show_overlay = false;
    short pin_a = 0;
    const IMG_TYPE *text_color = white;
    const IMG_TYPE *bg_color = black;
    const tcimg overlay = (darkness_image).get_resize(1024,1024).normalize(0,255);
    do
    {
        scoord local_pin_a = pins[pin_a] * cd_scale_mult;
        cimg_forC(cd_image, c)
        {
            if(show_overlay)
            {
                cd_image.get_shared_channel(c) = overlay;
            }
            else
            {
                cd_image.get_shared_channel(c).fill(bg_color[c]);
            }
        }
        
        for(auto b : line_scores_by_pin[pin_a])
        {
            scoord local_pin_b = pins[b.first]*cd_scale_mult;
            IMG_TYPE score_gray = (255 * *b.second) / SCORE_RESOLUTION;
            const IMG_TYPE score_color[3]{score_gray, score_gray, score_gray};
            std::stringstream score_text;
            score_text << std::fixed << std::setprecision(1) << 255 * (*b.second / SCORE_RESOLUTION);
            draw_line_RGB(cd_image, local_pin_a, local_pin_b,score_color);
            cd_image.draw_text(local_pin_b.x, local_pin_b.y, score_text.str().c_str(), text_color, bg_color, 1, 8);
        }
        cd -> display(cd_image);
        switch(cd -> key())
        {
            case back_key:
                pin_a = (pin_count + pin_a -1) % pin_count;
                break;
            case advance_key:
                pin_a = (pin_a + 1) % pin_count;
                break;
            case pause_key:
                show_overlay = !show_overlay;
                break;
            case escape_key:
                //cd -> close();
                break;
            default:
                break;
        }
        //Clear key buffer
        cd -> set_key();
    }while(!cd -> is_closed());
    */
}
#endif 
template <class IMG_TYPE>
short string_art<IMG_TYPE>::best_pin()
{
    IMG_TYPE best_score = 0;
    short best_pin_a = -1;
    short best_pin_b = -1;
    IMG_TYPE avg_a = 0, avg_b = 0;
    for (int i = 0; i < line_count; i++)
    {
        if (line_pairs[i].x >= 0)
        {
            IMG_TYPE cur_score = *line_scores_by_pin[line_pairs[i].x][line_pairs[i].y];
            if (cur_score > best_score)
            {
                best_score = cur_score;
                best_pin_a = line_pairs[i].x;
                best_pin_b = line_pairs[i].y;
            }
        }
    }
    assert(best_pin_a != -1);

    best_pin_for(best_pin_a, avg_a, score_depth);
    best_pin_for(best_pin_b, avg_b, score_depth);
    return (avg_a > avg_b) ? best_pin_a : best_pin_b;
}

template <class IMG_TYPE>
short string_art<IMG_TYPE>::best_pin_for(short from_pin, IMG_TYPE &score, short depth)
{

    short to_pin = 0;
    vector<short> visited{from_pin};
    score = best_score_for(from_pin, depth, to_pin, visited);

    return to_pin;
}

template <class IMG_TYPE>
void string_art<IMG_TYPE>::score_all_lines()
{
    vector<int> to_cull;
    for (int i = 0; i < line_count; i++)
    {
        *line_scores_by_pin[line_pairs[i].x][line_pairs[i].y] = initial_score(line_pairs[i].x, line_pairs[i].y);
        ai.set_int("Line", i);
        ai.set_flt("Score", *line_scores_by_pin[line_pairs[i].x][line_pairs[i].y], 1);
        ai.set_progress("Progress", i, line_count);
        if (*line_scores_by_pin[line_pairs[i].x][line_pairs[i].y] < SCORE_RESOLUTION * 0.01f)
        {
            to_cull.push_back(i);
        }
        std::cout << ai.to_string();
    }
    std::cout << ai.end_string();
    ai.clear();
    /*
    for (int c : to_cull)
    {
        cull_line(c);
    }*/
    
    return;
}

template <class IMG_TYPE>
void string_art<IMG_TYPE>::update_scores(const short pin_a,const short pin_b)
{
    /*
    #if defined(DEBUG) && defined(DEBUG_OVERLAP)
        cd_image.fill(0);
    #endif
    */
    old_string_image.assign(string_image);
    draw_line(string_image, pin_a, pin_b, SCORE_RESOLUTION);
    const short min_pin = min(pin_a, pin_b);
    const short max_pin = max(pin_a, pin_b);

    //#pragma omp parallel for
    for (int i = 0; i < line_count; i++)
    {
        if (line_pairs[i].y >= 0)
        {
            if ((line_pairs[i].x < min_pin && line_pairs[i].y > min_pin && line_pairs[i].y < max_pin) ||
                (line_pairs[i].x > min_pin && line_pairs[i].x < max_pin && line_pairs[i].y > max_pin))
            {
                update_score(line_pairs[i].x, line_pairs[i].y, pin_a, pin_b, old_string_image, string_image);
            }
        }
    }
    switch(score_method)
    {
        case 0:
        case 2:
        default:
            image_editing::multiply_line(darkness_image, pins[pin_a], pins[pin_b], score_modifier, 3, true);
            break;
        case 1:
            break;
    }

    *line_scores_by_pin[pin_a][pin_b] = 0;

}

template <class IMG_TYPE>
IMG_TYPE string_art<IMG_TYPE>::update_score(const short scored_a, const short scored_b, const short overlap_a, const short overlap_b, tcimg &old_string_image, tcimg &new_string_image)
{
    float line_length = line_lengths[scored_a][scored_b];
    float new_score = 0;
    if(line_length == 0) return *line_scores_by_pin[scored_a][scored_b];

    switch (score_method)
    {
    case 0:
    default:
    {
        line_intersection_iterator<IMG_TYPE> lii(darkness_image, pins[scored_a], pins[scored_b], pins[overlap_a], pins[overlap_b], 3);
        if(darkness_image(lii.get_center().x, lii.get_center().y) == 0) return *line_scores_by_pin[scored_a][scored_b];
        new_score = ((float)*line_scores_by_pin[scored_a][scored_b]) * line_length;
        do
        {
            if(lii.is_within_itersection())
            {
                float cur_score = lii.get();
                float left_score = darkness_image(lii.left().x, lii.left().y);
                float right_score = darkness_image(lii.right().x, lii.right().y);
                float total_add = 0;
                float total_remove = 0;
                if (cur_score > 0.01f)
                {
                    total_remove += cur_score;
                    total_add += cur_score * score_modifier;                    
                }
                if (left_score> 0.01f)
                {
                    total_remove += left_score * wg_neighbor;
                    total_add  += left_score * score_modifier * wg_neighbor;
                }
                if (right_score > 0.01f)
                {
                    total_remove += right_score * wg_neighbor;
                    total_add += right_score * score_modifier * wg_neighbor;
                }
                new_score += total_add;
                new_score -= total_remove;
            }
        } while (lii.step());
        new_score /= line_length;
        break;
    }
    case 1:
    {
        line_intersection_iterator<IMG_TYPE> lii(darkness_image, pins[scored_a], pins[scored_b], pins[overlap_a], pins[overlap_b], 3);
        if(darkness_image(lii.get_center().x, lii.get_center().y) == 0) return *line_scores_by_pin[scored_a][scored_b];

        std::deque<scoord> line_coords(3);
        new_score = ((float)*line_scores_by_pin[scored_a][scored_b]) * line_length;
        // Assign the first three pixels in the line
        line_coords[0] = lii.cur_coord();
        lii.step();
        line_coords[1] = lii.cur_coord();
        lii.step();
        line_coords[2] = lii.cur_coord();
        // Iterate through the rest of the line
        do
        {
            float cur_score =  score_square(new_string_image, line_coords);
            if(cur_score != 0)
            {
                new_score -= score_square(old_string_image, line_coords);
                new_score += cur_score;
            }
            line_coords.pop_front();
            line_coords.push_back(lii.cur_coord());
        } while (lii.step());
        new_score /= line_length;
        break;
    }
    case 2: // Root mean square error
    { 
        line_intersection_iterator<IMG_TYPE> lii(darkness_image, pins[scored_a], pins[scored_b], pins[overlap_a], pins[overlap_b], 3);
        if(darkness_image(lii.get_center().x, lii.get_center().y) == 0) return *line_scores_by_pin[scored_a][scored_b];

        new_score = pow(*line_scores_by_pin[scored_a][scored_b], 2) * line_length;
        do
        {
            if(lii.is_within_itersection())
            {                
                float cur_score = lii.get();
                float left_score = darkness_image(lii.left().x, lii.left().y);
                float right_score = darkness_image(lii.right().x, lii.right().y);
                float total_add = 0;
                float total_remove = 0;
                if (cur_score > 0.1f)
                {
                    total_remove += pow(cur_score, 2);
                    total_add += pow(cur_score * score_modifier, 2);
                }
                if(left_score > 0.1f)
                {
                    total_remove += pow(left_score, 2) * wg_neighbor;
                    total_add += pow(left_score * score_modifier, 2) * wg_neighbor;
                }
                if(right_score > 0.1f)
                {
                    total_remove += pow(right_score, 2) * wg_neighbor;
                    total_add += pow(right_score * score_modifier, 2) * wg_neighbor;
                }
                new_score += total_add;
                new_score -= total_remove;
            }
        } while (lii.step());
        new_score = sqrt(new_score / line_length);
        break;
    }
    } //switch(score_method)
    *line_scores_by_pin[scored_a][scored_b] = (IMG_TYPE)new_score;
    return new_score;
}

template <class IMG_TYPE>
void string_art<IMG_TYPE>::build_lines(short min_separation)
{
    line_scores = new IMG_TYPE[line_count];
    line_pairs = new scoord[line_count];
    int i = 0;
    for (short a = 0; a < pin_count; a++)
    {
        int b_start = (a + min_separation);
        int b_end = min(pin_count, (short)((pin_count + a - min_separation) + 1));
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
    short min_pin = min(pin_a, pin_b);
    short max_pin = max(pin_a, pin_b);
    for (int i = 0; i < line_count; i++)
    {
        if (line_pairs[i].x >= 0)
        {
            if ((line_pairs[i].x < min_pin && line_pairs[i].y > min_pin && line_pairs[i].y < max_pin) ||
                (line_pairs[i].x > min_pin && line_pairs[i].x < max_pin && line_pairs[i].y > max_pin))
            {
                lines.push_back(line_pairs[i]);
            }
        }
    }
    return lines;
}

template <class IMG_TYPE>
IMG_TYPE string_art<IMG_TYPE>::initial_score(const short pin_a, const short pin_b)
{
    float score = 0;
    line_iterator<IMG_TYPE> li(darkness_image, pins[pin_a].x, pins[pin_a].y, pins[pin_b].x, pins[pin_b].y, false, 3);
    float masked_length = 0;
    switch (score_method)
    {
    case 0:
    default:
    {
        do
        {
            float cur_score = li.get();
            float left_score = darkness_image(li.left().x,li.left().y);
            float right_score = darkness_image(li.right().x,li.right().y);
            float total_add = 0;
            if (cur_score > 0)
            {
                total_add += cur_score;
                masked_length++;
            }
            if (left_score > 0)
            {
                total_add += left_score * wg_neighbor;
                masked_length += wg_neighbor;
            }
            if (right_score > 0)
            {
                total_add += right_score * wg_neighbor;
                masked_length += wg_neighbor;
            }
            score += total_add;
        } while (li.step());
        if(masked_length > 0) score /= masked_length;
        break;
    }
    case 1:
    {

        std::deque<scoord> line_coords(3);
        scoord bot_left, top_right;
        line_coords[0] = li.cur_coord();
        li.step();
        line_coords[1] = li.cur_coord();
        li.step();
        line_coords[2] = li.cur_coord();
        while (li.step())
        {
            IMG_TYPE cur_val = li.get();
            if(cur_val > 0)
            {
                score += score_square(string_image, line_coords);
                masked_length++;
                line_coords.pop_front();
                line_coords.push_back(li.cur_coord());
            }
        }
        //if(score < 0) score = 0;
        if(masked_length > 0) score /= masked_length;
        break;
    }
    case 2: // RMSE
    {
        do
        {
            float cur_score = li.get();
            if (cur_score != 0)
            {
                score += pow(cur_score, 2);
                masked_length++;
            }
        } while (li.step());
        if(masked_length > 0) score = sqrt(score / masked_length);
        break;
    }
    }
    line_lengths[pin_a][pin_b] = masked_length;
    line_lengths[pin_b][pin_a] = masked_length;
    return score;
}

template <class IMG_TYPE>
IMG_TYPE string_art<IMG_TYPE>::score_square(const tcimg &string_image_ref, std::deque<scoord> &line_coords)
{
    scoord bot_left, top_right, cur_coord;
    float img_sum = 0;
    float line_sum = 0;
    float new_line_sum = 0;
    short square_area = 0;
    bot_left.x = line_coords[1].x - 1;
    bot_left.y = line_coords[1].y - 1;
    top_right.x = line_coords[1].x + 1;
    top_right.y = line_coords[1].y + 1;

    
    for (cur_coord.x = bot_left.x; cur_coord.x <= top_right.x; cur_coord.x++)
    {
        for (cur_coord.y = bot_left.y; cur_coord.y <= top_right.y; cur_coord.y++)
        {
            IMG_TYPE cur_val = darkness_image(cur_coord.x, cur_coord.y);
            if(cur_val > 0)
            {
                ++square_area;
                img_sum += cur_val;
                // Add to the string score if the point is on the string map, or if it's in the given list of coords.
                if (string_image_ref(cur_coord.x, cur_coord.y) > 0)
                {
                    line_sum += SCORE_RESOLUTION;
                    /*
                    #if defined(DEBUG) && defined(DEBUG_SCORING)
                    if(show_pixel_scoring)
                    {
                        cd_image.draw_point(cur_coord.x * cd_scale_mult, cur_coord.y * cd_scale_mult,red);
                    }
                    #endif
                    */
                }
                else if (std::find(line_coords.begin(), line_coords.end(), cur_coord) != line_coords.end())
                {
                    new_line_sum += SCORE_RESOLUTION;
                    /*
                    #if defined(DEBUG) && defined(DEBUG_SCORING)
                    if(show_pixel_scoring)
                    {
                        cd_image.draw_point(cur_coord.x * cd_scale_mult, cur_coord.y * cd_scale_mult,blue);
                    }
                    #endif
                    */
                }
            }
        }
    }
    if(square_area == 0) return 0;
    // Average value of the image in this square
    float image_score = img_sum / square_area;
    // Average value of the existing string image
    float existing_score = line_sum / square_area;
    // Average value with the new line added
    float potential_score = (line_sum + new_line_sum) / square_area;
    
    float existing_diff = image_score - existing_score;
    float potential_diff = image_score - potential_score;
    // Percent similarity of potential (100% = same, 0% = white to black)
    float potential_p_similarity = (1.f - (potential_diff / SCORE_RESOLUTION));
    // Potential reduction in image difference (negative if potential is worse)
    float score = (existing_diff > 0) ? existing_diff - abs(potential_diff) : potential_diff - existing_diff;
    score *= potential_p_similarity;
    /*
    #if defined(DEBUG) && defined(DEBUG_SCORING)
    {
        bool stay = show_pixel_scoring;
        while(stay)
        {
            if(zoom_pixel_scoring)
            {
                cd -> display(cd_image.get_crop(bot_left.x - 2, bot_left.y - 2, top_right.x + 2, top_right.y + 2).resize(1024,1024));
            }
            else
            {
                cd -> display(cd_image);
            }
            const uint key = cd -> key();
            switch(key)
            {
                case pause_key:
                    step_manual = !step_manual;
                    break;
                case zoom_key:
                    zoom_pixel_scoring = !zoom_pixel_scoring;
                    break;
                case advance_key:
                    stay = false;
                    break;
                case escape_key:
                    show_pixel_scoring = false;
                    break;
                default:
                    break;
            }
            cd -> set_key();
            stay &= step_manual;
            if(stay)
            {
                cd -> wait();
            }
        }
    }
    #endif //DEBUG
    */
    return score;
}

template <class IMG_TYPE>
void string_art<IMG_TYPE>::darken_line(tcimg &image, const short pin_a, const short pin_b)
{
    line_iterator<IMG_TYPE> li(image, pins[pin_a].x, pins[pin_a].y, pins[pin_b].x, pins[pin_b].y, true, 3);
    do
    {
        li.set(li.get() * score_modifier);
    } while (li.step());
    return;
}


template <class IMG_TYPE>
void string_art<IMG_TYPE>::draw_line(tcimg &image, const short pin_a, const short pin_b, const IMG_TYPE color)
{
    image_editing::draw_line<IMG_TYPE>(image, pins[pin_a], pins[pin_b], color);
}


template <class IMG_TYPE>
void string_art<IMG_TYPE>::draw_line_RGB(tcimg &image, const scoord line_a, const scoord line_b, const IMG_TYPE* color)
{
    if(image.spectrum() < 3) 
    {
        throw std::domain_error("Not enough channels for an RGB image (Spectrum = " + std::to_string(image.spectrum()) + ")");
    }
    line_iterator<IMG_TYPE> li(image, line_a.x, line_a.y, line_b.x, line_b.y, false);
    do
    {
        for(int c = 0; c < 3; c++)
        {
            image(li.cur_coord().x, li.cur_coord().y, 0, c) = color[c];
        }
    } while (li.step());
}


template <class IMG_TYPE>
void string_art<IMG_TYPE>::cull_line(const int line_index)
{
    line_scores_by_pin[line_pairs[line_index].x].erase(line_pairs[line_index].y);
    line_scores_by_pin[line_pairs[line_index].y].erase(line_pairs[line_index].x);
    line_lengths[line_pairs[line_index].x].erase(line_pairs[line_index].y);
    line_lengths[line_pairs[line_index].y].erase(line_pairs[line_index].x);
    line_pairs[line_index].x = -1;
    line_pairs[line_index].y = -1;
}

template <class IMG_TYPE>
IMG_TYPE string_art<IMG_TYPE>::best_score_for(short from_pin, short depth, short &to_pin, vector<short> &visited)
{
    if (depth == 0)
        return 0;
    short best_pin = -1;
    IMG_TYPE best_score = 0;

    for (auto b : line_scores_by_pin[from_pin])
    {
        short cur_pin = b.first;
        IMG_TYPE cur_score = *b.second;
        for (short v : visited)
        {
            if (to_pin == v)
                continue;
        }
        if (cur_score != 0)
        {
            visited.push_back(to_pin);
            cur_score += best_score_for(to_pin, depth - 1, to_pin, visited);
            visited.pop_back();
            if (cur_score > best_score)
            {
                best_pin = cur_pin;
                best_score = cur_score;
            }
        }
    }
    // If no suitable neighbor was found, return the nearest neighbor
    if (best_pin == -1)
    {
        for(short i = from_pin + 1; i != from_pin; i = (i+1) % pin_count)
        {
            auto b = line_scores_by_pin[from_pin].find(i);
            if(b != line_scores_by_pin[from_pin].end())
            {
                best_pin = i;
                break;
            }
        }
        assert(best_pin != -1);

    }
    to_pin = best_pin;
    return best_score;
}

template <typename IMG_TYPE>
CImgList<u_short> string_art<IMG_TYPE>::map_lines_to_slices()
{
    CImgList<u_short> slices;
    int lines_drawn = 0;
    map<short,map<short,bool>> pairs_made;
    for(short p_origin = 0; p_origin < pin_count; p_origin += 2)
    {
        CImg<u_short> slice(darkness_image.width(), darkness_image.height(), 1, 1, 0);
        int local_lines_drawn = 0;
        for(int offset = 0; offset < pin_count; offset++)
        {
            short p_a = (p_origin - (offset/2) + pin_count) % pin_count;
            short p_b = (p_origin + (offset/2) + (offset%2)) % pin_count;
            auto sub_a = line_scores_by_pin[p_a];
            auto sub_b = sub_a.find(p_b);
            if(sub_b != sub_a.end())
            {
                if(pairs_made[p_a].find(p_b) != pairs_made[p_a].end())
                {
                    std::cout << ((offset%2)?'E':'O') << "-offset pair " << p_a << "->" << p_b << " already made.\n";
                }
                else
                {
                    u_short pair_index = lines_drawn;
                    image_editing::draw_line<u_short>(slice, pins[p_a], pins[p_b], pair_index, 2);
                    pairs_made[p_a][p_b] = true;
                    pairs_made[p_b][p_a] = true;
                    lines_drawn++;
                    local_lines_drawn++;
                }
            }
        }
        slices.push_back(slice);
    }
    assert(lines_drawn == line_count);
    for(int i=0; i < line_count; i++)
    {
        assert(pairs_made.find(line_pairs[i].x) != pairs_made.end());
        assert(pairs_made[line_pairs[i].x].find(line_pairs[i].y) != pairs_made[line_pairs[i].x].end());
    }
    return slices;
}

template <typename IMG_TYPE>
coord<short> *string_art<IMG_TYPE>::circular_pins(const tcimg &image, float radius, short pin_count)
{
    scoord *pins = new scoord[pin_count];
    scoord center(image.width() / 2, image.height() / 2);
    for (int i = 0; i < pin_count; i++)
    {
        float angle = (2.f * 3.14159f * i) / pin_count;
        pins[i].x = center.x + std::cos(angle) * radius * image.width() / 2;
        pins[i].y = center.y + std::sin(angle) * radius * image.height() / 2;
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
int string_art<IMG_TYPE>::calculate_line_count(int pin_count, int min_separation)
{
    return (pin_count * pin_count - 2 * pin_count * min_separation + pin_count) / 2;
}

template <typename IMG_TYPE>
CImg<IMG_TYPE> string_art<IMG_TYPE>::make_accessibility_map() 
{
    CImg<float> am (darkness_image.width(), darkness_image.height(), 1, 1, 0);
    for(int i = 0; i < line_count; i++)
    {
        scoord p = line_pairs[i];
        line_iterator<float> li(am, pins[p.x], pins[p.y], false, 3);
        do
        {
            IMG_TYPE d = darkness_image(li.cur_coord().x, li.cur_coord().y);
            if(d > SCORE_RESOLUTION*0.65f)
            {
                li.set(li.get()+1);
            }
            else if(d != 0)
            {
                li.set(1);
            }
        }while(li.step());
        ai.set_progress("Progress", i+1, line_count);
        std::cout << ai.to_string();
    }

    std::cout << ai.end_string();
    ai.clear();
    cimg_forXY(am, x, y)
    {
        am(x,y) = 1.f / (am(x,y)+1);
    }
    return am;
}

template <typename IMG_TYPE>
CImg<IMG_TYPE> string_art<IMG_TYPE>::make_region_size_map()
{
    CImg<IMG_TYPE> image = image_analysis::sized_light_regions<IMG_TYPE>(darkness_image.get_blur_median(3), (IMG_TYPE)(0.1f*SCORE_RESOLUTION), true);
    
    cimg_forXY(image, x, y)
    {
        if(image(x,y))
            image(x,y) = 1.f / (image(x,y)+1);
    }
    
    return image;
}

template <typename IMG_TYPE>
cimg_library::CImg<IMG_TYPE> string_art<IMG_TYPE>::make_rgb_image(const char *image_file, short resolution)
{
    tcimg rgb_image(image_file);
    rgb_image.resize(resolution, resolution * rgb_image.width() / rgb_image.height(), 1, rgb_image.spectrum());

    return rgb_image;
}

template <typename IMG_TYPE>
cimg_library::CImg<IMG_TYPE> string_art<IMG_TYPE>::make_darkness_image(const tcimg &rgb_image, const float radius)
{
    tcimg b_w = 255 - (0.299 * rgb_image.get_shared_channel(0) +
                       0.587 * rgb_image.get_shared_channel(1) +
                       0.114 * rgb_image.get_shared_channel(2));
    tcimg mask;
    if(rgb_image.spectrum() == 4)
    {
        mask = rgb_image.get_channel(3).threshold(1.f);
    }
    else
    {
        mask = tcimg(rgb_image.width(), rgb_image.height(), 1, 1, 1.f);
    }
    float sqr_img_rad = pow(radius*rgb_image.width()/2, 2);
    scoord center(rgb_image.width()/2, rgb_image.height()/2);
    cimg_forXY(mask, x, y)
    {
        if(pow(x - center.x,2) + pow(y - center.y,2) > sqr_img_rad)
        {
            mask(x,y) = 0;
        }
    }
    b_w.mul(mask);
    CImg<float> hist = b_w.get_histogram(256);
    int cut_threshold = 255;
    float hist_percent = 0;
    while(hist_percent < 0.1f)
    {
        hist_percent += hist[cut_threshold] / mask.size();
        cut_threshold--;
    }
    b_w.cut(0,cut_threshold);
    b_w.equalize(255, 1, cut_threshold);
    std::cout << "End min/max: " << b_w.min() << ',' << b_w.max() << '\n';
    return b_w;
}

template <typename IMG_TYPE>
void string_art<IMG_TYPE>::weight_darkness_image()
{
    darkness_image.mul(1 + accessibility_map * wg_accessibility);
    darkness_image.mul(1 + region_size_map * wg_localsize);

}

template class string_art<short>;
template class string_art<int>;
template class string_art<float>;