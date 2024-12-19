#include "general.hpp"

class Command
{
    private:
        string  full_command;
        bool    has_target;
        bool    has_flags;
        void    parse_input();

    public:
        std::vector<string> args;
        std::vector<char> modes_to_add;
        std::vector<char> modes_to_remove;

        Command(string input);
        ~Command();
};