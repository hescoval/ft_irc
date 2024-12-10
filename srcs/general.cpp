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