#include "LoginServer.h"
#include "console.h"

extern Ini config;

LoginServer::LoginServer()
{
	//for (int i = 0; i < BUFFER_SIZE; i++)
	  //  buffer[i] = 0;

	serverlen = sizeof(server);
	clientlen = sizeof(client);
}

LoginServer::~LoginServer()
{
	Log::Out("Server Closing");
	WSACleanup();
}

bool LoginServer::Start()
{
	if (WSAStartup(514, &wsaData))
	{
		Log::Error("WSAStartup Error");
		return false;
	}

	server.sin_family = 2;
	u16 port = config.ReadInt("port", 21000, "LOGIN");
	server.sin_port = htons(port);
	server.sin_addr.s_addr = 0;

	listen_socket = socket(2, 1, 0);

	if (listen_socket == INVALID_SOCKET)
	{
		Log::Error("listen_socket Error");
		return false;
	}

	if (::bind(listen_socket, (struct sockaddr*)&server, serverlen) == SOCKET_ERROR)
	{
		Log::Error("bind Error");
		return false;
	}
	else
		Log::Out("listening on port {}", port);


	if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR)
	{
		Log::Error("listen Error");
		return false;
	}

	return true;
}

void Comm(void *args)
{
	SOCKET socket = (SOCKET)args;
	int usr_id = -1;
	u8 buffer[2048];

	while (socket)
	{
		auto retbufsize = recv(socket, (char*)buffer, sizeof(buffer), 0);

		if (!retbufsize)
		{
			Log::Info("Connection closed by client");
			closesocket(socket);
			if (usr_id != -1) {
				Log::Info("User ID : {} Disconnect", usr_id);
			}
			break;
		}

		if (retbufsize == SOCKET_ERROR)
		{
			Log::Info("Client socket closed");
			closesocket(socket);
			if (usr_id != -1) {
				Log::Info("User ID : {} Disconnect", usr_id);
			}
			break;
		}
		else Log::Out("recv {} bytes", retbufsize);

		u32 sz = *(u32*)buffer;
		if (sz != retbufsize)
		{
			Log::Warning("sz != retbufsize ({} != {})", sz, retbufsize);
		}

		//outBuffer();
		try {
			PacketHandler PackHandle(buffer, usr_id);
			if (PackHandle.nOfPackets) {
				Log::Info("Sending Response");
				retbufsize = send(socket, (char*)buffer, PackHandle.ServerResponse(buffer)*buffer[0], 0);
			}
			else Log::Warning("Server has no response");
		}
		catch (const std::exception & e) {
			Log::Error("Error handling client packet: {}", e.what());
		}

	}
	_endthread();
}

bool LoginServer::CommLoop()
{
	bool bExit = false;
	int retbufsize = 0;
	while (!bExit)
	{
		if ((msg_socket = accept(listen_socket, (struct sockaddr*)&client, &clientlen)) == INVALID_SOCKET)
		{
			Log::Error("Accept Error");
			return false;
		}
		else
			Log::Out("Accept Client: {}", inet_ntoa(client.sin_addr));
		_beginthread(Comm, 0, (void*)msg_socket);
	}

	return true;
}

void LoginServer::outBuffer()
{
	/*
	printf("---- Recieved Data From %s  ----\n",inet_ntoa(client.sin_addr));

	for (int i = 0; i < buffer[0]; i++)
	{
		if (i && i%16 == 0)printf("\n");
		if (*(BYTE*)(buffer+i) < 0x10)printf("0");
		printf("%x ",(int)*(BYTE*)(buffer+i));
	}
	printf("\n");
	*/
}
