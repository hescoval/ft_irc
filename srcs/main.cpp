#include "general.hpp"
#include "Server.hpp"

void incorrect_usage()
{
	cout << CYAN << "Usage: ./ircserv " BOLD GREEN "(server port) (server password)" << RESET << endl;
	exit(-1);
}

int main(int ac, char **av)
{
	if(ac != 3)
		incorrect_usage();
	
	try
	{
		Server	srv(av[1], av[2]);
	}
	catch(const std::exception &e)
	{
		std::cerr << RED << "Error: " BOLD CYAN << e.what() << RESET << endl;;
	}
}