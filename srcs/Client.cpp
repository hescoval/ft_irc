#include "../headers/Client.hpp"

Client::~Client(){}

Client::Client(string hostname, uint16_t port, int fd)
{
    _nickname = "default" + toString(fd);
    _username = "defaultUserName" + toString(fd);
    _realname = "defaultRealName" + toString(fd);
    _hostname = hostname;
    _port = port;
    _fd = fd;
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