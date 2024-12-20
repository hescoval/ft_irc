#include "../headers/Server.hpp"

Server::~Server(){}

Server::Server(string port, string password) : _Clients()
{

    if(atoi(port.c_str()) != SERVER_PORT || port.length() != 4)
        throw std::runtime_error("Incorrect port number, try " + toString(SERVER_PORT));

    this->_password = password;
    this->_clientMax = MAX_CLIENTS;
    this->_name = SERVER_NAME;
    this->_portNumber = SERVER_PORT;
    initializeFunctions();

    setupSocket();
}

void Server::initializeFunctions()
{
    functions.insert(std::make_pair("QUIT", &Server::QUIT));
    functions.insert(std::make_pair("PASS", &Server::PASS));
    functions.insert(std::make_pair("NICK", &Server::NICK));
}

void Server::setupSocket()
{
    this->_socketfd = socket(AF_INET, SOCK_STREAM, 0); // Create a socket
    if (this->_socketfd == -1) // Maybe need to close this socket
        throw std::runtime_error("socket() failed to create socket");

    int opt = 1;

    // Allows reuse of the socket in case it's already in use
	if (setsockopt(this->_socketfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		close(this->_socketfd);
		throw std::runtime_error("Couldn't set socket options");
	}

    // Set socket to non-blocking mode
    if (fcntl(this->_socketfd, F_SETFL, O_NONBLOCK) == -1)
	{
		close(this->_socketfd);
		throw std::runtime_error("Couldn't set socket to non-blocking mode");
	}
    setupIPV4();
}

void Server::setupIPV4()
{
    // Allows the server to accept connections on any of the host's IP addresses.
    this->_ipV4.sin_addr.s_addr = INADDR_ANY;

    // This constant specifies the address family for the socket. In this case the IPv4 protocol.
    this->_ipV4.sin_family = AF_INET;

    /*
        Converts SERVER_PORT from host byte order to network byte order (big-endian).
        Network protocols expect multi-byte values to be transmitted in a specific order.
    */
    this->_ipV4.sin_port = htons(SERVER_PORT);

    // Bind the socket to the address
    if (bind(this->_socketfd, (struct sockaddr *)&_ipV4, sizeof(_ipV4)) == -1)
	{
		close(this->_socketfd);
		throw std::runtime_error("Cannot bind socket to address");
	}

    // Listen for incoming connections
    if (listen(this->_socketfd, MAX_CLIENTS) == -1)
	{
		close(this->_socketfd);
		throw std::runtime_error("Cannot listen on socket");
	}

    pollfd server_fd = {this->_socketfd, POLLIN, 0};
    _fds.push_back(server_fd);

    mainServerLoop();
}


/*
    POLLHUP, on the other hand, indicates that your file descriptor is valid, but that it's in a state where:

    A device has been disconnected, or a pipe or FIFO has been closed by the last process that
    had it open for writing. 
    Once set, the hangup state of a FIFO shall persist until some process opens the FIFO
    for writing or until all read-only file descriptors for the FIFO are closed. 
    This event and POLLOUT are mutually-exclusive; a stream can never be writable if a hangup has occurred. 
    However, this event and POLLIN, POLLRDNORM, POLLRDBAND, or POLLPRI are not mutually-exclusive. 
    This flag is only valid in the revents bitmask; it shall be ignored in the events member.
*/
void Server::mainServerLoop()
{
    while(true)
    {
        if (poll(_fds.data(), _fds.size(), -1) == -1)
			throw std::runtime_error("Poll failed");

        for(_fdIT it = _fds.begin(); it != _fds.end(); ++it)
        {
            if (it->revents & POLLIN)
            {
                if (it->fd == this->_socketfd)
                    add_Client();
                else
                    checkClientRequest(it->fd);
                break;
            }

            if (it->revents & POLLHUP)
            {
                disconnect(it->fd);
                _fds.erase(it);
                break;
            }
        }
    }
}

void Server::disconnect(int fd)
{
    cout << "Bye bye Mr [ " CYAN << fd <<  RESET" ]" << endl;
    _clientIT it = _Clients.find(fd);
    close(fd);
    if (it != _Clients.end())
    {
        delete it->second;
        _Clients.erase(it);
    }

    _fdIT poll = getUserPoll(fd);
    if(poll != _fds.end())
        _fds.erase(poll);

    cout << RED << "Client disconnected" << RESET << endl;
}

void Server::add_Client()
{
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);
    int new_fd = accept(this->_socketfd, (struct sockaddr *)&clientAddress, &clientAddressLength);
    if (new_fd == -1)
        throw std::runtime_error("Couldn't accept new client");

    pollfd client_poll = {new_fd, POLLIN, 0};
	_fds.push_back(client_poll);

    if (_fds.size() >= MAX_CLIENTS)
    {
        close(new_fd);
        return;
    }

    pollfd new_client = {new_fd, POLLIN, 0};
    _fds.push_back(new_client);

    char hostname[NI_MAXHOST];

    if (gethostname(hostname, sizeof(hostname)) == -1)
        throw std::runtime_error("gethostname() failed");

    Client *client = new Client(hostname, ntohs(clientAddress.sin_port), new_fd);
    _Clients.insert(std::pair<int, Client*>(new_fd, client));
}

