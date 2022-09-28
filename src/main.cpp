#include <iostream>
#include "CImg.h"
#include "image_analysis.hpp"
#include "math.h"

#define SIZE 4096
#define PINS 250
#define STEPS 4000
int main(int, char**) 
{
    std::cout << "Hello, world!\n";
    CImg<u_char> test_img(SIZE,SIZE,1,3);
    float thickness = 5;
    u_char cur_val;
    for(int a = 0; a < STEPS; a++)
    {
        std::cout << "Step " << a << '\n';
        for(int i=0; i < PINS; i++)
        {
            float angle = i * 2 * M_PI / PINS;
            int x = std::cos(angle) * (SIZE/2 - 5) + SIZE/2;
            int y = std::sin(angle) * (SIZE/2 - 5) + SIZE/2;
            thickness = 1;
            image_analysis::line_iterator<u_char> li(test_img,SIZE/2,SIZE/2, x,y,thickness);
            while(!li.done)
            {
                //std::cout << li.start_x << ',' << li.start_y << " -> " << li.end_x << ',' << li.end_y << ": " << li.x << ',' << li.y << '\n';
                cur_val = *li;
                ++li;
            }
        }
    }
    cimg_library::CImgDisplay cd(test_img);
    while(!cd.is_keyESC()) cd.show();
}
