#include "Server.hpp"

volatile sig_atomic_t	g_end_signal;

Server::~Server(){}

Server::Server(string port, string password) : _Clients()
{

	if(atoi(port.c_str()) != SERVER_PORT || port.length() != 4)
		throw std::runtime_error("Incorrect port number, try " + toString(SERVER_PORT));

	if (this->_invalidPass(password))
		throw std::runtime_error("Invalid pass: " + string(password) + "\nPassword must be at least 8-characters long and only alphanumeric characters and specials \"!@#$%&*?\" are accepted.");
	this->_password = password;
	this->_clientMax = MAX_CLIENTS;
	this->_name = SERVER_NAME;
	this->_portNumber = SERVER_PORT;
	initializeFunctions();

	setupSocket();
}

bool Server::_invalidPass(const std::string& password)
{
	std::string valid = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%&*?";

	if (password.size() < 8)
		return (true);
	for (size_t i = 0; i < password.size(); i++)
	{
		if (valid.find(password[i]) == std::string::npos)
			return (true);
	}
	return (false);
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

void	signalHandler(int signal)
{
	if (signal == SIGINT || signal == SIGTERM || signal == SIGQUIT)
	{
		cout << "\nReceived signal " << signal << ", shutting down IRC" << std::endl;
		g_end_signal = 1;
	}
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
	struct sigaction sa;
	sa.sa_handler = &signalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGQUIT, &sa, NULL);
	setCreationDate(CurrentDate());
	setCreationTime(CurrentTime());

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
		if (g_end_signal == 1)
			break;
	}
}

void Server::disconnect(int fd)
{
	cout << "Bye bye Mr [ " CYAN << fd <<  RESET" ]" << endl;
	_clientIT it = _Clients.find(fd);
	cout << "deleting client from hashset - " << it->second.getNickname() << endl;
	inUseNicks.erase(it->second.getNickname());
	std::map<std::string, Channel>::iterator	begin = this->_channels.begin();
	std::map<std::string, Channel>::iterator	end = this->_channels.end();

	while (begin != end)
	{
		if (begin->second.findClientByNick(it->second.getNickname()))
			begin->second.removeUser(it->second, "");
		begin++;
	}
	if (it != _Clients.end())
	{
		cout << "Deleting client " << fd << endl;
		_Clients.erase(it);
	}

	close(fd);
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
	if (_fds.size() >= MAX_CLIENTS)
	{
		ServerToUser("Server is full, try again later", new_fd);
		close(new_fd);
		return;
	}
	_fds.push_back(client_poll);

	char hostname[NI_MAXHOST];

	if (gethostname(hostname, sizeof(hostname)) == -1)
		throw std::runtime_error("gethostname() failed");

	Client client = Client(hostname, ntohs(clientAddress.sin_port), new_fd);
	_Clients.insert(std::pair<int, Client>(new_fd, client));
}

void Server::checkClientRequest(int _fd)
{
	try
	{
		_clientIT it = _Clients.find(_fd);
		if (it == _Clients.end())
			return;
		
		int bytes_read = 0;
		string message = it->second._buffer;
		readfd(_fd, message, bytes_read);
		if(bytes_read == 0)
		{
			disconnect(_fd);
			return ;
		}

		it->second._buffer = message;
		if(it->second._buffer.find(EOM) == string::npos)
			return;
		std::vector<string> splits = split(it->second._buffer, EOM);
		for(size_t i = 0; i < splits.size(); i++)
			handleCMD(splits[i], _fd);
		it = _Clients.find(_fd);
		if (it == _Clients.end())
			return ;
		it->second._buffer = "";
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

	message += string(buffer, bytes_read);
}

Client& Server::getClient(int fd)
{
	_clientIT it = _Clients.find(fd);
	if(it != _Clients.end())
		return it->second;
	throw std::runtime_error("Can't find user");
}

Client* Server::getClientByNick(string nick)
{
	for(_clientIT it = _Clients.begin(); it != _Clients.end(); ++it)
		if (it->second.getNickname() == nick)
			return &it->second;
	return NULL;
}

_fdIT Server::getUserPoll(int fd)
{
	for( _fdIT it = _fds.begin(); it != _fds.end(); ++it)
		if (it->fd == fd)
			return it;
	return _fds.end();
}

// The holy grail
void Server::ServerToUser(string message, int fd)
{
	cout << "To user " << fd << " -> [" << message << "]" << endl;
	
	message += EOM;
	send(fd, message.c_str(), message.size(), 0);
}

void Server::setCreationDate(const string str)
{
	this->_creationDate = str;
}

void Server::setCreationTime(const string str)
{
	this->_creationTime = str;
}

string Server::getCreationDate() const
{
	return this->_creationDate;
}

string Server::getCreationTime() const
{
	return this->_creationTime;
}

Channel&	Server::addChannel(std::string name, Client& client)
{
	this->_channels[name] = Channel(name, *this, client);
	return (this->_channels[name]);
}

Channel* Server::getChannel(std::string name)
{

	if (this->_channels.count(name))
		return (&this->_channels[name]);
	return (NULL);
}