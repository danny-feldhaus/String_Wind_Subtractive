#include <image_editing.hpp>

template<typename T> 
void draw_points(CImg<T>& img, vector<int>& idxs, T color = 255)
{
    for(int idx : idxs)
    {
        img[idx] = color;
    }
}