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
	functions.insert(std::make_pair("PART", &Server::PART));
	functions.insert(std::make_pair("KICK", &Server::KICK));
	functions.insert(std::make_pair("INVITE", &Server::INVITE));
	functions.insert(std::make_pair("PING", &Server::PING));
	functions.insert(std::make_pair("MOTD", &Server::MOTD));
	functions.insert(std::make_pair("PRIVMSG", &Server::PRIVMSG));
}

void Server::PING(Command input, int fd)
{
	if(input.args.size() == 0)
		return ServerToUser(ERR_NEEDMOREPARAMS(input.command, getClient(fd).getNickname()), fd);
	
	ServerToUser(PONG(input.args[0]), fd);
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

void Server::MOTD(Command input, int fd)
{
	(void) input;
	Client& client = getClient(fd);
	ServerToUser(RPL_MOTDSTART(client.getHostmask()), fd);
	
	string file = readFile("srcs/motd.txt");
	std::vector<string> lines = split(file, "\n");
	for(size_t i = 0; i < lines.size(); i++)
		ServerToUser(RPL_MOTD(client.getNickname(), lines[i]), fd);

	ServerToUser(RPL_ENDOFMOTD(client.getHostmask()), fd);
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
	MOTD(input, fd);
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
	if (!client.getAuth())
		return ServerToUser(ERR_NEEDPWD(client.getNickname()), fd);

	if(input.args.size() < 1)
		return ServerToUser(ERR_NEEDMOREPARAMS(input.command, client.getNickname()), fd);

	if (!this->_channels.count(input.args[0]))
	{
		channel = &this->addChannel(input.args[0], client);
	}
	else
	{
		channel = this->getChannel(input.args[0]);
		const std::deque<Client*>& clientList = channel->getClients();
		if (channel->findClient(client.getHostmask()))
			return ServerToUser(ERR_ALREADYONCHANNEL(client.getNickname(), channel->getName()), fd);
		else if (channel->getModes() & MODE_INVITEONLY && !channel->isInvited(client))
			return ServerToUser(ERR_INVITEONLYCHAN(client.getNickname(), channel->getName()), fd);
		else if (channel->getModes() & MODE_KEY && (input.args.size() != 2 || input.args[1] != channel->getPassword()))
			return ServerToUser(ERR_BADCHANNELKEY(client.getNickname(), channel->getName()), fd);
		else if (channel->getModes() & MODE_LIMIT && clientList.size() >= channel->getMaxClients())
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

	if(input.args.size() < 1)
		return ServerToUser(ERR_NEEDMOREPARAMS(input.command, client.getNickname()), fd);

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
		if ((channel->getModes() & MODE_TOPIC) && !channel->findOperator(client.getHostmask()))
			return ServerToUser(ERR_NOPRIVILEGES(client.getNickname()), fd);
		channel->topic(client, input.getFullCommand());
	}
}

void	Server::NAMES(Command input, int fd)
{
	Client		&client = getClient(fd);

	if(input.args.size() < 1)
		return ServerToUser(ERR_NEEDMOREPARAMS(input.command, client.getNickname()), fd);

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
		channel->_bcName(*op_member, "@");
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
			channel->_bcName(*cl_member, "");
		}
		cl_begin++;
	}
	channel->_bcEndName();
}

