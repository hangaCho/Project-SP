#ifndef __LOGINSERVER_H__
#define __LOGINSERVER_H__

#define BUFFER_SIZE     1000

#include "PacketHandler.h"

class LoginServer {
public:
	LoginServer();
	~LoginServer();
	bool Start();
	bool CommLoop();
	void outBuffer();

private:
	//unsigned char buffer[BUFFER_SIZE];
	WSADATA wsaData;
	struct sockaddr_in server, client;
	int serverlen, clientlen;
	SOCKET listen_socket, msg_socket;
};


#endif
