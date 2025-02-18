#include "Server.hpp"

void Server::initializeFunctions()
{
	functions.insert(std::make_pair("QUIT", &Server::QUIT));
	functions.insert(std::make_pair("PASS", &Server::PASS));
	functions.insert(std::make_pair("NICK", &Server::NICK));
	functions.insert(std::make_pair("USER", &Server::USER));
	functions.insert(std::make_pair("JOIN", &Server::JOIN)); 
	functions.insert(std::make_pair("TOPIC", &Server::TOPIC));
	functions.insert(std::make_pair("NAMES", &Server::NAMES));
	functions.insert(std::make_pair("MODE", &Server::MODE));
	functions.insert(std::make_pair("WHO", &Server::WHO));
}

void Server::handleCMD(string message, int fd)
{
	std::vector<string> messagesplits = split(message, " ");
	Client& client = getClient(fd);
	if(messagesplits.size() == 0)
		return;
		
	Command input(message);
	std::map<std::string, void (Server::*)(Command, int)>::iterator it = functions.find(input.command);

	if(input.command != "CAP" && input.command != "PASS" && !client.getPASSUsed())
		return ServerToUser(ERR_NEEDPWD(client.getNickname()), fd);
	if(it != functions.end())
		(this->*(it->second))(input, fd);
	else if(input.command != "CAP")
		ServerToUser(ERR_COMMANDNOTFND(input.command, client.getNickname()), fd);
}

void Server::FinishRegistration(Command input, int fd)
{
	Client& client = getClient(fd);
	if(client.getPassword() != this->_password)
	{
		ServerToUser(ERR_PASSWDMISMATCH(client.getNickname()), fd);
		QUIT(input, fd);
		return;
	}

	client.setAuth(true);
	client.setHostmask(client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname());
	client.setcleanHostmask(client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname());
	ServerToUser(RPL_WELCOME(client.getHostmask(), client.getNickname()), fd);
	ServerToUser(RPL_YOURHOST(client.getNickname()), fd);
	ServerToUser(RPL_CREATED(getCreationDate(), getCreationTime(), client.getNickname()), fd);
	cout << "Fd number " << fd << " finished his registration" << endl;
	cout << "Nickname: " CYAN << client.getNickname() << RESET << endl;
	cout << "Username: " CYAN << client.getUsername() << RESET << endl;
	cout << "Realname: " CYAN << client.getRealname() << RESET << endl;
	cout << "Hostname: " CYAN << client.getHostname() << RESET << endl;
	cout << "Hostmask: " CYAN << client.getHostmask() << RESET << endl;
}

void Server::QUIT(Command input, int fd)
{
	//Send message to other clients maybe idk
	(void)input;
	disconnect(fd);
}

void Server::NICK(Command input, int fd)
{
	string valid = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789[]{}\\|";
	Client& client = getClient(fd);
	string old_nick = client.getNickname();
	string nick = join_strings(input.args);

	if(!checkValidChars(strUpper(nick), valid))
		return ServerToUser(ERR_ERRONEUSNICKNAME(client.getNickname(), nick), fd);

	if(nick == old_nick)
		return ServerToUser(ERR_SAMENICKNAME(client.getNickname(), nick), fd);

	std::pair<std::set<string>::iterator, bool> in_use;
	in_use = inUseNicks.insert(nick);
	if(!in_use.second)
	{
		string err_msg = ERR_NICKNAMEINUSE(client.getNickname(), nick);
		if(!client.getNICKused())
			err_msg += ". Defaulting to \"" + client.getNickname() + "\"";

		ServerToUser(err_msg, fd);
		return;
	}
	inUseNicks.erase(old_nick);

	if(client.getNICKused())
		ServerToUser(NAMECHANGE(client.getHostmask(), nick), fd);
	else
		ServerToUser(NAMECHANGE(client.getHostname(), nick), fd);
	client.setNickname(nick);
	client.setNICKUsed(true);
	if(client.getRegistered() && !client.getAuth())
		FinishRegistration(input, fd);
}

void Server::PASS(Command input, int fd)
{
	string fullpass = "";
	Client& client = getClient(fd);

	if(client.getRegistered())
		return ServerToUser(ERR_ALREADYREGISTERED(client.getNickname()), fd);

	fullpass = join_strings(input.args);

	if(input.args.size() == 0)
		return ServerToUser(ERR_NEEDMOREPARAMS(input.command, client.getNickname()), fd);

	client.setPassword(fullpass);
	client.setPASSUsed(true);
	if(client.getRegistered())
		FinishRegistration(input, fd);
	return;
}

void Server::USER(Command input, int fd)
{
	Client& client = getClient(fd);
	string valid = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789[]{}\\|";

	if(client.getRegistered())
		return ServerToUser(ERR_ALREADYREGISTERED(client.getNickname()), fd);
	if(input.args.size() != 4)
		return ServerToUser(ERR_NEEDMOREPARAMS(input.command, client.getNickname()), fd);
	if(input.args[1] != "0" || input.args[2] != "*")
		return ServerToUser(ERR_BADPROTOCOL(client.getNickname()), fd);

	string username = input.args[0];
	string realname  = split(input.args[3], ":")[0];

	if(!checkValidChars(strUpper(username), valid) || !checkValidChars(strUpper(realname), valid))
		return;

	client.setUSERUsed(true);
	client.setUsername(username);
	client.setRealname(realname);

	if(client.getRegistered())
		FinishRegistration(input, fd);
}

