#include <iostream>
#define cimg_use_png 1
#define cimg_use_openmp 1
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
#define SIZES {4096}
#define PIN_COUNTS {250}
#define STEPS {4000}
#define MIN_SEPARATIONS {10}
#define MODIFIERS {0.35,0.4,0.45,0.5,0.55,0.6,0.7}
#define CULL_THRESH (short)100
#define DEPTHS {3}
#define IMAGE_PATHS {"/home/danny/Programming/String_Wind_Subtractive/images/vg2trans"}
#define METHODS {0}
typedef float IMG_TYPE;
int main(int, char**) 
{
    short* instructions;
    
    for(int size : SIZES)
    for(int pin_count : PIN_COUNTS)
    for(int steps : STEPS)
    for(int separation : MIN_SEPARATIONS)
    for(float modifier : MODIFIERS)
    for(int depth : DEPTHS)
    for(const char* path : IMAGE_PATHS)
    for(int method : METHODS)
    {
        std::stringstream filename;
        filename << std::fixed << std::setprecision(2) << path << "_mod=" << modifier << "_s=" << steps << "_r=" << size << "_meth=" << method << ".png";

        string_art<IMG_TYPE> sa((std::string(path) + ".png").c_str(), size, pin_count, 0.95f, separation, method, modifier);
        instructions = sa.generate(steps);

        sa.save_string_image(filename.str().c_str(),true);
        delete[] instructions;
    }
}