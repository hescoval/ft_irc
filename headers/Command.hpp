#pragma once 

#include "general.hpp"

class Command
{
    private:
        string  full_command;
        bool    has_target;
        bool    has_flags;
        std::vector<string>  errors;

    public:
        string 								command;
        std::vector<string>					args;
		std::map<int, std::string>			flags;
		std::map<std::string, std::string>	flagArgs;
		string								plus;
		string								minus;

        Command(string input);
        ~Command();

		string	getFullCommand()const ;
};