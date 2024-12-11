#include "../headers/general.hpp"

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

std::vector<string> split(string str, string delims)
{
	std::vector<string> ret;
	size_t start = 0;
	size_t end = 0;

	while((end = str.find_first_of(delims, start)) != string::npos)
	{
		if (end != start)
			ret.push_back(str.substr(start, end - start));
		start = end + 1;
	}
	if (start < str.size())
		ret.push_back(str.substr(start));
	return ret;	
}