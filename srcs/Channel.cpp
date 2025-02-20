/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: txisto-d <txisto-d@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/17 12:10:35 by txisto-d          #+#    #+#             */
/*   Updated: 2025/02/20 14:23:59 by txisto-d         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"


Channel::Channel()
{
	this->_name = "";
	this->_password = "";
	this->_topic = "";
	this->_server = NULL;
	this->_modes = 0;
	this->_maxClient = 0;
	this->_creationTime = CurrentDate() + string(" ") + CurrentTime();
}

Channel::Channel(std::string name, Server& server, Client& client)
{
	this->_name = name;
	this->_password = "";
	this->_topic = "";
	this->_server = &server;
	this->_modes = 0;
	this->_maxClient = 0;
	this->_clientList.push_back(&client);
	this->_operatorList.push_back(&client);
	this->_inviteList.push_back(&client);
	this->_creationTime = CurrentDate() + string(" ") + CurrentTime();
}

Channel::Channel(const Channel& obj)
{
	this->_name = obj.getName();
	this->_password = obj.getPassword();
	this->_topic = obj.getTopic();
	this->_server = &obj.getServer();
	this->_modes = obj.getModes();
	this->_maxClient = obj.getMaxClients();
	this->_clientList = obj.getClients();
	this->_operatorList = obj.getOperators();
	this->_inviteList = obj.getInviteList();
	this->_creationTime = obj.getTime();
}

Channel& Channel::operator=(const Channel& obj)
{
	if (this != & obj)
	{
		this->_name = obj.getName();
		this->_password = obj.getPassword();
		this->_topic = obj.getTopic();
		this->_server = &obj.getServer();
		this->_modes = obj.getModes();
		this->_maxClient = obj.getMaxClients();
		this->_clientList = obj.getClients();
		this->_operatorList = obj.getOperators();
		this->_creationTime = obj.getTime();
	}
	return (*this);
}

Channel::~Channel()
{

}

void	Channel::join(Client& client)
{
	this->_clientList.push_back(&client);
	this->_bcJoin(client);
}

void	Channel::topic(Client& client, std::string message)
{
	std::stringstream ss;
	std::string	aux;
	
	ss.str(message);
	getline(ss, aux, ':');
	getline(ss, aux);
	
	this->_topic = aux;
	this->_bcTopic(client);
}

void	Channel::kick(string kicker, string target, string reason)
{
	Client* target_client = this->findClientByNick(target);

	this->_broadcast(*this->findClient(kicker), KICKRPL(kicker, this->_name, target, reason));

	_clientList.erase(find(_clientList.begin(), _clientList.end(), target_client));
	if(this->isOperator(*target_client))
		_operatorList.erase(find(_operatorList.begin(), _operatorList.end(), target_client));
}

void	Channel::invite(Client* target)
{
	if(find(_inviteList.begin(), _inviteList.end(), target) == _inviteList.end())
		_inviteList.push_back(target);
}

void	Channel::changeModes(Client& client, Command& input)
{
	(void) client;
	std::map<int, string>::iterator begin = input.flags.begin();
	std::map<int, string>::iterator end = input.flags.end();
	while (begin != end)
	{
		if ((*begin).second == "i" && input.plus.find('i') != string::npos)
			this->_modes |= MODE_INVITEONLY;
		else if ((*begin).second == "i" && input.minus.find('i')  != string::npos)
			this->_modes &= ~MODE_INVITEONLY;
		if ((*begin).second == "t"  && input.plus.find('t') != string::npos)
			this->_modes |= MODE_TOPIC;
		else if ((*begin).second == "t" && input.minus.find('t')  != string::npos)
			this->_modes &= ~MODE_TOPIC;
		if ((*begin).second == "l"  && input.plus.find('l') != string::npos)
		{
			this->_maxClient = std::strtol(input.flagArgs["l"].c_str(), NULL, 10);
			this->_modes |= MODE_LIMIT;
		}
		else if ((*begin).second == "l" && input.minus.find('l') != string::npos)
		{
			this->_maxClient = 0;
			this->_modes &= ~MODE_LIMIT;
		}
		if ((*begin).second == "k" && input.plus.find('k') != string::npos)
		{
			this->_password = input.flagArgs["k"];
			this->_modes |= MODE_KEY;
		}
		else if ((*begin).second == "k" && input.minus.find('k') != string::npos)
		{
			this->_password = "";
			this->_modes &= ~MODE_KEY;
		}
		if ((*begin).second == "o"  && input.plus.find('k') != string::npos)
		{
			Client* cl = (this->findClientByNick(input.flagArgs["o"]));
			if (!cl)
				this->_server->ServerToUser(ERR_NOTONCHANNEL(client.getNickname(), this->_name), client.getFd());
			else if (!this->isOperator(*cl))
				this->addOperator(*cl);
		}
		else if ((*begin).second == "o"  && input.minus.find('k') != string::npos)
		{
			Client* cl = (this->findClientByNick(input.flagArgs["o"]));
			if (!cl)
				this->_server->ServerToUser(ERR_NOTONCHANNEL(client.getNickname(), this->_name), client.getFd());
			else if (this->isOperator(*cl))
				this->_operatorList.erase(find(this->_operatorList.begin(), this->_operatorList.end(), cl));
		}
		begin++;
	}
	string	all_args = "";
	int		arg_size = input.args.size();
	int		i = 0;
	while (i < arg_size)
	{
		all_args+= input.args[i];
		all_args += string(" ");
		i++;
	}
	this->_server->ServerToUser(SERVER_NAMERPL + string(" 324 ") + client.getNickname() + string(" ") + all_args, client.getFd());
}



std::string	Channel::getName()  const
{
	return (this->_name);
}

std::string	Channel::getPassword()  const
{
	return (this->_password);
}

std::string	Channel::getTopic()  const
{
	return (this->_topic);
}

Server&	Channel::getServer() const
{
	return (*this->_server);
}

unsigned char	Channel::getModes()  const
{
	return (this->_modes);
}

string	Channel::formattedModes()
{
	string	str= "";
	if (this->_modes & MODE_INVITEONLY)
		str += "i";
	if (this->_modes & MODE_LIMIT)
		str += "l";
	if (this->_modes & MODE_KEY)
		str += "k";
	if (this->_modes & MODE_TOPIC)
		str += "t";
	return (str);
}

void	Channel::addOperator(Client& client)
{
	if (this->isOperator(client))
		this->_operatorList.push_back(&client);
}

const std::deque<Client*>&	Channel::getClients()  const
{
	return (this->_clientList);
}

const std::deque<Client*>&	Channel::getOperators()  const
{
	return (this->_operatorList);
}

const std::deque<Client*>&	Channel::getInviteList()  const
{
	return (this->_inviteList);
}

size_t	Channel::getMaxClients()  const
{
	return (this->_maxClient);
}

string	Channel::getTime() const
{
	return (this->_creationTime);
}

void	Channel::removeUser(Client& client, string reason)
{
	std::deque<Client*>::iterator begin;
	std::deque<Client*>::iterator end;
	_bcPart(client, reason);
	
	begin = this->_clientList.begin();
	end = this->_clientList.end();
	while (begin != end)
	{
		if ((*begin)->getHostmask() == client.getHostmask())
		{
			this->_clientList.erase(begin);
			break;
		}
		begin++;
	}
	begin = this->_operatorList.begin();
	end = this->_operatorList.end();
	while (begin != end)
	{
		if ((*begin)->getHostmask() == client.getHostmask())
		{
			this->_operatorList.erase(begin);
			break;
		}
		begin++;
	}

	begin = _clientList.begin();
	end = _clientList.end();

}

Client*				Channel::findClientByNick(std::string nickname)
{
	std::deque<Client*>::iterator begin;
	std::deque<Client*>::iterator end;
	
	begin = this->_clientList.begin();
	end = this->_clientList.end();
	while (begin < end)
	{
		if ((*begin)->getNickname() == nickname)
			return (*begin);
		begin++;
	}
	return (NULL);
}

Client*	Channel::findClient(std::string hostmask)
{
	std::deque<Client*>::iterator begin;
	std::deque<Client*>::iterator end;
	
	begin = this->_clientList.begin();
	end = this->_clientList.end();
	while (begin < end)
	{
		if ((*begin)->getHostmask() == hostmask)
			return (*begin);
		begin++;
	}
	return (NULL);
}

Client*	Channel::findOperator(std::string hostmask)
{
	std::deque<Client*>::iterator begin;
	std::deque<Client*>::iterator end;

	begin = this->_operatorList.begin();
	end = this->_operatorList.end();
	while (begin != end)
	{
		if ((*begin)->getHostmask() == hostmask)
			return (*begin);
	}
	return (NULL);
}

bool	Channel::isOperator(Client& client)
{
	const std::deque<Client*>& list = this->getOperators();
	std::deque<Client*>::const_iterator op_begin;
	std::deque<Client*>::const_iterator op_end;

	op_begin = list.begin();
	op_end = list.end();
	while (op_begin < op_end)
	{
		if ((*op_begin)->getHostmask() == client.getHostmask())
			return (true);
		op_begin++;
	}
	return (false);
}

bool	Channel::isInvited(Client& client)
{
	const std::deque<Client*>& list = this->getInviteList();
	std::deque<Client*>::const_iterator iv_begin;
	std::deque<Client*>::const_iterator iv_end;

	iv_begin = list.begin();
	iv_end = list.end();
	while (iv_begin < iv_end)
	{
		if ((*iv_begin)->getHostmask() == client.getHostmask())
			return (true);
		iv_begin++;
	}
	return (false);
}

void	Channel::_broadcast(Client& client, string message)
{
	std::deque<Client*>::iterator begin;
	std::deque<Client*>::iterator end;
	
	begin = this->_clientList.begin();
	end = this->_clientList.end();
	while (begin < end)
	{
		Client* member = *begin;
		if (member->getFd() != client.getFd())
			(*this->_server).ServerToUser(message, member->getFd());
		begin++;
	}
}

void	Channel::_bcJoin(Client& client)
{
	this->_broadcast(client, JOINRPL(client.getHostmask(), this->_name));
}

void	Channel::_bcTopic(Client& client)
{
	this->_broadcast(client, TOPICRPL(client.getHostmask(), this->_name, this->_topic));
}

void	Channel::_bcMessage(Client& client, std::string message)
{
	this->_broadcast(client, CLIENT_MESSAGE(client.getHostmask(), this->_name, message));
}

void	Channel::_fullBroadcast(string message)
{
	std::deque<Client*>::iterator begin;
	std::deque<Client*>::iterator end;
	
	begin = this->_clientList.begin();
	end = this->_clientList.end();
	while (begin < end)
	{
		Client* member = *begin;
		(*this->_server).ServerToUser(message, member->getFd());
		begin++;
	}
}

void	Channel::_bcName(Client& client, std::string prefix)
{
	std::deque<Client*>::iterator begin;
	std::deque<Client*>::iterator end;
	
	begin = this->_clientList.begin();
	end = this->_clientList.end();
	while (begin < end)
	{
		Client* member = *begin;
		this->_fullBroadcast(RPL_NAMREPLY(member->getNickname(), prefix, this->_name, client.getNickname()));
		begin++;
	}
}

void	Channel::_bcEndName()
{
	std::deque<Client*>::iterator begin;
	std::deque<Client*>::iterator end;
	
	begin = this->_clientList.begin();
	end = this->_clientList.end();
	while (begin < end)
	{
		Client* member = *begin;
		this->_fullBroadcast(RPL_ENDOFNAMES(member->getNickname(), this->_name));
		begin++;
	}
}

void	Channel::_bcPart(Client& client, std::string reason)
{
	this->_fullBroadcast(PARTRPL(client.getHostmask(), this->_name, reason));
}