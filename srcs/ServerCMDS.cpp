#include "../headers/Server.hpp"

void Server::initializeFunctions()
{
    functions.insert(std::make_pair("QUIT", &Server::QUIT));
    functions.insert(std::make_pair("PASS", &Server::PASS));
    functions.insert(std::make_pair("NICK", &Server::NICK));
    functions.insert(std::make_pair("USER", &Server::USER));
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
        return ServerToUser(ERR_NEEDPWD(), fd);
    if(it != functions.end())
        (this->*(it->second))(input, fd);
    else if(input.command != "CAP")
        ServerToUser(ERR_COMMANDNOTFND(input.command), fd);
}

void Server::FinishRegistration(Command input, int fd)
{
    Client& client = getClient(fd);
    if(client.getPassword() != this->_password)
    {
        ServerToUser(ERR_PASSWDMISMATCH(), fd);
        QUIT(input, fd);
        return;
    }

    client.setAuth(true);
    cout << "Fd number " << fd << " finished his registration" << endl;
    client.setHostmask(client.getNickname() + "[!" + client.getUsername() + "@" + client.getHostname() + "]");
    ServerToUser(RPL_WELCOME(client.getHostmask()), fd);
    ServerToUser(RPL_YOURHOST(), fd);
    ServerToUser(RPL_CREATED(getCreationDate(), getCreationTime()), fd);
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
        return ServerToUser(ERR_ERRONEUSNICKNAME(nick), fd);

    if(nick == old_nick)
        return ServerToUser(ERR_SAMENICKNAME(), fd);

    std::pair<std::set<string>::iterator, bool> in_use;
    in_use = inUseNicks.insert(nick);
    if(!in_use.second)
    {
        string err_msg = ERR_NICKNAMEINUSE(nick);
        if(!client.getNICKused())
            err_msg += ". Defaulting to \"" + client.getNickname() + "\"";

        ServerToUser(err_msg, fd);
        return;
    }
    inUseNicks.erase(old_nick);
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
        return ServerToUser(ERR_ALREADYREGISTERED(), fd);

    fullpass = join_strings(input.args);

    if(input.args.size() == 0)
        return ServerToUser(ERR_NEEDMOREPARAMS(input.command), fd);

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
        return ServerToUser(ERR_ALREADYREGISTERED(), fd);
    if(input.args.size() != 4)
        return ServerToUser(ERR_NEEDMOREPARAMS(input.command), fd);

    if(input.args[1] != "0" || input.args[2] != "*")
        return ServerToUser(ERR_BADPROTOCOL(), fd);

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