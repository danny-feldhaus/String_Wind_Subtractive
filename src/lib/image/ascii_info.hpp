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

using std::string;
using std::stringstream;
using std::vector;
using std::pair;
using std::make_pair;
using std::map;
class ascii_info
{
public:
    ascii_info();

    void set_str(const string name, const string val, const short indents = 0);

    void set_int(const string name, const int val, const short indents = 0);

    void set_flt(const string name, const float val, const int precision, const short indents = 0);

    void set_percent(const string name, const float val, const short precision = 0, const bool is_ratio =  false, const short indents = 0);

    void set_progress(const string name, const float cur_val, const float end_val, const short bar_width = 100, const short precision = 0, const short indents = 0);

    string to_string(bool use_ANSII=true) const;
    
    string end_string() const;

    void clear();
private:
    vector<pair<string, string>> fields;
};
