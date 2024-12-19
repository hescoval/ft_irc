#include "../headers/Server.hpp"

typedef std::vector<pollfd>::iterator _fdIT;
typedef std::map<int, Client*>::iterator _clientIT;

Server::~Server(){}

Server::Server(string port, string password) : _Clients()
{
    if(atoi(port.c_str()) != SERVER_PORT || port.length() != 4)
        throw std::runtime_error("Incorrect port number 6697");

    this->_password = password;
    this->_clientMax = MAX_CLIENTS;
    this->_name = SERVER_NAME;
    this->_portNumber = SERVER_PORT;
    
    setupSocket();
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

    for(_fdIT it = _fds.begin(); it != _fds.end(); ++it)
    {
        if (it->fd == fd)
        {
            _fds.erase(it);
            return;
        }
    }
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