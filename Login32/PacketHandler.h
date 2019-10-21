#ifndef __PACKETHANDLER_H__
#define __PACKETHANDLER_H__

#include "IOSocket.h"
#include "ServerEnum.h"
#include "ClientPackets.h"
#include "ServerPackets.h"
#include "MySQL.h"

class PacketHandler {
private:
	int usr_id;
	unsigned char* buffer;
	MySQL MySql;

	// Client Packet Structures
	LoginInfo *Login_Info;
	ServerInfoRequest *Server_Info_Request;
	DefaultCharacterChangeRequest *Default_Character_Change_Request;
	LoginStatusRequest *Login_Status_Request;

	// Server Packet Structures
	LoginResponse Login_Response;
	ServerInfo Server_Info[3];
	DefaultCharacterChangeResponse Default_Character_Change_Response;
	TrainingDoneResponse TrainingDone_Response;


public:
	PacketHandler(unsigned char*, int&);
	~PacketHandler();
	int UpdateState();
	int IdentifyPacketType();
	int IdentifyPacketType(unsigned char*);
	void Encrypt(unsigned char*);
	void Decrypt(unsigned char*);
	bool IsCorrectLogin();
	void GetCharInfo(char *, MyCharInfo &);
	void SetDefaultCharacter(Character);
	void GenerateResponse(int);
	char* ServerResponse();
	int ServerResponse(unsigned char*);

	static int nOfPackets, State;
};

#endif
