#pragma once

#include "Command.hpp"
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
class Command;

typedef std::vector<pollfd>::iterator _fdIT;
typedef std::map<int, Client*>::iterator _clientIT;

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
        void    handleCMD(string message, int fd);
        void    safely_leave(int fd);

        _fdIT   getUserPoll(int fd);
        Client* getClient(int fd);
        /*######################## COMMANDS ##############################*/

        void    PASS(Command nickname, int fd);

    public:
        Server(string port, string password);
        void startServer();
        ~Server();
};  