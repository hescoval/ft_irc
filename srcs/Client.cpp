#include "../headers/Client.hpp"

Client::~Client(){}

Client::Client(string hostname, uint16_t port, int fd)
{
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

string Client::getPassword() const 
{
    return _password;
}