#pragma once

#include "Server.hpp"

//STANDARD RESPONSE FORMAT
#define SERVER_RESPONSE(numeric)							(SERVER_NAMERPL + string(" ") + numeric + string(" :"))
#define CLIENT_RESPONSE(numeric, nickname)					(SERVER_NAMERPL + string(" ") + numeric + string(" ") + nickname + string(" :"))
#define CLCHAN_RESPONSE(numeric, nickname, channel)			(SERVER_NAMERPL + string(" ") + numeric + string(" ") + nickname + string(" ") + channel + string(" :"))
#define CLEXTR_RESPONSE(numeric, nickname, extra)			(SERVER_NAMERPL + string(" ") + numeric + string(" ") + nickname + string(" ") + extra + string(" :"))
#define CLEXCH_RESPONSE(numeric, nickname, extra, channel)	(SERVER_NAMERPL + string(" ") + numeric + string(" ") + nickname + string(" ") + extra + string(" ") + channel + string(" :"))

//USER COMMAND AND MESSAGE

#define CLIENT_COMMAND(hostmask, command, target)			(string(":") + hostmask + string(" ") + command + string(" ") + target)
#define CLIENT_MESSAGE(hostmask, target, message)			(CLIENT_COMMAND(hostmask, string("PRIVMSG"), target) + string(" :") + message)

//ERRORS
#define ERR_NOSUCHCHANNEL(nickname, channel)				(CLCHAN_RESPONSE(string("403"), nickname, channel) + string("No such channel"))
#define ERR_NOTONCHANNEL(nickname, channel)					(CLCHAN_RESPONSE(string("442"), nickname, channel) + string("You're not on that channel"))
#define ERR_NEEDMOREPARAMS(command, nickname)				(CLIENT_RESPONSE(string("461"), nickname) + command + string(" - Not enough parameters"))
#define ERR_ALREADYREGISTERED(nickname)						(CLIENT_RESPONSE(string("462"), nickname) + string("Already Registered"))
#define ERR_NEEDPWD(nickname)								(CLIENT_RESPONSE(string("463"), nickname) + string("Please use PASS before attempting any other command"))
#define ERR_NOPRIVILEGES(nickname)							(CLIENT_RESPONSE(string("481"), nickname) + string("Permission Denied- You're not an IRC operator"))
#define ERR_CHANOPRIVSNEEDED(nickname, channel)				(CLCHAN_RESPONSE(string("482"), nickname, channel) + string("You're not channel operator"))
#define ERR_COMMANDNOTFND(command, nickname)				(CLIENT_RESPONSE(string("987"), nickname) + command + string(" - Command not found"))
#define	ERR_USERNOTFOUND(nickname, target)					(CLEXTR_RESPONSE(string("981"), nickname, target) + string("No such nick/channel."))
#define ERR_USERONCHANNEL(hostmask, target, channel)		(CLEXCH_RESPONSE(string("443"), hostmask, target, channel) + string("is already on channel."))

//Welcome Messages
#define RPL_WELCOME(hostmask, nickname)						(CLIENT_RESPONSE(string("001"), nickname) + string("Welcome to the ") + SERVER_NAME + string(" Network, ") + hostmask + string("!"))
#define RPL_YOURHOST(nickname)								(CLIENT_RESPONSE(string("002"), nickname) + string("Your host is ") + SERVER_NAME + string(", running version 1.0"))
#define RPL_CREATED(date, time, nickname)					(CLIENT_RESPONSE(string("003"), nickname) + string("This server was created ") + date + string(" at ") + time)

//NICK
#define NAMECHANGE(hostmask, target)						(CLIENT_COMMAND(hostmask, string("NICK"), target))
#define ERR_NONICKNAMEGIVEN(nickname)						(CLIENT_RESPONSE(string("431"), nickname) + string(" No nickname given."))
#define ERR_ERRONEUSNICKNAME(nickname, target)				(CLEXTR_RESPONSE(string("432"), nickname, target) + string("Erroneous nickname."))
#define ERR_NICKNAMEINUSE(nickname, target)					(CLEXTR_RESPONSE(string("433"), nickname, target) + string("Nickname is already in use."))
#define ERR_SAMENICKNAME(nickname, target)					(CLEXTR_RESPONSE(string("985"), nickname, target) + string("You already have that nickname."))

//PASS
#define ERR_PASSWDMISMATCH(nickname)						(CLIENT_RESPONSE(string("464"), nickname) + string("Password incorrect."))

//USER
#define ERR_BADPROTOCOL(nickname)							(CLIENT_RESPONSE(string("986"), nickname) + string("Bad protocol ( [USER] 0 * [realname] )"))
#define ERR_INVALIDUSER(nickname)							(CLIENT_RESPONSE(string("983"), nickname) + string("Invalid username. Only alphanumeric characters are acceptable."))

