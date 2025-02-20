#include "general.hpp"

string CurrentDate() 
{
    std::time_t now = std::time(0);
    std::tm* now_tm = std::localtime(&now);

    std::ostringstream date_stream;
    date_stream << (now_tm->tm_year + 1900) << "-"
                << std::setw(2) << std::setfill('0') << (now_tm->tm_mon + 1) << "-"
                << std::setw(2) << std::setfill('0') << now_tm->tm_mday;
    return date_stream.str();
}

string CurrentTime() 
{
    std::time_t now = std::time(0);
    std::tm* now_tm = std::localtime(&now);

    std::ostringstream time_stream;
    time_stream << std::setw(2) << std::setfill('0') << now_tm->tm_hour << ":"
                << std::setw(2) << std::setfill('0') << now_tm->tm_min << ":"
                << std::setw(2) << std::setfill('0') << now_tm->tm_sec;
    return time_stream.str();
}

bool checkValidChars(const std::string& str, const std::string& charset)
{
    return !(str.find_first_not_of(charset) != std::string::npos);
}

string strUpper(std::string str)
{
	for(size_t i = 0; i < str.size(); i++)
		str[i] = toupper(str[i]);

	return str;
}

void trim(std::string &test)
{
	size_t end = test.size() - 1;

	while(end > 0 && isspace(test[end]))
		end--;
	
	test.erase(end + 1);

	size_t i = 0;

	while(i < test.size() && isspace(test[i]))
		i++;
	
	test.erase(0, i);
}

void replace_whitespace(string &input)
{
	size_t i = 0;
	while(i < input.size())
	{
		if (isspace(input[i]))
			input[i] = ' ';
		i++;
	}
}

void strReplace(string &str, const string &from, const string &to)
{
	size_t start_pos = 0;
	while((start_pos = str.find(from, start_pos)) != string::npos)
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
}

void remove_consecutives(string &str, char to_remove)
{
	size_t i = 0;
	while(i < str.size())
	{
		if (str[i] == to_remove && str[i + 1] == to_remove)
			str.erase(i--, 1);
		i++;
	}
}

std::vector<string> split(string str, string delim)
{
    std::vector<string> ret;
    size_t start = 0;
    size_t end = 0;

    while ((end = str.find(delim, start)) != string::npos) 
	{
        if (end != start) 
            ret.push_back(str.substr(start, end - start));
        start = end + delim.length();
    }
    if (start < str.size())
        ret.push_back(str.substr(start));
    return ret;
}

string join_strings(std::vector<string> vec)
{
	string ret = "";
	for(size_t i = 0; i < vec.size(); i++)
	{
		ret += vec[i];
	}
	return ret;
}

string join_strings(std::vector<string> vec, int start_i)
{
	string ret = "";
	for(size_t i = start_i; i < vec.size(); i++)
	{
		ret += vec[i];
	}
	return ret;
}

string readFile(const string& filePath) 
{
    std::ifstream file(filePath.c_str());
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file");
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool	flagExists(string str, char c)
{
	return (str.find(c) != string::npos);
}

bool	uniqueChar(string str, char c)
{
	if (std::count(str.begin(), str.end(), c) > 1)
		return (false);
	return (true);
}