/**
 * @file output.hpp
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
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
using std::vector;
using std::string;
using std::stringstream;

struct field_obj
{
    string name = "";
    string val = "";
    short indents = 0;
};

class progress_object
{
    public:
    progress_object(const short _field_count = 0, const char** _fields = nullptr, const char** _field_start_vals = nullptr, const short* _field_indents = nullptr, float progress_end_val = 0)
    {
        field_count = _field_count;
        prog_end = progress_end_val;
        fields = vector<field_obj>(field_count);
        for(int i=0; i < field_count; i++)
        {
            string name = string(_fields[i]);
            fields[i].name = string(_fields[i]);
            fields[i].val = (_field_start_vals == nullptr) ? "" : string(_field_start_vals[i]);
            fields[i].indents = (_field_indents == nullptr) ? 0 : _field_indents[i];
        }
    }
    bool add(const char* name, const char* value = "", const short indents = 0)
    {
        string strname(name);
        for(int i=0; i < (int)fields.size(); i++) if(fields[i].name == strname) return false;
        field_obj new_field;
        new_field.name = strname;
        new_field.val = string(value);
        new_field.indents = indents;
        fields.push_back(new_field);
        return true;
    }

    bool add_int(const char* name, const int value, const short indents = 0)
    {
        return add(name, std::to_string(value).c_str(),indents);
    }

    bool add_flt(const char* name, const float value, const int precision = 2, const short indents = 0)
    {
        stringstream ss;
        ss << std::fixed << std::setprecision(precision) << value;
        return add(name, ss.str().c_str(),indents);
    }

    bool set(const char* name, const char* new_val)
    {
        string strname(name);
        for(int i=0; i < (int)fields.size(); i++) 
        {
            if(strname == fields[i].name)
            {
                fields[i].val = new_val;
                return true;
            }
        }
        return false;
    }

    bool set_int(const char* name, const int new_val)
    {
        return set(name, std::to_string(new_val).c_str());
    }   

    bool set_flt(const char* name, const float new_val, const int precision = 2)
    {
        stringstream ss;
        ss << std::fixed << std::setprecision(precision) << new_val;
        return set(name, ss.str().c_str());
    }

    bool set_percent(const char* name, float percent, const int precision = 2, const bool is_ratio = false)
    {
        if(is_ratio) percent *= 100.f;
        stringstream ss;
        ss << std::fixed << std::setprecision(precision) << "\%";
        return set(name, ss.str().c_str());
    }




    bool remove(const char* name)
    {
        string strname(name);
        for(int i=0; i < (int)fields.size(); i++) 
        {
            if(strname == fields[i].name)
            {
                fields.erase(fields.begin() + i);
                return true;
            }
        }
        return false;
    }

    void update_progress(const float new_prog)
    {
        cur_prog = new_prog;
    }
    void reset_progress(const float new_prog_end = 0)
    {
        prog_end = new_prog_end;
        cur_prog = 0;
    }
    string to_string() const
    {
        stringstream ss;
        for(field_obj f : fields)
        {
            for(int i = 0; i < f.indents; i++) ss << '\t';
            ss << f.name << ": " << f.val << '\n';
        }
        if(prog_end != 0)
        {
            ss << '|';
            int prog_percent = 100 * cur_prog / prog_end;
            for(int i = 1; i <= 100; i++)
            {
                if(i <= prog_percent)
                {
                    ss << '=';
                }
                else
                {
                    ss << '-';
                }
            }
            ss << '|' << std::fixed << std::setprecision(1) << "(" << cur_prog << "/" << prog_end << ") " << prog_percent << "\%\n";
        }
        
        ss << "\033[" << fields.size() + (prog_end != 0) << "A";

        return ss.str();
    }
    string clear() const
    {
        stringstream ss;
        for(int i = 0; i < (int)fields.size(); i++) ss << "\33[2K\n";
        if(prog_end != 0) ss << "\33[2K\n";
        return ss.str();
    }
    private:
    vector<field_obj> fields;
    float cur_prog;
    float prog_end;
    bool first_disp = true;
    short field_count;

};


#endif