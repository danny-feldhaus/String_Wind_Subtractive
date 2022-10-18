#include "display_manager.hpp"

display_manager::display_manager(const short _width, const short _height, const char* title, vector<key_options>* _allowed_keys = nullptr,  bool full_screen = false, bool is_closed = false)
{
	cd = CImgDisplay(_width, _height, title, 0, full_screen, is_closed);
	if(_allowed_keys == nullptr)
	{   
		std::copy(default_allowed_keys.begin(), default_allowed_keys.end(), std::back_inserter(allowed_keys));
	}
	else
	{
		std::copy(_allowed_keys->begin(), _allowed_keys->end(), std::back_inserter(allowed_keys));
	}

}

template <typename T>
void display_manager::update(const char* text = nullptr, bool force_redraw = false)
{
	fps = cd.frames_per_second();
	update_input();
	update_state(text, force_redraw);
}

template <typename T>
void display_manager::update_input()
{
	state.changed = false;
	state.x_move_dir = state.y_move_dir = state.zoom_dir = 0;
	bool holding_move = false;
	for(u_int k : allowed_keys)
	{
		if(cd.is_key(k))
		{
			switch(k)
			{
				case key_options::up:
					if(!state.cur_pressed[k] || (state.move_hold_time > hold_threshold)) state.y_move_dir--;
					holding_move = true;
					break;
				case key_options::down:
					if(!state.cur_pressed[k] || (state.move_hold_time > hold_threshold)) state.y_move_dir++;
					holding_move = true;
					break;
				case key_options::left:
					if(!state.cur_pressed[k] || (state.move_hold_time > hold_threshold)) state.x_move_dir--;
					holding_move = true;
					break;
				case key_options::right:
					if(!state.cur_pressed[k] || (state.move_hold_time > hold_threshold)) state.x_move_dir++;
					holding_move = true;
					break;
				case key_options::zoom_in:
					if(!state.cur_pressed[k] || (state.move_hold_time > hold_threshold)) state.zoom_dir++;
					holding_move = true;
					break;
				case key_options::zoom_out:
					if(!state.cur_pressed[k] || (state.move_hold_time > hold_threshold)) state.zoom_dir--;
					holding_move = true;
				default:
					break;
			}
			if(!state.cur_pressed[k] || state.move_hold_time > hold_threshold) //update movement keys
			{

				state.changed |= holding_move;
			}

			if(!state.cur_pressed[k])
			{
				switch(k)
				{
					case key_options::menu:
						state.is_menu = !state.is_menu;
						state.changed = true;
						break;
					case key_options::pause:
						state.is_paused = !state.is_paused;
						state.changed = true;
						break;
					case cimg::key1 ... cimg::key9:
						if((k != state.image_idx) && (k - cimg::key1) < disp_infos.size())
						{
							state.image_idx = k - cimg::key1;
							state.changed = true;
						}
						break;
					default:
						break;
					
				}
			}
			state.cur_pressed[k] = true;
		}
		else
		{
			state.cur_pressed[k] = false;
		}
	}
	state.move_hold_time = holding_move ? state.move_hold_time + 1.f/fps : 0;
}

template <typename T>
void display_manager::wait(u_int milliseconds)
{
	cd.wait(milliseconds);
}

template <typename T>

u_int display_manager::add_image(const CImg<T>* _image, const u_int _normalization = 0)
{
	disp_infos.push_back({_image, _normalization, 0.5f, 0.5f, 1.f});
	return disp_infos.size()-1;
}

template <typename T>
void display_manager::set_image(const u_int _index, const CImg<T>* _image, const u_int _normalization = 0)
{
	if(index > disp_infos.size()) throw std::domain_error("Index is not assigned to an image.");
	disp_infos[_index].image = _image;
	disp_infos[_index].normalization = _normalization;
}

template <typename T>
bool display_manager::is_paused() const
{
	return state.is_paused;
}

template <typename T>
float display_manager::spf() const
{
	return 1.f / fps;
}

template <typename T>
size_t display_manager::size() const
{
	return disp_infos.size();
}

template <typename T>

bool display_manager::is_key(const u_int key) const
{
	return cd.is_key(key);
}

template <typename T>
void display_manager::update_state(const char* text, bool force_redraw)
{
	
	if((force_redraw || state.changed) && (disp_infos.size() > 0))
	{
		display_info& info = disp_infos[state.image_idx];
		const CImg<T>* image = info.image;
		const float dtime = (fps != 0) ? 1.f/fps : 1.f/60.f;
		info.zoom = std::clamp(info.zoom + state.zoom_dir * zoom_speed * dtime, 1.f, image->width()/2.f);
		int width = pow((float)image->width(), 1.f/info.zoom);
		int height = pow((float)image->height(), 1.f/info.zoom);

		info.center_x = std::clamp(info.center_x + state.x_move_dir * move_speed * dtime, 0.5f/info.zoom, 1.f-0.5f/info.zoom);
		info.center_y = std::clamp(info.center_y + state.y_move_dir * move_speed * dtime, 0.5f/info.zoom, 1.f-0.5f/info.zoom);
		int top_left_x = info.center_x * image->width() - width/2;
		int top_left_y = info.center_y * image->height() - height/2;
		CImg<T> disp_image = image->get_crop(top_left_x, top_left_y, 0, 0, top_left_x+width, top_left_y+width, 0, image->spectrum()-1);
		disp_image.resize(cd);
		cd.set_normalization(info.normalization);
		T w = (info.normalization == 3) ? std::numeric_limits<T>::max() : 255;
		const T white[]{w,w,w};
		const T black[]{0,0,0};
		if(text)
		{
			disp_image.draw_text(0, 0, text, white,black, 1);
		}
		cd.display(disp_image);
	}	
}

template class display_manager<float>;