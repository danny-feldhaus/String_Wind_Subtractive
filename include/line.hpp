//A struct for 2D coordinates, with CImg integration.

#ifndef LINE_H
#define LINE_H
#include <math.h>
#include <CImg/CImg.h>
#include <coord.hpp>
using std::sqrt;
using std::pow;
using namespace cimg_library;
namespace coordinates
{
    template <class T = short>
    class line
    {
        typedef coord<short> scoord;
        typedef coord<float> fcoord;
        typedef CImg<T> tcimg;

        public:
        tcimg* image;
        /**
         * @brief Constructor
         * @param _a Coordinate of first line end
         * @param _b Coordinate of second line end
         * @param _image Optional, required for pixel access and line_iterator.
         * @param _z Slice index of image
         * @param _c Spectrum index of image
         */
        line(const fcoord _a, const fcoord _b, tcimg* _image = nullptr, const short _z=0, const short _c=0);
        /**
         * @brief Constructor
         * @param ax X coord of first line end
         * @param ay Y coord of first line end
         * @param bx X coord of second line end
         * @param by Y coord of second line end
         * @param _image Optional, required for pixel access and line_iterator.
         * @param _z Slice index of image
         * @param _c Spectrum index of image
         */
        line(const short ax, const short ay, const short bx, const short by, tcimg* _image = nullptr, const short _z=0, const short _c=0);

        T& at(const double d_from_start, const short c = 0, const short z = 0) const;

        bool coord_at(const double d_from_start, fcoord& result) const;

        bool contained_in(const tcimg& img) const;

        bool is_empty() const;

        size_t size() const
        {
            return (size_t)length + 1;
        }

        /**
         * @brief Return a line that covers the intersecting range of two lines
         * @note Follows the direction of the left-hand argument
         * @return 
         *  -Intersection exists: Line over the intersecting region
         *  -Intersection longer than line: Copy of left-hand argument
         *  -Intersection doesn't exist: empty line
         */
        line<T> operator&(const line<T>& other) const;
        
        class line_iterator : public std::iterator<std::random_access_iterator_tag, T>
        {
            friend class line;
            typedef CImg<T> tcimg;
            typedef coord<short> scoord;
            typedef coord<float> fcoord;
            public:
                typedef typename std::iterator<std::random_access_iterator_tag, T>::pointer pointer;
                typedef typename std::iterator<std::random_access_iterator_tag, T>::reference reference;
                typedef typename std::iterator<std::random_access_iterator_tag, T>::difference_type difference_type;

                /**
                 * @brief Constructor
                 * 
                 * @param _image Image to iterate over
                 * @param _pos Position in the line
                 * @param _d Step size (dy / dx)
                 * @param _index Iterator ID. Used for comparison.
                */
                line_iterator(tcimg &_image, const fcoord _pos, const fcoord _d, const short _index = 0, const short _z = 0, const short _c = 0);
                
                reference operator*() const
                {
                    return image(pos.x,pos.y,z,c);
                }

                pointer operator->() const
                {
                    return &image(pos.x,pos.y,z,c);
                }

                reference operator[](const difference_type& n) const
                {
                    fcoord o_pos = pos + d*n;
                    return image(o_pos.x,o_pos.y,z,c);
                }

                bool operator==(const line_iterator& other) const;

                bool operator!=(const line_iterator& other) const;

                bool operator<(const line_iterator& other) const;

                bool operator>(const line_iterator& other) const;

                bool operator<=(const line_iterator& other) const;

                bool operator>=(const line_iterator& other) const;

                line_iterator& operator++();

                line_iterator& operator--();

                line_iterator operator++(int);

                line_iterator operator--(int);

                line_iterator operator+(const difference_type& n) const;

                line_iterator operator-(const difference_type& n) const;

                line_iterator& operator+=(const difference_type& n);

                line_iterator& operator-=(const difference_type& n);

                line_iterator left() const;

                line_iterator right() const;

                difference_type operator+(const line_iterator& other)
                {
                    return index + other.index;
                }

                difference_type operator-(const line_iterator& other)
                {
                    return index - other.index;
                }

                const fcoord& get_pos() const;

            private:
                tcimg &image;
                fcoord pos;
                short index;
                const fcoord d;/**x and y step size*/
                const short z, c;
        };

        line_iterator begin() const;

        line_iterator end() const;

        private:
        const fcoord a;
        const fcoord b;
        const float length;
        const fcoord d;
        const short z, c;

    };

    template <class T = short>
    static double angle(const line<T> &line_a, const line<T> &line_b)
    {
        return acos(dot(line_a.b - line_a.a,line_b.b-line_b.a) / (line_a.length() * line_b.length()));
    }


};    



#endif