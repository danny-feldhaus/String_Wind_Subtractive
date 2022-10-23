#include <iostream>
#define cimg_use_png 1
#define cimg_use_openmp 1
#include <string_art.hpp>
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

#define SIZES {4096, 2048, 1024}
#define PIN_COUNTS {250}
#define STEPS {8000}
#define MIN_SEPARATIONS {10}
#define MODIFIERS {0.5f, 0.6f, 0.7f, 0.8f, 0.9f}
#define CULL_THRESH (short)100
#define DEPTHS {2}
#define IMAGE_PATHS {"/home/danny/Programming/String_Wind_Subtractive/images/vg2_hr"}
#define METHODS {0}
#define ACC_WEIGHTS {0.f}//, 0.5f, 1.f}
#define SZ_WEIGHTS {0.f}//, 0.5f, 1.f}
#define NEIGHBOR_WEIGHTS {0.f}
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
    for(float wg_sz : SZ_WEIGHTS)
    for(float wg_ng : NEIGHBOR_WEIGHTS)
    {
        std::stringstream filename;
        filename << std::fixed << std::setprecision(2) << path <<
                "_mod=" << modifier << 
                "_s=" << steps << 
                "_r=" << size << 
                "_meth=" << method << 
                "_d=" << depth <<
                "_wgsz=" << wg_sz <<
                "_wgng=" << wg_ng <<
                ".png";
        std::cout << "Calculating image " << filename.str() << '\n';
        string_art<IMG_TYPE> sa((std::string(path) + ".png").c_str(), size, pin_count, 0.95f, separation, method, modifier, depth, wg_sz, wg_ng);
        instructions = sa.generate(steps);

        sa.save_string_image(filename.str().c_str(),true);
        delete[] instructions;
    }
}