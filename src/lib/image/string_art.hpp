#ifndef STRING_ART_H
#define STRING_ART_H
#define SCORE_RESOLUTION 1000
#define DEPTH 2
#define cimg_use_png 1
#include "coord.hpp"
#include "CImg.h"
#include "line_iterator.hpp"
#include <map>
#include <vector>
#include <deque>
#include <math.h>
#include <assert.h>
#include <stdexcept>
#include <string>
#include <iomanip>
using std::make_pair;
using std::map;
using std::max;
using std::min;
using std::pair;
using std::vector;
using coordinates::coord;



/**
 * @brief Calculates a strings-around-pegs representation of an image.
 * 
 * 
 * @tparam IMG_TYPE 
 */
template <typename IMG_TYPE>
class string_art
{
    typedef cimg_library::CImg<IMG_TYPE> tcimg;
    typedef coord<short> scoord;

    public:
        /**
         * @brief Construct a new string art object
         * 
         * @param _image_file Filename of the input image
         * @param _resolution Resized width of the input image
         * @param _pin_count Number of pins to generate
         * @param _pin_radius Radius of the pin circle. A ratio of the image radius
         * @param _mclearin_separation Minimum pin difference between string connections
         * @param _score_method Method for scoring the lines
         * - 0: Line darkening
         * - 1: 3x3 differenceg
         * - 2: Root mean square error / darkening
         * @param _score_modifier Only used for score_method = 1. 1 = no darkening, 0 = 100% darkening
         */
        string_art(const char* _image_file,const short _resolution,const short _pin_count, float _pin_radius, short _min_separation = 1, u_char _score_method = 0, float _score_modifier = 0);
        
        ~string_art();

        /**
         * @brief Generate the path
         * @details Should be called after construction to start generation
         * @param path_steps Number of steps in the generated path
         * @return A dynamically-allocated array of steps.
         */
        short* generate(const short path_steps);

        bool write_to_csv(const char* instruction_file);

        bool save_string_image(const char* image_file, bool append_debug_info = false);

    private:
        /** @brief Re-sized image from the input filepath */
        const tcimg rgb_image;
        /** @brief Same image as rgb_image in Lab color space */
        tcimg lab_image;
        /** @brief Image darkness map. 0 = white, 10000 = black */
        tcimg darkness_image;
        /** @brief Visual representation of the chosen string path */
        tcimg string_image;
        /** @brief Copy of string_image, one step behind.*/
        tcimg old_string_image;
        /** @brief Number of pins */
        const short pin_count; 
        /** @brief Coordinates of the generated pins
         * @details In image space (e.g. in a 256x556 image, (254,254) corresponds to the top right corner).
         */
        const scoord* pins;
        /** @brief Method for scoring the lines (see constructor)*/
        const u_char score_method;
        /** @brief Darkening modifier when score_method == 0 (see constructor)*/
        const float score_modifier;
       /** @brief Total number of possible connections */
        short line_count;

        /** @brief The current score of each line 
         *  @details This doesn't have a calculated order, so each element is mapped to by line_scores_by_pin.
        */
        IMG_TYPE* line_scores;
        /** @brief A map of all line scores with pins as indices
         * @details Each element is a pointer to an element of line_scores. <br>
         * Each element is double-represented at [a][b] and [b][a], to avoid the need for swapping min/max values.
         */
        map<short, map<short, IMG_TYPE*>> line_scores_by_pin;
        /** @brief Every allowed connection
         * @details Each (x,y) pair corresponds to a pair of pins that may have a string drawn between them.
        */
        scoord* line_pairs;


 
        
        /** @brief Make the containers for lines and their scores.
         * @param min_separation Minimum difference between pins in a line
         */
        void build_lines(short min_separation);

        /**
         * @brief Create a vector of all line combinations that overlap the given line
         * @details Each coordinate in the vector corresponds to a pin-pair.
         * @param pin_a Pin A of the overlapping line
         * @param pin_b Pin B of the overlapping line
         * @return vector<scoord> 
         */
        vector<scoord> overlapping_lines(short pin_a, short pin_b);

        /**
         * @brief Calculate initial scores for all possible connections
         * @details Behaves differently depending on score_method
         * @return Highest scoring pin
         */
        void score_all_lines();