void	Server::MODE(Command input, int fd)
{
	Client		&client = getClient(fd);
	Channel		*channel = getChannel(input.args[0]);
	string		plus = "";
	string		minus = "";

	if (!channel)
		return ServerToUser(ERR_NOSUCHCHANNEL(client.getNickname(), input.args[0]), fd);
	if (input.args.size() == 1)
	{
		string				parameters = "";
		std::stringstream	ss;
		ss.str(parameters);
		if (channel->getPassword() != "")
			ss << channel->getPassword() << " ";
		if (channel->getMaxClients() != 0)
			ss << channel->getMaxClients();
		getline(ss, parameters);
		ServerToUser(RPL_CHANNELMODEIS(client.getNickname(), channel->getName(), channel->formattedModes(), parameters), fd);
		return ServerToUser(RPL_CREATIONTIME(client.getNickname(), channel->getName(), channel->getTime()), fd);
	}
	else if (!channel->isOperator(client))
		return ServerToUser(ERR_CHANOPRIVSNEEDED(client.getNickname(), channel->getName()), fd);
	try
	{
		getFlags(input, plus, minus);
	}
	catch(int i)
	{
		(void) i;
		return ServerToUser(RPL_UMODEUNKOWNFLAG(client.getNickname()), fd);
	}
	if ((flagExists(plus, 'k') || flagExists(plus, 'o') || flagExists(plus, 'l')) && input.args.size() < 3)
		return ServerToUser(ERR_NEEDMOREPARAMS(input.command, client.getNickname()), fd);
	if (((flagExists(plus, 'k') && flagExists(plus, 'o'))
		|| (flagExists(plus, 'k') && flagExists(plus, 'l'))
		|| (flagExists(plus, 'o') && flagExists(plus, 'l'))) && input.args.size() < 4)
		return ServerToUser(ERR_NEEDMOREPARAMS(input.command, client.getNickname()), fd);
	if (flagExists(plus, 'k') && flagExists(plus, 'o') && flagExists(plus, 'l') && input.args.size() < 5)
		return ServerToUser(ERR_NEEDMOREPARAMS(input.command, client.getNickname()), fd);
	
	try
	{
		input.plus = plus;
		input.minus = minus;
		flagOrder(input);
	}
	catch (int e)
	{
		(void) e;
		return ServerToUser(RPL_UMODEUNKOWNFLAG(client.getNickname()), fd);
	}
	channel->changeModes(client, input);
}

void	Server::getFlags(Command& input, string& plus, string& minus)
{
	string				aux;
	std::stringstream	ss;

	aux = input.args[1];
	if (!uniqueChar(aux, 'i') || !uniqueChar(aux, 'o') || !uniqueChar(aux, 'k') || !uniqueChar(aux, 'l') || !uniqueChar(aux, 't') || !uniqueChar(aux, '+') || !uniqueChar(aux, '-'))
		throw (-1);
	ss.str(input.args[1]);
	getline(ss, aux, '+');
	getline(ss, plus);
	try
	{
		parsePlus(plus);
	}
	catch(int i)
	{
		__throw_exception_again;
	}
	ss.clear();
	ss.str(input.args[1]);
	getline(ss, aux, '-');
	getline(ss, minus);
	try
	{
		parseMinus(minus);
	}
	catch(int i)
	{
		__throw_exception_again;
	}
	aux = input.args[1];
}

void	Server::flagOrder(Command& input)
{
	string str = input.args[1];
	int	i = 0;
	int j = 0;

	while (str[i])
	{
		if (validFlag(str[i]))
		{
			input.flags[i] = str[i];
			if (str[i] == 'l' && input.plus.find(str[i]) != string::npos)
			{
				checkLimit(input.args[2 + j]);
				input.flagArgs[string(&str[i])] = input.args[2 + j++];
			}
			else if (str[i] == 'k' && input.plus.find(str[i]) != string::npos)
			{
				checkKey(input.args[2 + j]);
				input.flagArgs[string(&str[i])] = input.args[2 + j++];
			}
			else if (str[i] == 'o')
				input.flagArgs[string(&str[i])] = input.args[2 + j++];
		}
		i++;
	}
}

void	Server::checkLimit(string& str)
{
	int	i;

	i = 0;
	while (str[i])
	{
		if (!std::isdigit(str[i]))
			throw (-1);
		i++;
	}
	long l = strtol(str.c_str(), NULL, 10);
	if (l > std::numeric_limits<int>::max() || l < 1)
		throw(-1);
}

void	Server::checkKey(string& str)
{
	int i = 0;
	while (str[i])
	{
		if (std::iswblank(str[i]))
			throw (-1);
		if (str.size() < 4)
			throw (-1);
		i++;
	}
}

void	Server::parsePlus(string& plus)
{
	int					i = -1;
	std::stringstream	ss;

	ss.str(plus);
	while (plus[++i])
	{
		if (!validFlag(plus[i]))
		{
			if (plus[i] != '-')
				throw (-1);
			else
			{
				getline(ss, plus, '-');
				return ;
			}
		}
	}
	getline(ss, plus);
	return ;
}

void	Server::parseMinus(string& minus)
{
	int					i = -1;
	std::stringstream	ss;

	ss.str(minus);
	while (minus[++i])
	{
		if (!validFlag(minus[i]))
		{
			if (minus[i] != '+')
				throw (-1);
			else
			{
				getline(ss, minus, '+');
				return ;
			}
		}
	}
	getline(ss, minus);
	return ;
}

