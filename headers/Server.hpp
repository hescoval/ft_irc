#pragma once

#include "Client.hpp"
#include "general.hpp"

#include <sys/socket.h>
#include <sys/select.h>
#include <fcntl.h>
#include <poll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdexcept>
#include <netdb.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 10
#define SERVER_NAME "42IRC"
#define SERVER_PORT 6697
#define EOM "\r\n"

class Client;

class Server
{
    private:
        int _portNumber;
        int _socketfd;
        int _clientMax;
        int _currentClients;
        sockaddr_in _ipV4;
        std::vector<pollfd> _fds;

        std::map<int, Client*> _Clients;
        //std::map<string, Channel*> _Channels; 

        string _password;
        string _name;
        Server();
        void    setupSocket();
        void    setupIPV4();
        void    mainServerLoop();
        void    add_Client();
        void    checkClientRequest(int _fd);
        void    readfd(int _fd, string& message, int& bytes_read);
        void    disconnect(int fd);

    public:
        Server(string port, string password);
        void startServer();
        ~Server();
};  