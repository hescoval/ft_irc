#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <limits>
#include <cmath>
#include <iostream>
#include <cctype>
#include <fstream>
#include <filesystem>
#include <vector>
#include <map>
#include <sstream>
#include <ctime>
#include <iomanip>

/*
	The using keyword is only forbidden if you use it for entire namespaces
	eg: using namespace std;

	Using keyword is allowed if you use it for specific elements of a namespace
	eg: Under this comment block

	Rule is likely in place to prevent namespace pollution and to make sure you know
	where the things you are using come from.
*/

using std::string;
using std::cout;
using std::endl;

# define RESET	"\e[0m"

# define BOLD	"\e[1m"
# define DIM	"\e[2m"
# define ITAL	"\e[3m"
# define ULINE	"\e[4m"

# define BLACK	"\e[30m"
# define RED	"\e[31m"
# define GREEN	"\e[32m"
# define YELLOW	"\e[33m"
# define BLUE	"\e[34m"
# define PURPLE	"\e[35m"
# define CYAN	"\e[36m"
# define WHITE	"\e[37m"

# define BRIGHT_BLACK	"\e[90m"
# define BRIGHT_RED		"\e[91m"
# define BRIGHT_GREEN	"\e[92m"
# define BRIGHT_YELLOW	"\e[93m"
# define BRIGHT_BLUE	"\e[94m"
# define BRIGHT_PURPLE	"\e[95m"
# define BRIGHT_CYAN	"\e[96m"
# define BRIGHT_WHITE	"\e[97m"

string join_strings(std::vector<string> vec);
void trim(std::string &test);
void replace_whitespace(string &input);
string strUpper(std::string str);
void remove_consecutives(string &str, char to_remove);
bool checkValidChars(const std::string& str, const std::string& charset);
void strReplace(string &str, const string &from, const string &to);
std::vector<string> split(string str, string delims);
string CurrentDate();
string CurrentTime();

template<typename T>
string toString(const T& value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}