        /**
         * @brief Update the scores of lines that overlap pin_a->pin_b
         * @details This updates the scores originally calculated by score_all_lines(), 
         *          except it's much faster because it only operates near the pixels that intersect with pin_a->pin_b
         * @param pin_a Pin A of the overlapping line
         * @param pin_b Pin B of the overlapping line
         */
        void update_scores(short pin_a, short pin_b);

        /**
         * @brief Update the score of a line
         * @details Re-calculates a line's score using its existing score, and changes to the changed (intersecting) region.
         * @param scored_a Pin A of the line to score
         * @param scored_b Pin B of the line to score
         * @param overlap_a Pin A of the overlapping line
         * @param overlap_b Pin B of the overlapping line
         * @param old_string_image String image before the overlapping line is added
         * @param new_string_image String image after the overlapping line is added
         * @return IMG_TYPE Updated score
         */
        IMG_TYPE update_score(const short scored_a, const short scored_b, const short overlap_a, const short overlap_b, tcimg& old_string_image, tcimg& new_string_image);
        
        /**
         * @brief Score the given line
         * @details Behavior depends on score_method.
         * @param pin_a Pin A of connection
         * @param pin_b Pin B of connection         * @return IMG_TYPE Score
         */
        IMG_TYPE initial_score(const short pin_a, const short pin_b);

        /**
         * @brief Score potential improvement in a given range
         * @details Calculates the potential increase in image similarity, if the given three coordinates are drawn.
         *          The area scored is the 3x3 square that contains all three coordinates, with line_coords[1] at the center.
         * @param string_image_ref #Image to score against
         * @param line_coords Three-pixel segment of a potential line
         * @return IMG_TYPE Score
         */
        IMG_TYPE score_square(const tcimg& string_image_ref, std::deque<scoord>& line_coords) const;
        
        /**
         * @brief Multiply pixels in a line by a number
         * 
         * @param pin_a Pin A of line
         * @param pin_b Pin B of line
         * @param multiplier Amount to multiply each pixel
         * @return IMG_TYPE New average value of line
         */
        void darken_line(tcimg& image, const short pin_a,const short pin_b);

        /**
         * @brief Assign a color to pixels in a line
         * @param image Image to modify
         * @param pin_a Pin A in line
         * @param pin_b Pin B in line
         * @param color Color to assign
         */
        void draw_line(tcimg& image, short pin_a, short pin_b, IMG_TYPE color = 0);
        
        /**
         * @brief Find the highest-scoring pin out of all pins
         * @details Used to find the first pin in the path
         * 
         * @return short Highest scoring pin
         */
        short best_pin();

        /**
         * @brief Find the highest-scoring path from from_pin
         * @details Wrapper for best_score_for(short from_pin, IMG_TYPE& score, short depth). Performs a recursive sum of future steps, without re-calculating scores. 
         * @warning Performance is proportinal to \f$\textrm{pin_count}^\textrm{depth}\f$. Extremely slow after about depth = 3.
         * @param from_pin First pin in the connection
         * @param score Reference to the returned line's score
         * @param depth Depth of the recursive search
         * @return short Best pin to move to
         */
        short best_pin_for(short from_pin, IMG_TYPE& score, short depth = 1);

        /**
         * @brief Recursively search for best-scoring combination of steps
         * @details Performs a recursive sum of future steps, without re-calculating scores. Beware: The number of operation is \f$\textrm{pin_count}^\textrm{depth}\f$, so it gets very big very quickly.
         * @param from_pin First pin in the connection
         * @param depth Depth of the recursive search
         * @param to_pin Reference to the returned score's line
         * @param visited A list of points in the current recursive path (used to avoid loops)
         * @return IMG_TYPE Best score
         */
        IMG_TYPE best_score_for(short from_pin, short depth, short& to_pin, vector<short>& visited);

        std::string ASCII_line(const short from_pin, const short to_pin, const short resolution);

        static scoord* circular_pins(const tcimg& image, float radius, short pin_count);
        
        /**
         * @brief Calculate the number of possible connections
         * @param pin_count Number of pins
         * @param pin_separation Minimum difference between pins in connection
         * @return short Number of connections
         */
        static short calculate_line_count(short pin_count, short min_separation);

        static tcimg make_rgb_image(const char* image_file, short resolution);

        static tcimg make_darkness_image(const tcimg& rgb_image);

        
};
#endif