bool	Server::validFlag(char c)
{
	if (c == 'i' || c == 't' || c == 'k' || c == 'o' || c == 'l')
		return (true);
	return (false);
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

void Server::PART(Command input, int fd)
{
	Client		&client = getClient(fd);
	string reason = join_strings(input.args, 1);
	if(reason == "")
	{
		reason = "Leaving";
	}

	if(input.args.size() < 1)
		return ServerToUser(ERR_NEEDMOREPARAMS(input.command, client.getNickname()), fd);

	std::vector<string> all_channels = split(input.args[0], ",");
	
	for(size_t i = 0; i < all_channels.size(); i++)
	{
		Channel		*channel = getChannel(all_channels[i]);
		if (!channel)
		{
			ServerToUser(ERR_NOSUCHCHANNEL(client.getNickname(), input.args[0]), fd);
			continue;
		}
		if (!channel->findClient(client.getHostmask()))
		{
			ServerToUser(ERR_NOTONCHANNEL(client.getNickname(), channel->getName()), fd);
			continue;
		}

		channel->removeUser(client, reason);
	}
}

void	Server::KICK(Command input, int fd)
{
	Client &client = getClient(fd);
	string kicker = client.getHostmask();
	Channel *channel = getChannel(input.args[0]);
	string reason = join_strings(input.args, 2);
	if(reason == "")
		reason = "Behave";

	if(input.args.size() < 2)
		return ServerToUser(ERR_NEEDMOREPARAMS(input.command, client.getNickname()), fd);

	if(!channel)
		return ServerToUser(ERR_NOSUCHCHANNEL(client.getNickname(), input.args[0]), fd);

	if(!channel->findClient(client.getHostmask()))
		return ServerToUser(ERR_NOTONCHANNEL(client.getNickname(), channel->getName()), fd);

	if(!channel->findOperator(client.getHostmask()))
		return ServerToUser(ERR_CHANOPRIVSNEEDED(client.getNickname(), channel->getName()), fd);

	Client *target = channel->findClientByNick(input.args[1]);
	if(!target)
		return ServerToUser(ERR_USERNOTINCHANNEL(client.getNickname(), input.args[1], channel->getName()), fd);
	channel->kick(kicker, target->getNickname(), reason);
}

void	Server::INVITE(Command Input, int fd)
{
	Client& client = getClient(fd);

	if(Input.args.size() < 2)
		return ServerToUser(ERR_NEEDMOREPARAMS(Input.command, client.getNickname()), fd);

	Channel* channel = getChannel(Input.args[1]);


	if(!channel)
		return ServerToUser(ERR_NOSUCHCHANNEL(client.getNickname(), Input.args[1]), fd);

	if(channel->findClientByNick(Input.args[0]))
		return ServerToUser(ERR_USERONCHANNEL(client.getNickname(), Input.args[0], channel->getName()), fd);

	Client* target = getClientByNick(Input.args[0]);	
	if(!target)
		return ServerToUser(ERR_USERNOTFOUND(client.getHostmask(), Input.args[0]), fd);

	if(!channel->findClient(client.getHostmask()))
		return ServerToUser(ERR_NOTONCHANNEL(client.getNickname(), channel->getName()), fd);
	

	ServerToUser(RPL_INVITING(client.getNickname(), target->getNickname(), channel->getName()), fd);
	ServerToUser(INVITERPL(client.getHostmask(), target->getNickname(), channel->getName()), target->getFd());
	channel->invite(target);
}

void		Server::PRIVMSG(Command input, int fd)
{
	Client&		client = getClient(fd);

	if(input.args.size() < 1)
		return ServerToUser(ERR_NEEDMOREPARAMS(input.command, client.getNickname()), fd);

	std::stringstream ss;
	std::string	message;
	
	ss.str(input.getFullCommand());
	getline(ss, message, ':');
	getline(ss, message);
	
	if (input.args[0][0] == '#')
	{
		Channel*	target = getChannel(input.args[0]);
		if(!target)
			return ServerToUser(ERR_NOSUCHCHANNEL(client.getNickname(), input.args[0]), fd);
		if (!target->findClient(client.getHostmask()))
			return ServerToUser(ERR_NOTONCHANNEL(client.getNickname(), target->getName()), fd);
		target->bcMessage(client, message);
	}
	else
	{
		Client*		target = this->getClientByNick(input.args[0]);
		if(!target)
			return ServerToUser(ERR_USERNOTFOUND(client.getHostmask(), input.args[0]), fd);
		return ServerToUser(PRIVMSGRPL(client.getHostmask(), target->getNickname(), message), target->getFd());
	}
}