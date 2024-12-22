#include "Server.hpp"

#define ERR_COMMANDNOTFND(command) (SERVER_NAMERPL " 987 : " + command + " Command not found.")
#define ERR_NEEDMOREPARAMS(command) (SERVER_NAMERPL " 461 : " + command + " Not enough parameters")
#define ERR_ALREADYREGISTERED() (SERVER_NAMERPL " 462 : Already Registered")
#define ERR_NEEDPWD() (SERVER_NAMERPL " 463 : Please use PASS before attempting any other command.")

//Welcome Messages
#define RPL_WELCOME(hostname) (SERVER_NAMERPL " 001 : Welcome to the " SERVER_HOST " Network " + hostname)
#define RPL_YOURHOST() (SERVER_NAMERPL " 002 : Your host is " SERVER_NAME ", running version 1.0")
#define RPL_CREATED(date, time) (SERVER_NAMERPL " 003 : This server was created " + date + " at " + time)


//NICK
#define ERR_NONICKNAMEGIVEN() (SERVER_NAMERPL " 431 : No nickname given")
#define ERR_NICKNAMEINUSE(nickname) (SERVER_NAMERPL " 433 : " + nickname + " is already in use")
#define ERR_ERRONEUSNICKNAME(nickname) (SERVER_NAMERPL " 432 : " + nickname + " Erroneous nickname")
#define ERR_SAMENICKNAME() (SERVER_NAMERPL " 985 : You already have that nickname")

//PASS
#define ERR_PASSWDMISMATCH() (SERVER_NAMERPL " 464 : Password incorrect")

//USER
#define ERR_BADPROTOCOL() (SERVER_NAMERPL "986 : Bad protocol (Need 0 *)")