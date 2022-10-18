/**
 * @file display_manager.hpp
 * @author Danny Feldhaus (danny.b.feldhaus@gmail.com)
 * @brief Tool for printing progress information to the console
 * @version 0.1
 * @date 2022-10-11
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef OUTPUT_H
#define OUTPUT_H
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <exception>
#include <utility>
#include <algorithm>
#include <limits>
#include "CImg.h"
using std::string;
using std::stringstream;
using std::vector;
using std::pair;
using std::make_pair;
using std::map;
template <typename T>
class display_manager
{
public:
    /** @brief All allowed keys (excluding numerical) */
    enum key_options : const u_int
    {
        menu = cimg::keyESC,
        pause = cimg::keySPACE,
        zoom_in = cimg::keyZ,
        zoom_out = cimg::keyX,
        up = cimg::keyARROWUP,
        down = cimg::keyARROWDOWN,
        left = cimg::keyARROWLEFT,
        right = cimg::keyARROWRIGHT,
    };

	/**
	 * @brief Constructor for display_manager
	 * @param _width Width of window
	 * @param _height Height of window
	 * @param title Title of window
	 * @param _allowed_keys List of allowed keys, in the cimg::key format. Defaults to allowing all implemented keys
	 * @param full_screen Whether to start in full screen. 
	 * @param is_closed Whether to start closed.
	 */
	display_manager(const short _width, const short _height, const char* title, vector<key_options>* _allowed_keys = nullptr,  bool full_screen = false, bool is_closed = false);

	/**
	 * @brief Update the display based on user input
	 * @param text Text to overlay on the image
	 * @param force_redraw If \c true, display re-draws without keypresses.
	 */
	void update(const char* text = nullptr, bool force_redraw = false);

	/**
	 * @brief Read input and change state, but do not redraw the image
	 */
	void update_input();

	/**
	 * @brief Wait the given length of time
	 * @param milliseconds Time to wait in milliseconds
	 */
	void wait(u_int milliseconds);

	/**
	 * @brief Add a new image to the end of the display list
	 * @param _image Image pointer to add
	 * @param _normalization 
	 * @return Normalization method used (0=none, 1=always, 2=once, 3=pixel type-dependent)
	 */
	u_int add_image(const CImg<T>* _image, const u_int _normalization = 0);

	/**
	 * @brief Assign a new image to the given index
	 * @param _index Index assigned to (within 0 to size()-1)
	 * @param _image Image pointer to assign
	 * @param _normalization Normalization method used (0=none, 1=always, 2=once, 3=pixel type-dependent)
	 */
	void set_image(const u_int _index, const CImg<T>* _image, const u_int _normalization = 0);

	/**
	 * @brief Return the pause state of the display
     * @note Being paused changes nothing about interior function. 
     *       All pause actions must be controlled by the object's owner.
	 */
	bool is_paused() const;

	/**
	 * @brief Seconds per-frame
	 * @return Number of seconds between display draws
	 */
	float spf() const;

	/**
	 * @brief Number of images in the display list
	 */
	size_t size() const;

	/**
	 * @brief Return true if key specified by given keycode is being pressed on the associated window, false otherwise. 
	 * @param key CImg keycode to test
	 */
	bool is_key(const u_int key) const;

private:
    //All display info for an individual slide
    struct display_info
    {
        const CImg<T>* image = nullptr;
        u_int normalization = 0;
        //x / y coodinates in local space (0 => 0, image.width() => 1)
        float center_x = 0.5f;
        float center_y = 0.5f;
        float zoom = 1;
    };
    //Current state of the window. Changed by user input.
    struct window_state
    {
        bool changed = false;
        u_int image_idx = 0;
        bool is_menu = false;
        bool is_paused = false;
        int x_move_dir = 0;
        int y_move_dir = 0;
        int zoom_dir = 0;
        bool is_moved(){return x_move_dir || y_move_dir || zoom_dir;};
        float move_hold_time = 0;
        map<u_int, bool> cur_pressed;
    };
    /** @brief Number of seconds before a key is considered held*/
    const float hold_threshold = 0.1f;
    /** @brief units per-second travelled with arrow keys */
    const float move_speed = 0.5f;
    /** @brief Zoom levels per-second travelled with zoom keys */
    const float zoom_speed = 0.5f;
    /** @brief Default keys, assigned if none are given. */
    vector<u_int> default_allowed_keys{key_options::zoom_out,
                                             key_options::down, 
                                             key_options::left, 
                                             key_options::up, 
                                             key_options::right, 
                                             key_options::pause, 
                                             key_options::menu, 
                                             key_options::zoom_in,
                                             cimg::key1, cimg::key2, cimg::key3, cimg::key4, cimg::key5, cimg::key6, cimg::key7, cimg::key8, cimg::key9};
    /** @brief Keys that will be checked for during an update*/
    vector<u_int> allowed_keys;
    /** @brief  Info for each image slide*/
    vector<display_info> disp_infos;
    /** @brief Current state of the window*/
    window_state state;
    /**  @brief Handles all actual display operations */
    CImgDisplay cd; 
    /** @brief Current fps, measured by the time between calls to update()*/
    float fps = 0;
	/** @brief Update the display based on \c state
	 * @param text Text to overlay on the image
	 * @param force_redraw If \c true, display re-draws without keypresses.*/
	void update_state(const char* text, bool force_redraw);
};
#endif