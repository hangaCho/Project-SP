#include "ChannelServer.h"
#include "console.h"

IOSocket cIOSocket2;

ChannelServer::ChannelServer()
{
	//for (int i = 0; i < BUFFER_SIZE; i++)
	//  buffer[i] = 0;

	serverlen = sizeof(server);
	clientlen = sizeof(client);
}

ChannelServer::~ChannelServer()
{
	WSACleanup();
	//    delete buffer;
}

bool ChannelServer::Start(int port)
{
	if (WSAStartup(514, &wsaData))
	{
		Log::Error("WSAStartup Error");
		return false;
	}

	server.sin_family = 2;
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

struct _args {
	SOCKET msg_socket;
	char LastIP[20];
};

void Comm(void *args)
{
	_args *a = (_args*)args;
	{
		SOCKET msg_socket = (SOCKET)a->msg_socket;
		PacketHandler PackHandle(msg_socket, a->LastIP);
		int retbufsize = 0, n = 0;
		unsigned char *buffer = new unsigned char[9000];
		while (msg_socket)
		{
			retbufsize = recv(msg_socket, (char*)buffer, 9000, 0);

			if (!retbufsize)
			{
				Log::Info("Connection closed by client");
				closesocket(msg_socket);
				break;
			}

			if (retbufsize == SOCKET_ERROR)
			{
				Log::Info("Client socket closed");
				closesocket(msg_socket);
				break;
			}
			else Log::Out("recv {} bytes", retbufsize);


			i32 sz = *(i32*)buffer;
			if (sz != retbufsize)
			{
				Log::Warning("sz != retbufsize ({} != {})", sz, retbufsize);
			}
			//ASSERT(sz == retbufsize);

			try {
				if(n == 0) {
					n = 1;
					PackHandle.Handle(buffer);
					cout << "Sending First Response" << endl;
					retbufsize = send(msg_socket, (char*)buffer, PackHandle.ServerResponse(buffer)*buffer[0], 0);

					PackHandle.GenerateResponse(JOIN_MISSIONLEVEL_RESPONSE);
					PackHandle.ServerResponse(buffer);
					retbufsize = send(msg_socket, (char*)buffer, *(int*)buffer, 0);

					PackHandle.GenerateResponse(JOIN_PLAYERDATA_RESPONSE);

					memcpy(buffer, (unsigned char*)&PackHandle.Join_Channel_PlayerData_Response, 0x980);
					*(int*)(buffer + 0xc) = cIOSocket2.MakeDigest((u8*)buffer);

					for(int i = 4; i < (*(int*)buffer); i++)
						buffer[i] = ~((BYTE)(buffer[i] << 3) | (BYTE)(buffer[i] >> 5));

					retbufsize = send(msg_socket, (char*)buffer, 0x980, 0);

					PackHandle.GenerateResponse(LOBBY_USERINFO_RESPONSE);
					int x = PackHandle.ServerResponse(buffer);
					send(msg_socket, (char*)buffer, x, 0);

					PackHandle.GenerateResponse(ROOM_LIST_RESPONSE);
					PackHandle.ServerResponse(buffer);
					send(msg_socket, (char*)buffer, *(int*)buffer, 0);

					continue;
				}


				PackHandle.Handle(buffer);
				if(PackHandle.nOfPackets) {
					Log::Info("Sending Response");
					int x = PackHandle.ServerResponse(buffer);
					if(x < 10)
						retbufsize = send(msg_socket, (char*)buffer, *(int*)buffer, 0);
					else retbufsize = send(msg_socket, (char*)buffer, x, 0);

				} else Log::Warning("Server has no response");
			} catch (const std::exception & e) {
				Log::Error("Error handling client packet: {}", e.what());
			}
		}
		delete buffer;
	}
	_endthread();
}

bool ChannelServer::CommLoop()
{
	bool bExit = false;
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)PacketHandler::startUDP, 0, 0, 0);
	_args args;
	while (!bExit)
	{
		if ((msg_socket = accept(listen_socket, (struct sockaddr*)&client, &clientlen)) == INVALID_SOCKET)
		{
			Log::Error("Accept Error");
			return false;
		}
		else
		{
			args.msg_socket = msg_socket;
			strcpy(args.LastIP, inet_ntoa(client.sin_addr));
			Log::Out("Accept Client: {}", args.LastIP);
		}
		_beginthread((void(*)(void *))Comm, 0, (void *)&args);
		Sleep(50);
	}

	return true;
}

void ChannelServer::outBuffer()
{
	/*
	Log::Info("---- Received Data From {} ----",inet_ntoa(client.sin_addr));

	for (int i = 0; i < buffer[0]; i++)
	{
		if (i && i%16 == 0)printf("\n");
		if (*(BYTE*)(buffer+i) < 0x10)printf("0");
		printf("%x ",(int)*(BYTE*)(buffer+i));
	}
	printf("\n");
	*/
}

