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
        string  _hostmask;
        string  _cleanhostmask;
        string  _password;
        bool    _auth;
        int     _fd;
        int     _port;
        bool    _PASSUsed;
        bool    _NICKUsed;
        bool    _USERUsed;

    public:
        Client(string hostname, uint16_t port, int fd);
        ~Client();
        bool    getRegistered() const;
        string  getNickname() const;
		string  getUsername() const;
		string  getRealname() const;
		string  getHostname() const;
        string  getHostmask() const;
        string  getPassword() const;
        string  getcleanHostmask() const;
        bool    getAuth() const;
		int		getFd() const;

        bool    getNICKused() const;

        void    setcleanHostmask(const string str);
        void    setAuth(bool value);
        void    setPassword(const string str);
        void    setPASSUsed(bool value);
        void    setNICKUsed(bool value);
        void    setUSERUsed(bool value);
        void    setNickname(const string str);
        void    setUsername(const string str);
        void    setRealname(const string str);
        void    setHostmask(const string str);
        bool    getPASSUsed() const;
};