#include <image_analysis.hpp>

template <typename T>
CImg<int> image_analysis<T>::sized_dark_regions(const CImg<T>& input_image,const T threshold, bool ignore_zeros)
{
    CImg<int> regions = input_image.get_threshold(threshold).label(false, 0, true);        
    map<int, int> region_sizes;
    region_sizes[-1] = 0;
    cimg_forXY(regions, x, y)
    {
        if((input_image(x,y) > threshold )|| (ignore_zeros && input_image(x,y) == 0))
        {
            regions(x,y) = -1;
        }
        else
        {
            int cur_region = regions(x,y);
            if(region_sizes.find(cur_region) != region_sizes.end())
            {
                ++region_sizes[cur_region];
            }
            else
            {
                region_sizes[cur_region] = 1;
                std::cout << "adding region label " << cur_region << '\n';
            }
        }
    }
    cimg_forXY(regions, x, y)
    {
        regions(x,y) = region_sizes[regions(x,y)];
    }
    return regions;
};

template <typename T>
CImg<int> image_analysis<T>::sized_light_regions(const CImg<T>& input_image,const T threshold, bool ignore_zeros){
    CImg<int> regions = CImg<int>(input_image.get_threshold(threshold)).label(false, 0, true);            
    map<int, int> region_sizes;
    region_sizes[-1] = 0;
    cimg_forXY(regions, x, y)
    {
        int cur_region = regions(x,y);
        if((input_image(x,y) < threshold ) || (ignore_zeros && (cur_region == 0)))
        {
            regions(x,y) = -1;
        }
        else if(region_sizes.find(cur_region) != region_sizes.end())
        {
            ++region_sizes[cur_region];
        }
        else
        {
            region_sizes[cur_region] = 1;
        }
    }
    cimg_forXY(regions, x, y)
    {   

        regions(x,y) = region_sizes[regions(x,y)];
    }
    return regions;
};

template <typename T>
CImg<T> image_analysis<T>::color_difference(const CImg<T>& rgb, const T* color)
{
    CImg<T> rgb_Lab = rgb.get_RGBtoLab();
    CImg<T> color_Lab = CImg<T>(color, 1, 1, 1, 3).RGBtoLab();
    CImg<T> diff(rgb.width(), rgb.height(), 1, 1);
    
    cimg_forC(rgb_Lab, c)
    {
        diff += (rgb_Lab.get_shared_channel(c) - color_Lab[c]).sqr();
    }
    diff.sqrt();
    return diff;
}

template<typename T>
float image_analysis<T>::color_difference3x3(const CImg<T>& image_a, const CImg<T>& image_b, const scoord center)
{
    if(!image_a.is_sameXYC(image_b))
        throw CImgArgumentException("Images are not the same size.");
    if(!image_a.containsXYZC(center.x-1, center.y-1, 0, 0) || !image_a.containsXYZC(center.x+1, center.y+1, 0, 0))
        throw CImgArgumentException("Center is not within image.");

    const short samples = 9;
    float diff_sum = 0;
    for(short x = center.x-1; x <= center.x+1; x++)
    {
        for(short y = center.y-1; y <= center.y+1; y++)
        {
            float local_sum = 0;
            cimg_forC(image_a, c)
            {
                local_sum += pow(image_a(x,y,0,c) - image_b(x,y,0,c),2);
            }
            diff_sum += sqrt(local_sum);
        }
    }
    return diff_sum / samples;
}

template <typename T>
float image_analysis<T>::line_color_difference3x3(const CImg<T>& image_a, const CImg<T>& image_b, const scoord point_a, const scoord point_b)
{
    line<T> L(point_a, point_b);
    float diff_sum = 0;
    for(auto a = L.begin(); a < L.end(); a++)
    {
        diff_sum += color_difference3x3(image_a, image_b, a.get_pos());
    }
    return diff_sum / L.size();
}

template class image_analysis<short>;
template class image_analysis<float>;
template class image_analysis<int>;