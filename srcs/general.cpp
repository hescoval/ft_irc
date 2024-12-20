#include "../headers/general.hpp"


bool checkValidChars(const std::string& str, const std::string& charset)
{
    return str.find_first_not_of(charset) != std::string::npos;
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
