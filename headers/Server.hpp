#pragma once

#include "Command.hpp"
#include "Client.hpp"
#include "general.hpp"
#include "ServerToClient.hpp"

#include <sys/socket.h>
#include <sys/select.h>
#include <fcntl.h>
#include <poll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdexcept>
#include <netdb.h>
#include <arpa/inet.h>
#include <utility>
#include <set>


#define MAX_CLIENTS 10
#define SERVER_NAME "42IRC"
#define SERVER_PORT 6697
#define EOM "\r\n"
#define NUMCOMMANDS 2

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

        std::set<string> inUseNicks;
        std::vector<pollfd> _fds;
        std::map<string, void (Server::*)(Command, int)> functions;
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
        void    initializeFunctions();
        void    ServerToUser(string message, int fd);

        _fdIT   getUserPoll(int fd);
        Client* getClient(int fd);

        /*######################## COMMANDS ##############################*/

        void    QUIT(Command input, int fd);
        void    PASS(Command input, int fd);
        void    NICK(Command input, int fd);

    public:
        Server(string port, string password);
        void startServer();
        ~Server();
};  