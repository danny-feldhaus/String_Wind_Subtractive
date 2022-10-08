#include "string_art.hpp"

template <class IMG_TYPE>
string_art<IMG_TYPE>::string_art(const char* image_file, short pin_count, float pin_radius, short pin_separation = 1, u_char score_method = 0, float score_modifier = 0) : image(_image), pin_count(_pins.size()), pins(_pins)
{
        make_map( min_separation);
        string_image = tcimg(image, 0);
        string_image_old = tcimg(string_image);
        score_all_lines();
        return;
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
    for(coord line : line_pairs)
    {
        *scores_by_pin[line.x][line.y] = calculate_score(line.x,line.y);
    }
    return;
}

template <class IMG_TYPE>
void string_art<IMG_TYPE>::update_scores(short pin_a, short pin_b)
{
    vector<scoord> lines_to_update = overlapping_lines(pin_a, pin_b);
    scoord &a_coord = pins[pin_a];
    scoord &b_coord = pins[pin_b];
    
    float line_length;
    scoord cur_coord;
    //int idx;
    long new_score;
    //Update the score for each overlapping line
    float update_count = 0;
    int match_total = 0;
    int match_count = 0;
    string_image_old = string_image;
    draw_line(pin_a,pin_b,255);
    for (scoord p : lines_to_update)
    {
        match_count = 0;
        line_length = distance(pins[p.x],pins[p.y]);
        new_score = ((long)*scores_by_pin[p.x][p.y]) * line_length;
        line_intersection_iterator<IMG_TYPE> lii(image, pins[p.x], pins[p.y],a_coord,b_coord, 3);
        std::deque<scoord> line_coords(3);
        line_coords[0] = lii.cur_coord();
        lii.step();
        line_coords[1] = lii.cur_coord();
        lii.step();
        line_coords[2] = lii.cur_coord();
        while(lii.step())
        {
            //Update the un-rolled average by removing the old score, and adding the new one.
            new_score -= score_square(string_image_old, line_coords);
            new_score += score_square(string_image, line_coords);              
            line_coords.pop_front();
            line_coords.push_back(lii.cur_coord());
            ++match_count;
        };
        new_score /= line_length;
        *scores_by_pin[p.x][p.y] = (IMG_TYPE)new_score;

        update_count += (match_count > 0);
        match_total += match_count;
    }
    std::cout << "\t\t" << update_count << " overlaps updated out of " << lines_to_update.size() << " possible.\n";
    std::cout << "\t\t Average overlap count: " << match_total / update_count << '\n';
    *scores_by_pin[pin_a][pin_b] = 0;

}

template <class IMG_TYPE>
void string_art<IMG_TYPE>::make_map(short min_separation)
{
    int b_start = 0, b_end = 0;
    //Some math I made up, calculates the total number of connections.
    line_count = (pin_count*pin_count - 2*pin_count*min_separation + pin_count)/2;
    //line_indices = vector<vector<int>*>(line_count);
    line_scores = vector<IMG_TYPE>(line_count);
    line_pairs = vector<scoord>(line_count);
    int i = 0;
    for (short a = 0; a < pin_count; a++)
    {
        b_start = (a + min_separation);
        b_end = min(pin_count, (short)((pin_count + a - min_separation)+1));
        for (short b = b_start; b < b_end; b++)
        {
            line_scores[i] = 0;
            scores_by_pin[a][b] = &(line_scores[i]);
            scores_by_pin[b][a] = scores_by_pin[a][b];
            
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
    for(scoord line : line_pairs)
    {
        if((line.x < min_pin && line.y > min_pin && line.y < max_pin) ||
            (line.x > min_pin &&  line.x < max_pin && line.y > max_pin))
        {
                lines.push_back(line);
        }
    }
    return lines;
}

template <class IMG_TYPE>
IMG_TYPE string_art<IMG_TYPE>::calculate_score(short pin_a, short pin_b, u_char method)
{
    long score = 0;
    switch(method)
    {
        case 0: default:
        {
            line_iterator<IMG_TYPE> li(image, pins[pin_a].x, pins[pin_a].y, pins[pin_b].x, pins[pin_b].y, false);
            score = 0;
            do
            {
                score += li.get();
            } while (li.step());
            score /= li.line_length;
        }
            break;
            
        case 1:
        {
            line_iterator<IMG_TYPE> li(image, pins[pin_a].x, pins[pin_a].y, pins[pin_b].x, pins[pin_b].y, false);
            std::deque<scoord> line_coords(3);

            coord bot_left, top_right;
            line_coords[0] = li.cur_coord();
            li.step();
            line_coords[1] = li.cur_coord();
            li.step();
            line_coords[2] = li.cur_coord();
            long score = 0;

            while(li.step())
            {
                score += score_square(string_image, line_coords);
                line_coords.pop_front();
                line_coords.push_back(li.cur_coord());
            }
            score /= li.line_length;
        }
            break; 
    }
    return score;

}

template <class IMG_TYPE>
IMG_TYPE string_art<IMG_TYPE>::score_square(tcimg& string_image_ref, std::deque<scoord>& line_coords)
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
            img_sum += image(cur_coord.x, cur_coord.y);
            //Add to the string score if the point is on the string map, or if it's in the given list of coords.
            if(string_image_ref(cur_coord.x, cur_coord.y))
            {
                line_sum += 10000;
            }
            else
            {
                for(scoord c : line_coords)
                {
                    if(c == cur_coord)
                    {
                        new_line_sum += 10000;
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
    return score + 10000; //just avoiding negatives here.
}

template <class IMG_TYPE>
IMG_TYPE string_art<IMG_TYPE>::darken_line(short pin_a, short pin_b, float multiplier)
{
    line_iterator<IMG_TYPE> li(image, pins[pin_a].x, pins[pin_a].y, pins[pin_b].x, pins[pin_b].y, true);
    long sum = 0;
    do
    {
        sum += li.set(li.get() * multiplier);
    } while (li.step());
    return (IMG_TYPE)(sum / li.line_length);
}

template <class IMG_TYPE>
void string_art<IMG_TYPE>::draw_line(short pin_a, short pin_b, IMG_TYPE color)
{
    line_iterator<IMG_TYPE> li(image, pins[pin_a].x, pins[pin_a].y, pins[pin_b].x, pins[pin_b].y, true);
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
    
    for(auto b : scores_by_pin[from_pin])
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
        auto b = scores_by_pin[from_pin].begin();
        while(b -> first < from_pin && b != scores_by_pin[from_pin].end()) ++b;
        if(b == scores_by_pin[from_pin].end()) --b;
        best_pin = b -> first;
    }
    to_pin = best_pin;
    return best_score;
}

template class string_art<short>;