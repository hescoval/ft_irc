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
        string  command;
        std::vector<string> args;
        std::vector<char> modes_to_add;
        std::vector<char> modes_to_remove;

        Command(string input);
        ~Command();
};