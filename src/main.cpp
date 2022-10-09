#include <iostream>
#define cimg_use_png 1

#include "CImg.h"
#include "string_art.hpp"
//#include "image_analysis.hpp"
#include "image_editing.hpp"
#include "coord.hpp"
#include <math.h>
#include <vector>
#include <map>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
using std::vector;
using std::map;
using std::pair;
using std::ofstream;
using std::ifstream;
#define SIZE 4096
#define PIN_COUNT 250
#define STEPS 4000
#define MIN_SEPARATION 10
#define MULTIPLIER 0.5
#define CULL_THRESH (short)100
#define SCORE_RANGE 10000
#define DEPTH 2
#define IMAGE_PATH "/home/danny/Programming/String_Wind_Subtractive/images/vg"
#define METHOD 2
typedef short IMG_TYPE;
int main(int, char**) 
{
    short* steps;
    
    for(float m = 0.2; m < 1; m += 0.1) 
    {
        std::stringstream filename;
        filename << std::fixed << std::setprecision(2) << IMAGE_PATH << m << "_s=" << STEPS << "_r=" << SIZE << "_meth=" << METHOD << ".png";
        std::cout <<  "Trial: Multiplier = " << m << "x, Steps = " << STEPS << ", Resolution: " << SIZE << ", Image = " << filename.str() << "\n\n";

        string_art<IMG_TYPE> sa(IMAGE_PATH ".png", SIZE, PIN_COUNT, 0.95f, MIN_SEPARATION, METHOD, m);
        steps = sa.generate(STEPS);
        sa.save_string_image(filename.str().c_str(),true);
        delete[] steps;
    }
};