#include <ascii_info.hpp>
void ascii_info::set_str(const string name, const string val, const short indents)
{
    auto f = std::find_if(fields.begin(), fields.end(), [name](pair<string,string> p){return p.first == name;});
    stringstream ss;
    ss << name << ": ";
    for(short i = 0; i < indents; i++)
    {
        ss << "\t";
    }
    ss << val;
    if(f == fields.end())
    {
        fields.push_back(pair<string,string>(name, ss.str()));
    }
    else
    {
        f -> second = ss.str();
    }
}

void ascii_info::set_int(const string name, const int val, const short indents)
{
    set_str(name, std::to_string(val), indents);
}

void ascii_info::set_flt(const string name, const float val, const int precision, const short indents)
{
    stringstream ss;
    ss << std::fixed << std::setprecision(precision) << val;
    set_str(name, ss.str(), indents);
}

void ascii_info::set_percent(const string name, const float val, const short precision, const bool is_ratio, const short indents)
{
    stringstream ss;
    ss << std::fixed << std::setprecision(precision);
    if(is_ratio) ss << val * 100;
    else ss << val;
    ss << "\%";
    set_str(name, ss.str(), indents);
}

void ascii_info::set_progress(const string name, const float cur_val, const float end_val, const short bar_width, const short precision, const short indents)
{
    stringstream ss;
    int width_finished = (cur_val/end_val) * bar_width;
    ss << "|";
    for(int i = 0; i < bar_width; i++)
    {
        if(i < width_finished) ss << "=";
        else ss << "-";
    }
    ss << "| " << std::fixed << std::setprecision(precision) << cur_val << '/' << end_val
    << " (" << 100.f * (cur_val / end_val) << "\%)";
    switch(width_finished % 5)
    {
        case 0:
            ss << "|";
            break;
        case 1:
            ss << "/";
            break;
        case 2:
            ss << "-";
            break;
        case 3:
        default:
            ss << "\\";
            break;
    }
    set_str(name, ss.str(), indents);
}

string ascii_info::to_string(bool use_ANSII) const
{
    stringstream ss;

    for(pair<string, string> f : fields)
    {
        if(use_ANSII) 
            ss << "\033[2K";
        ss << f.second << '\n';
    }
    if(use_ANSII)
        ss << "\033[" << fields.size() << "A";
    return ss.str();
}

string ascii_info::end_string() const
{
    stringstream ss;
    for(int i = 0; i < (int)fields.size(); i++)
    {
        ss << "\033[2K\n";
    }
    ss << "\033[" << fields.size() << "A";
    return ss.str();
}

void ascii_info::clear()
{
    fields.clear();
}