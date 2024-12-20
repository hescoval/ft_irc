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
        string  _hostname;
        int     _fd;
        int     _port;
        bool    _verified;

    public:
        Client(string hostname, uint16_t port, int fd);
        ~Client();
        bool    getVerified() const;
        string  getNickname() const;
		string  getUsername() const;
		string  getRealname() const;
		string  getHostname() const;
        void    setVerified(bool value);
        void    setNickname(const string str);
        void    setUsername(const string str);
        void    setRealname(const string str);
        void    setHostName(const string str);
};