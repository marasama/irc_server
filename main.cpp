#include "include/Server.hpp"
#include "include/Client.hpp"
#include "include/Bot.hpp"

int main(int argc, char *argv[])
{
	if (argc != 3) {
	std::cerr << "Usage: ./server <port> <password>\n";
		return 1;
	}

	int port = std::atoi(argv[1]);
	if (port <= 0 || port > 65535) {
		std::cerr << "Invalid port\n";
		return 1;
	}

	std::string password = argv[2];

	try {
		Server	srv(AF_INET, SOCK_STREAM, port, "42 Kocaeli IRC Sunucusu");
		srv.setSrvPass(password); // Şifreyi ayarlaC
		srv.serverRun();
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	return 0;
}
