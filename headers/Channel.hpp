/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: txisto-d <txisto-d@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/17 12:10:55 by txisto-d          #+#    #+#             */
/*   Updated: 2025/02/17 22:16:07 by txisto-d         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once 

#include "general.hpp"
#include "ServerToClient.hpp"
#include "Client.hpp"
#include "Server.hpp"

class Server;
class Client;

class Channel
{
	private:
		std::string			_name;
		std::string			_password;
		std::string			_topic;
		Server*				_server;
		unsigned char		_modes;
		size_t				_maxClient;
		std::deque<Client*>	_clientList;
		std::deque<Client*>	_operatorList;
		void				_broadcast(Client& client, string message);
		void				_bcJoin(Client& client);
		void				_bcTopic(Client& client);
		void				_bcMessage(Client& source, std::string message);
		
	public:
		Channel();
		Channel(std::string name, Server& server, Client& client);
		Channel(const Channel& obj);
		Channel& operator=(const Channel& obj);
		~Channel();

		void				join(Client& client);
		void				topic(Client& source, std::string message = "");
		void				kick(Client& source, std::string target);
		void				invite(Client& source, std::string target);
		void				changeModes(Client& source, unsigned char mode);
		std::string			getName() const;
		std::string			getPassword() const;
		std::string			getTopic() const;
		Server&				getServer() const;
		unsigned char		getModes() const;
		std::deque<Client*>	getClients() const;
		std::deque<Client*>	getOperators() const;
		size_t				getMaxClients() const;
		Client*				findClient(std::string hostmask);
		Client*				findOperator(std::string hostmask);
};