void Server::checkClientRequest(int _fd)
{
	try
	{
		std::map<int, Client *>::iterator it = _Clients.find(_fd);
		if (it == _Clients.end())
			return;
		
		//Client *client = it->second;
        int bytes_read = 0;
		string message;
        readfd(_fd, message, bytes_read);
        if(bytes_read == 0)
        {
            disconnect(_fd);
            return;
        }

        std::vector<string> splits = split(message, EOM);
        for(size_t i = 0; i < splits.size(); i++)
            cout << splits[i];
        for(size_t i = 0; i < splits.size(); i++)
            handleCMD(splits[i], _fd);
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
		throw std::runtime_error("Error: Cannot handle message from client");
	}
}

void Server::readfd(int fd, string& message, int& bytes_read)
{
	char buffer[1024];
	bytes_read = recv(fd, buffer, 1024, 0);
	if (bytes_read == -1)
		throw std::runtime_error("Error: Cannot read from socket");

	message = string(buffer, bytes_read);
    cout << message << endl;
}

Client* Server::getClient(int fd)
{
    _clientIT it = _Clients.find(fd);
    if(it != _Clients.end())
        return it->second;
    return NULL;
}

_fdIT Server::getUserPoll(int fd)
{
    _fdIT it = _fds.begin();
    for(; it != _fds.end(); ++it)
    {
        if (it->fd == fd)
        {
            return it;
        }
    }
    return _fds.end();
}

void Server::handleCMD(string message, int fd)
{
    std::vector<string> messagesplits = split(message, " ");

    if(messagesplits.size() == 0)
        return;
    
    Command input(message);
    cout << input.command << endl;
    std::map<std::string, void (Server::*)(Command, int)>::iterator it = functions.find(input.command);
    if(it != functions.end())
        (this->*(it->second))(input, fd);
    else if(input.command != "CAP")
        ServerToUser(_name + ERR_COMMANDNOTFND, fd);
    
}

void Server::QUIT(Command input, int fd)
{
    (void)input;
    _fdIT userIT = getUserPoll(fd);
    if(userIT != _fds.end())
    {
        _fds.erase(userIT);
    }
	close(fd);
	disconnect(fd);
}

void Server::NICK(Command input, int fd)
{
    string valid = "ABCDEFGHIJKLMNOPQRSTUVXYZ0123456789[]{}\\|";
    Client* client = getClient(fd);

    string nick = strUpper( join_strings(input.args) );
    if(!checkValidChars(nick, valid))
        return;
    
    if(nick == client->getNickname())
    {
        ServerToUser(nick + ERR_NICKNAMEINUSE, fd);
        return;
    }

    std::pair<std::set<string>::iterator, bool> in_use;
    in_use = inUseNicks.insert(nick);

    if(!in_use.second)
    {
        ServerToUser(nick + ERR_NICKNAMEINUSE, fd);
        return;
    }
}

void Server::PASS(Command input, int fd)
{
    string fullpass = "";

    fullpass = join_strings(input.args);

    if(fullpass != this->_password)
    {
        ServerToUser(ERR_PASSWDMISMATCH, fd);
        QUIT(input, fd);
    }
    return;
}


// The holy grail
void Server::ServerToUser(string message, int fd)
{
    message += EOM;
    send(fd, message.c_str(), message.size(), 0);
}