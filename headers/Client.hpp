#pragma once

#include "general.hpp"
#include "Server.hpp"
#include <inttypes.h>

class Client
{
    private:
        string  _nickname;
        string  _username;
        string  _realname;
        string  _password;
        string  _hostname;
        int     _fd;
        int     _port;

    public:
        Client(string hostname, uint16_t port, int fd);
        ~Client();
        string getNickname() const;
		string getUsername() const;
		string getRealname() const;
		string getHostname() const;
		string getPassword() const;
};