#include "../headers/Client.hpp"

Client::~Client(){}

Client::Client(string hostname, uint16_t port, int fd)
{
    string id = toString(reinterpret_cast<uintptr_t>(this));

    _nickname = "default" + id;
    _username = "defaultUserName" + id;
    _realname = "realname" + id;
    _hostname = hostname;
    _hostmask = _nickname + "[!" + _username + "@" + _hostname + "]";
    _USERUsed = false;
    _NICKUsed = false;
    _PASSUsed = false;

    _port = port;
    _fd = fd;
}

void Client::setPassword(const string str)
{
    _password = str;
}

string Client::getPassword() const
{
    return _password;
}

void Client::setNickname(const string str)
{
    _nickname = str;
}

void Client::setUsername(const string str)
{
    _username = str;
}

void Client::setRealname(const string str)
{
    _realname = str;
}

void Client::setHostmask(const string str)
{
    _hostmask = str;
}

void Client::setPASSUsed(bool value)
{
    _PASSUsed = value;
}

void Client::setNICKUsed(bool value)
{
    _NICKUsed = value;
}

void Client::setUSERUsed(bool value)
{
    _USERUsed = value;
}

string Client::getHostmask() const
{
    return _hostmask;
}

bool Client::getRegistered() const 
{
    return (_USERUsed && _NICKUsed && _PASSUsed);
}

string Client::getNickname() const 
{
    return _nickname;
}

string Client::getUsername() const 
{
    return _username;
}

string Client::getRealname() const 
{
    return _realname;
}

string Client::getHostname() const 
{
    return _hostname;
}

bool    Client::getPASSUsed() const
{
    return _PASSUsed;
}

bool   Client::getAuth() const
{
    return _auth;
}

void    Client::setAuth(bool value)
{
    _auth = value;
}

bool    Client::getNICKused() const
{
    return _NICKUsed;
}