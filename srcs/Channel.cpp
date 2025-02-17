/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: txisto-d <txisto-d@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/17 12:10:35 by txisto-d          #+#    #+#             */
/*   Updated: 2025/02/18 14:22:07 by txisto-d         ###   ########.fr       */
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
	this->_maxClient = 12;
}

Channel::Channel(std::string name, Server& server, Client& client)
{
	this->_name = name;
	this->_password = "";
	this->_topic = "";
	this->_server = &server;
	this->_modes = 0;
	this->_maxClient = 12;
	this->_clientList.push_back(&client);
	this->_operatorList.push_back(&client);
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

void	Channel::kick(Client& client, std::string target)
{
	(void) client;
	(void) target;
}

void	Channel::invite(Client& client, std::string target)
{
	(void) client;
	(void) target;
}

void	Channel::changeModes(Client& client, unsigned char mode)
{
	(void) client;
	(void) mode;
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

const std::deque<Client*>&	Channel::getClients()  const
{
	return (this->_clientList);
}

const std::deque<Client*>&	Channel::getOperators()  const
{
	return (this->_operatorList);
}

size_t	Channel::getMaxClients()  const
{
	return (this->_maxClient);
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

Client*				Channel::findClient(std::string hostmask)
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

Client*				Channel::findOperator(std::string hostmask)
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