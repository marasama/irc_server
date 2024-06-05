#include "../include/Server.hpp"

void Server::processPartialCommands(int clientSocketFD)
{
	string& clientBuffer = clientBuffers[clientSocketFD].getBuffer();
	size_t endOfCommand;
	string command;

	if (clientBuffer[0] == '/')
	{
		while ((endOfCommand = clientBuffer.find('\n')) != string::npos)
		{
			command = clientBuffer.substr(0, endOfCommand);
			clientBuffer.erase(0, endOfCommand + 1);
			CommandParser::commandParser(command.c_str(), _clients[clientSocketFD], this);
		}
		if (!clientBuffer.find('\n'))
			clientBuffer.clear();
	}

	else
	{
		while ((endOfCommand = clientBuffer.find("\r\n")) != string::npos)
		{
			command = clientBuffer.substr(0, endOfCommand);
			clientBuffer.erase(0, endOfCommand + 2);  // '\r\n' karakterlerini sil
			CommandParser::commandParser(command.c_str(), _clients[clientSocketFD], this);
		}
		clientBuffer.clear();
	}
}

void Server::handleClient(int clientSocketFD)
{
		const size_t BUFFER_SIZE = 512;
		char tempBuffer[BUFFER_SIZE];
		memset(tempBuffer, 0, BUFFER_SIZE);

		ssize_t received = recv(clientSocketFD, tempBuffer, BUFFER_SIZE - 1, 0);
		if (received > 0) {
			clientBuffers[clientSocketFD].appendtoBuffer(string(tempBuffer, received));
			cout << "Received: " << tempBuffer << endl;
			processPartialCommands(clientSocketFD);
		} else if (received == 0 || errno == ECONNRESET) {
			FD_CLR(clientSocketFD, &read_set);
			clientDisconnect(clientSocketFD);
			clientBuffers.erase(clientSocketFD);
		} else {
			if (errno != EAGAIN && errno != EWOULDBLOCK) {
				FD_CLR(clientSocketFD, &read_set);
				ErrorLogger("recv error", __FILE__, __LINE__);
				close(clientSocketFD);
				clientBuffers.erase(clientSocketFD);
			}
		}
}

void Server::clientDisconnect(int clientSocketFD)
{
     try
    {
        std::map<int, Client*>::iterator it = _clients.find(clientSocketFD);
        if (it == _clients.end()) {
            std::cout << "Client not found for removal.\n";
            return;
        }

        removeClientFromAllChannels(it->second);
        it->second->leave();

        ostringstream messageStreamDisconnect;
        messageStreamDisconnect << "Client " << it->second->getNickName() << " has disconnected.";
        log(messageStreamDisconnect.str());

        FD_CLR(clientSocketFD, &read_set);

        close(clientSocketFD);
        delete it->second;
        _clients.erase(it);
    }
    catch (const std::exception &e)
    {
        std::cout << e.what() << std::endl;
    }
}

void Server::setSrvPass(const string& pass) {
	_serverPass = pass;
}

bool Server::verifySrvPass(const string& pass) {
	if (_serverPass == pass) {
		return true;
	}
	return false;
}

void Server::signalHandler(int signum)
{
	Server::getInstance()->shutdownSrv();
	exit(signum);
}

void Server::shutdownSrv()
{
	string outmessage = "Shutting down the server...\n";
	std::cout <<  outmessage.c_str();

	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		Client* client = it->second;
		if (client != NULL) {
			client->sendMessage("Shutting down the server. Your connection is being terminated.");
			removeClientFromAllChannels(client);
			close(it->first);
			delete client;
		}
	}
	_clients.clear();

	if (_serverSocketFD != -1) {
		close(_serverSocketFD);
		_serverSocketFD = -1;
	}

	if (_bot != NULL) {
		delete _bot;
		_bot = NULL;
	}

	FD_ZERO(&read_set);
	string outmessage2 = "Server shut down.\n";
	std::cout << outmessage2.c_str();
}