void Server::JOIN(Command input, int fd)
{
	Client					&client = getClient(fd);
	Channel*				channel;
	std::deque<Client*>*	clientList;
	if (!client.getAuth())
		return ServerToUser(ERR_NEEDPWD(client.getNickname()), fd);
	if (!this->_channels.count(input.args[0]))
	{
		channel = &this->addChannel(input.args[0], client);
	}
	else
	{
		channel = this->getChannel(input.args[0]);
		*clientList = channel->getClients();
		if (!channel->findClient(client.getHostmask()))
			return ServerToUser(ERR_ALREADYONCHANNEL(client.getNickname(), channel->getName()), fd);
		else if (channel->getModes() == 1)
			return ServerToUser(ERR_INVITEONLYCHAN(client.getNickname(), channel->getName()), fd);
		else if (channel->getModes() == 2)
			return ServerToUser(ERR_BADCHANNELKEY(client.getNickname(), channel->getName()), fd);
		else if (clientList->size() == channel->getMaxClients())
			return ServerToUser(ERR_CHANNELISFULL(client.getNickname(), channel->getName()), fd);
		else
			channel->join(client);
	}
	ServerToUser(JOINRPL(client.getHostmask(), input.args[0]), fd);
	if (channel->getTopic() == "")
		ServerToUser(RPL_NOTOPIC(client.getNickname(), channel->getName()), fd);
	else
		ServerToUser(RPL_TOPIC(client.getNickname(), channel->getName(), channel->getTopic()), fd);
	this->NAMES(input, fd);
	return;
}

void	Server::TOPIC(Command input, int fd)
{
	Client		&client = getClient(fd);
	Channel		*channel = getChannel(input.args[0]);
	
	if (!channel)
	{
		if (input.args[0] == SERVER_NAMERPL)
			return ;
		return ServerToUser(ERR_NOSUCHCHANNEL(client.getNickname(), input.args[0]), fd);
	}
	if (input.args.size() == 1)
	{
		if (channel->getTopic() == "")
			return ServerToUser(RPL_NOTOPIC(client.getNickname(), channel->getName()), fd);
		else if (!channel->findClient(client.getHostmask()))
			return ServerToUser(ERR_NOTONCHANNEL(client.getNickname(), channel->getName()), fd);
		return ServerToUser(RPL_TOPIC(client.getNickname(), channel->getName(), channel->getTopic()), fd);
	}
	else
	{
		if (input.getFullCommand().find(':') == string::npos)
			return ServerToUser(ERR_NEEDMOREPARAMS(input.command, client.getNickname()), fd);
		if (!channel->findOperator(client.getHostmask()))
			return ServerToUser(ERR_NOPRIVILEGES(client.getNickname()), fd);
		channel->topic(client, input.getFullCommand());
	}
}

void	Server::NAMES(Command input, int fd)
{
	Client		&client = getClient(fd);
	Channel		*channel = getChannel(input.args[0]);

	if (!channel)
		return ServerToUser(ERR_NOSUCHCHANNEL(client.getNickname(), input.args[0]), fd);
	std::deque<Client*>::const_iterator cl_begin;
	std::deque<Client*>::const_iterator cl_end;
	std::deque<Client*>::const_iterator op_begin;
	std::deque<Client*>::const_iterator op_end;

	cl_begin = channel->getClients().begin();
	cl_end = channel->getClients().end();
	op_begin = channel->getOperators().begin();
	op_end = channel->getOperators().end();
	while (op_begin < op_end)
	{
		Client* op_member = *op_begin;
		std::cout << std::endl << channel->getOperators().size() << std::endl << std::endl;
		ServerToUser(RPL_NAMREPLY(client.getNickname(), string("@"), channel->getName(), op_member->getNickname()), fd);
		op_begin++;
	}
	while (cl_begin < cl_end)
	{
		op_begin = channel->getOperators().begin();
		while (op_begin < op_end && *cl_begin != *op_begin)
			op_begin++;
		if (*cl_begin != *op_begin)
		{
			Client* cl_member = *cl_begin;
			ServerToUser(RPL_NAMREPLY(client.getNickname(), string("+"), channel->getName(), cl_member->getNickname()), fd);
		}
		cl_begin++;
	}
	ServerToUser(RPL_ENDOFNAMES(client.getNickname(), channel->getName()), fd);
}

void	Server::MODE(Command input, int fd)
{
	Client		&client = getClient(fd);
	Channel		*channel = getChannel(input.args[0]);

	if (!channel)
		return ServerToUser(ERR_NOSUCHCHANNEL(client.getNickname(), input.args[0]), fd);
}

void	Server::WHO(Command input, int fd)
{
	if (input.args.empty())
		return ;
	Client		&client = getClient(fd);
	Channel		*channel = getChannel(input.args[0]);

	if (!channel)
		return ServerToUser(ERR_NOSUCHCHANNEL(client.getNickname(), input.args[0]), fd);

	std::deque<Client*>::const_iterator cl_begin;
	std::deque<Client*>::const_iterator cl_end;

	cl_begin = channel->getClients().begin();
	cl_end = channel->getClients().end();

	while (cl_begin < cl_end)
	{
		Client* target_client = *cl_begin;
		std::string status;

		if (channel->isOperator(*target_client))
			status += "@";
		else
			status += "+";
		ServerToUser(RPL_WHOREPLY(
			client.getNickname(),
			channel->getName(),
			target_client->getUsername(),
			target_client->getHostname(),
			SERVER_HOST,
			target_client->getNickname(),
			status,
			target_client->getRealname()
		), fd);
		cl_begin++;
	}
	ServerToUser(RPL_ENDOFWHO(client.getNickname(), channel->getName()), fd);
}