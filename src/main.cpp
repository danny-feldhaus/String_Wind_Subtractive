#include <iostream>
#define cimg_use_png 1

#include "CImg.h"
#include "line_map.hpp"
//#include "image_analysis.hpp"
#include "image_editing.hpp"
#include "math.h"
#include <vector>
#include <map>
#include <fstream>
#include <string>
#include <sstream>

using std::vector;
using std::map;
using std::pair;
using std::ofstream;
using std::ifstream;
#define SIZE 4096
#define PIN_COUNT 250
#define STEPS 4000
#define MIN_SEPARATION 10
#define THICKNESS 2
#define MULTIPLIER 0
#define CULL_THRESH (short)100
#define SCORE_RANGE 10000
typedef short IMG_TYPE;
int main(int, char**) 
{
    vector<int> path;
    //map<int, vector<image_analysis::score_update>> to_update;

        std::cout << "Processing input image...\n";

    cimg_library::CImg<IMG_TYPE> img("/home/danny/Documents/Prog/String_Wind_Subtractive/images/vg.png");
    img = img.get_channels(0,2); //remove alpha channel
    cimg_library::CImg<IMG_TYPE> cmp_img(img.width(),img.height(),1,3,0);
    cimg_library::CImg<IMG_TYPE> str_img(SIZE,SIZE,1,1,255);
    image_editing::color_difference(img, cmp_img, img);
    img.resize(SIZE,SIZE,1,1);
    img = (255 - img).normalize(0,SCORE_RANGE);

        std::cout << "Generating line information...\n";

    vector<pair<short,short>> pins = circular_pins(SIZE/2,SIZE/2, SIZE/2-10, PIN_COUNT);
    line_map<IMG_TYPE> lines(img, pins,THICKNESS,MIN_SEPARATION);

    ofstream instruction_file("/home/danny/Documents/Prog/String_Wind_Subtractive/instruction_file_400_cull.csv");

    IMG_TYPE best_score = 0;
    int cur_pin = 0;
    int next_pin = 0;
    path.push_back(cur_pin);
    instruction_file << cur_pin << '\n';

        std::cout << "Beginning pathfinding...\n";

    for(int i =0; i < STEPS; i++)
    {
            std::cout << i << ": \n";
            std::cout << "\tFinding next pin... ";
            
        next_pin = lines.best_pin_for(cur_pin,best_score);

            std::cout << "\tNext Pin: " << next_pin << ", Score: " << best_score << '\n';

        path.push_back(next_pin);

            std::cout << "\tUpdating scores...\n";

        lines.update_scores(cur_pin, next_pin, MULTIPLIER);

        image_editing::mult_points(img, lines.line_between(cur_pin, next_pin), MULTIPLIER);
        

        //image_analysis::add_to_updates(min_pin, max_pin, line_scores, to_update);
        //image_analysis::score_pin_from_updates(img, to_update[next_pin], line_map, line_scores, MULTIPLIER);
        //image_editing::mult_points(img, line_map[min_pin][max_pin], MULTIPLIER);
       // image_editing::score_and_multiply<IMG_TYPE>(img, min_pin, max_pin, line_map, to_update,MULTIPLIER);

            std::cout << "\tDrawing String...\n";

        image_editing::draw_points<IMG_TYPE>(str_img, lines.line_between(cur_pin, next_pin), 0);
        if((i+1)%100 == 0)
        {
            img.get_normalize(0,255).save("/home/danny/Programming/String_Wind_Subtractive/images/data.png");
            str_img.save("//home/danny/Programming/String_Wind_Subtractive/images/strings.png");
        }
        /*
        if(max_pin != line_map[min_pin].begin()->first) //Leave at least one connection for each pin, so that it doesn't end the path.
        {
            line_map[min_pin].erase(max_pin);
            line_scores[min_pin].erase(max_pin);
            if(line_map[min_pin].empty())
            {
                line_map.erase(min_pin);
                line_scores.erase(min_pin);
            }
        }*/
        instruction_file << next_pin << '\n';
        cur_pin = next_pin;
    }
    instruction_file.close();
    img.get_normalize(0,255).save("/home/danny/Documents/Prog/String_Wind_Subtractive/images/modified.png");
    str_img.save("/home/danny/Documents/Prog/String_Wind_Subtractive/images/string.png");

};