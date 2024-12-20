#include "../headers/Command.hpp"

Command::Command(string input)
{
    this->full_command = input;
    std::vector<string> splits = split(input, " ");
    this->command = strUpper(splits[0]);

    for(size_t i = 1; i < splits.size(); i++)
        args.push_back(splits[i]);

    has_flags = false;
    has_target = false;
}

Command::~Command()
{
    return;
}