//JOIN
#define JOINRPL(hostmask, channel)							(CLIENT_COMMAND(hostmask, string("JOIN"), channel))
#define ERR_CHANNELISFULL(nickname, channel)				(CLCHAN_RESPONSE(string("471"), nickname, channel) + string("Channel is full. (+l)"))
#define ERR_INVITEONLYCHAN(nickname, channel)				(CLCHAN_RESPONSE(string("473"), nickname, channel) + string("Cannot join channel. (+i)"))
#define ERR_BADCHANNELKEY(nickname, channel)				(CLCHAN_RESPONSE(string("475"), nickname, channel) + string("Cannot join channel. (+k)"))
#define ERR_INVALIDKEY(nickname, channel)					(CLCHAN_RESPONSE(string("525"), nickname, channel) + string("Key is not well-formed."))
#define ERR_ALREADYONCHANNEL(nickname, channel)				(CLCHAN_RESPONSE(string("984"), nickname, channel) + string("Already on channel."))
#define ERR_ERRONEUSCHANNAME(channel_name, target)			(CLEXTR_RESPONSE(string("976"), channel_name, target) + string("Erroneous Channel Name."))

//TOPIC
#define TOPICRPL(hostmask, channel, topic)					(CLIENT_COMMAND(hostmask, string("TOPIC"), channel) + string(" ") + topic)
#define RPL_NOTOPIC(nickname, channel)						(CLCHAN_RESPONSE(string("331"), nickname, channel) + "No Topic is set")
#define RPL_TOPIC(nickname, channel, topic)					(CLCHAN_RESPONSE(string("332"), nickname, channel) + topic)

//NAMES
#define RPL_NAMREPLY(nickname, prefix, channel, member)		(CLEXCH_RESPONSE(string("353"), nickname, string("="), channel) + prefix + member)
#define RPL_ENDOFNAMES(nickname, channel)					(CLCHAN_RESPONSE(string("366"), nickname, channel) + string("End of /NAMES list"))

//PART
#define PARTRPL(hostmask, channel, reason)					(CLIENT_COMMAND(hostmask, string("PART"), channel) + string(" ") + reason)

//KICK
#define KICKRPL(hostmask, channel, target, reason)			(CLIENT_COMMAND(hostmask, string("KICK"), channel) + string(" ") + target + string(" :") + reason)
#define ERR_USERNOTINCHANNEL(nickname, target, channel)		(CLEXCH_RESPONSE(string("441"), nickname, target, channel) + string("They aren't on that channel."))

//INVITE
#define RPL_INVITING(hostmask, channel, target)				(CLIENT_COMMAND(hostmask, string("INVITE"), channel) + string(" ") + target)
#define INVITERPL(issuer, target, channel)					(CLIENT_COMMAND(issuer, string("INVITE"), target) + string(" ") + channel)

//MODE
#define MODERPL(hostmask, target, flag)						(CLIENT_COMMAND(hostmask, string("MODE"), target) + flag)
#define RPL_CHANNELMODEIS(nickname, channel, modes, args)	(CLCHAN_RESPONSE(string("324"), nickname, channel) + modes + string(" ") + args)
#define RPL_CREATIONTIME(nickname, channel, creationtime)	(SERVER_NAMERPL + string(" 329 ") + nickname + string(" ") + channel + string(" ") + creationtime)
#define RPL_UMODEUNKOWNFLAG(nickname)						(CLIENT_RESPONSE(string("501"), nickname) + string("Unknown MODE flag"))

//MOTD
#define RPL_MOTDSTART(hostmask)								(CLIENT_RESPONSE(string("375"), hostmask) + string(" :- ") + SERVER_NAME + string(" Message of the Day - "))
#define RPL_MOTD(hostmask, line)							(CLIENT_RESPONSE(string("372"), hostmask) + string(" :") + line)
#define RPL_ENDOFMOTD(hostmask)								(CLIENT_RESPONSE(string("376"), hostmask) + string("End of /MOTD command."))

//PONG
#define PONG(token)											(SERVER_NAMERPL + string(" PONG ") + SERVER_NAME + " " + token)

//WHO
#define WHORPL(hostmask, target)							(CLIENT_COMMAND(hostmask, string("WHO"), target))
#define RPL_WHOREPLY(cli_nickname, channel, t_user, t_host, t_server, t_nick, t_status, t_real) \
															(CLCHAN_RESPONSE(string("352"), cli_nickname, channel) + t_user + string(" ") + t_host + string(" ") + t_server + string(" ") + t_nick + string(" ") + t_status + string(" :0 ") + t_real)
#define RPL_ENDOFWHO(nickname, channel)						(CLCHAN_RESPONSE(string("315"), nickname, channel) + string("End of WHO list"))

//PRIVMSG
#define PRIVMSGRPL(hostmask, target, message)				(CLIENT_COMMAND(hostmask, string("PRIVMSG"), target) + string(" :") + message)


#define MODE_INVITEONLY	0b0001
#define MODE_LIMIT		0b0010
#define MODE_KEY		0b0100
#define MODE_TOPIC		0b1000