#include "../headers/Command.hpp"

Command::Command(string input)
{
    this->full_command = input;
    parse_input();
}

void Command::parse_input()
{
    return;
}