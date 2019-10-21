#include "PacketHandler.h"
#include <time.h>

extern Ini config;

IOSocket cIOSocket;
int PacketHandler::nOfPackets = 0, PacketHandler::State = -1;

PacketHandler::PacketHandler(unsigned char* buf, int &socket_usr_id)
{
	nOfPackets = 0;
	Decrypt(buf);

	switch (IdentifyPacketType(buf))
	{
	case LOGIN_REQ:
		Login_Info = (LoginInfo*)buf;
		Log::Info("Client login attempt: {} // {}", Login_Info->username, Login_Info->password);
		GenerateResponse(LOGIN_RESPONSE);
		socket_usr_id = usr_id;
		break;
	case SERVERINFO_REQ:
	case AFTER_SERVERINFO_REQ:
		Server_Info_Request = (ServerInfoRequest*)buf;
		Log::Info("Client Requesting Server Info");
		GenerateResponse(SERVERINFO_RESPONSE);
		break;
	case CHANGE_DEFAULT_CHARACTER_REQ:
		Default_Character_Change_Request = (DefaultCharacterChangeRequest*)buf;
		Log::Info("Client Requesting To Change Character");
		GenerateResponse(CHANGE_DEFAULT_CHARACTER_RESPONSE);
		break;
	case TRAINING_DONE_REQ:
		Log::Info("TRAINING_DONE_REQ");
		GenerateResponse(TRAINING_DONE_RESPONSE);
		break;
	case LOGIN_STATUS_REQ:
		Login_Status_Request = (LoginStatusRequest*)buf;
		Log::Info("LOGIN_STATUS_REQ");
		if (Login_Status_Request->status == 1) {
			Log::Info("Client Logout : {} ", Login_Status_Request->username);
		}
		else {
			Log::Info("Client Quit Channel : {} ", Login_Status_Request->username);			
		}
		break;
	}
}

PacketHandler::~PacketHandler()
{
	nOfPackets = 0;
}

int PacketHandler::UpdateState()
{
	return State = abs(((State = (~State + 0x14fb) * 0x1f) >> 16) ^ State);
}

int PacketHandler::IdentifyPacketType(unsigned char* buf)
{
	return *(int*)(buf + 4);
}

void PacketHandler::Encrypt(unsigned char *buf)
{
	for (int i = 4; i < buf[0] * nOfPackets; i++)
	{
		if (i == buf[0]) i += 4;
		buf[i] = ~((BYTE)(buf[i] << 3) | (BYTE)(buf[i] >> 5));
	}


	printf("---- Encrypted Data  ----\n");

	for (int i = 0; i < buf[0]; i++)
	{
		if (i && i % 16 == 0)printf("\n");
		if (*(BYTE*)(buf + i) < 0x10)printf("0");
		printf("%x ", (int)*(BYTE*)(buf + i));
	}
	printf("\n");
}

void PacketHandler::Decrypt(unsigned char *buf)
{
	for (int i = 4; i < buf[0]; i++)
		buf[i] = ~((BYTE)(buf[i] >> 3) | (BYTE)(buf[i] << 5));

	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);

	printf("---- Decrypted Data  ----\n");
	printf("Time: %s", asctime(timeinfo));
	for (int i = 0; i < buf[0]; i++)
	{
		if (i && i % 16 == 0)printf("\n");
		if (*(BYTE*)(buf + i) < 0x10)printf("0");
		printf("%x ", (int)*(BYTE*)(buf + i));
	}
	printf("\n");
}

bool PacketHandler::IsCorrectLogin()
{
	if (usr_id = MySql.Login(Login_Info->username, Login_Info->password))return true;
	return false;
}

void PacketHandler::GetCharInfo(char *charname, MyCharInfo &Info)
{
	if (usr_id)MySql.GetUserInfo(usr_id, Info);
}

void PacketHandler::SetDefaultCharacter(Character DefaultCharacter)
{
	if (usr_id)MySql.SetDefaultCharacter(usr_id, DefaultCharacter);
}

void PacketHandler::GenerateResponse(int ResponsePacketType)
{
	nOfPackets = 1;
	switch (ResponsePacketType)
	{	
	case LOGIN_RESPONSE:
		Login_Response.size = 0xA8;
		Login_Response.type = LOGIN_RESPONSE;
		Login_Response.unk1 = 11036;
		Login_Response.state = UpdateState();
		if (!IsCorrectLogin())
		{
			Login_Response.Response = WrongPasswd;
		}
		else
		{
			Login_Response.Response = CorrectPasswd;
			MyCharInfo UsrInfo;
			GetCharInfo(Login_Info->username, UsrInfo);
			Login_Response.UserType = UsrInfo.UserType;
			*(int*)&Login_Response.DefaultCharacter = UsrInfo.DefaultCharacter;
			Login_Response.UserLevel1 = UsrInfo.Level;
			Login_Response.UserLevel2 = UsrInfo.Level;
			Login_Response.AgeCheck = 1;
			Login_Response.Points = UsrInfo.Points;
			Login_Response.Code = UsrInfo.Code;
			memset(&Login_Response.unk5, 0, 20 * 4);
		}
		Login_Response.checksum = cIOSocket.MakeDigest((unsigned char*)&Login_Response);
		buffer = (unsigned char*)&Login_Response;
		break;

	case SERVERINFO_RESPONSE:

		//by_hyun �ҽ� ����(ä�� ���� ����)
		Server_Info[0].size = 0x68;
		Server_Info[0].type = Server_Info_Request->type + 1;
		Server_Info[0].unk1 = 11036;
		Server_Info[0].state = UpdateState();
		
		if (Server_Info_Request->serverType == 1)
		{
			config.SetSection("CHANNEL1");
			Server_Info[0].channel = 1;
			Server_Info[0].servernumber = 0;

			string str = config.ReadString("ip", "121.200.112.133");
			strcpy(Server_Info[0].ip, str.c_str());

			Server_Info[0].port = config.ReadInt("port", 9303);
			Server_Info[0].serverload = 0;
			str = config.ReadString("name", " ���� 500��/ ���� 50�� ");
			strcpy(Server_Info[0].chname, str.c_str());
			str = config.ReadString("guild", " A Guild");
			strcpy(Server_Info[0].gname, str.c_str());

					
		}
		else if (Server_Info_Request->serverType == 2)
		{
			config.SetSection("CHANNEL2");
			Server_Info[0].channel = 2;
			Server_Info[0].servernumber = 0;

			string str = config.ReadString("ip", "121.200.112.133");
			strcpy(Server_Info[0].ip, str.c_str());

			Server_Info[0].port = config.ReadInt("port", 9304);
			Server_Info[0].serverload = 0;
			str = config.ReadString("name", " ���� 1000��/ ���� 100��  ");
			strcpy(Server_Info[0].chname, str.c_str());
			str = config.ReadString("guild", " B Guild ");
			strcpy(Server_Info[0].gname, str.c_str());			
			
			/*
			config.SetSection("CHANNEL5");
			Server_Info[1].size = 0x68;
			Server_Info[1].type = Server_Info_Request->type + 1;
			Server_Info[1].unk1 = 11036;
			Server_Info[1].state = UpdateState();
			Server_Info[1].channel = 2;
			Server_Info[1].servernumber = 1;

			str = config.ReadString("ip", "119.201.80.130");
			strcpy(Server_Info[1].ip, str.c_str());

			Server_Info[1].port = config.ReadInt("port", 9304);
			Server_Info[1].serverload = 0;

			str = config.ReadString("name", "Test ChannelX4 2");
			strcpy(Server_Info[1].chname, str.c_str());
			str = config.ReadString("guild", " CGuild");
			strcpy(Server_Info[1].gname, str.c_str());

			Server_Info[1].checksum = cIOSocket.MakeDigest((unsigned char*)&Server_Info[1]);

			nOfPackets = 2; // �ʴ� ���ϴ� ������???
			*/
			
		}
		else if (Server_Info_Request->serverType == 3)
		{
			config.SetSection("CHANNEL3");
			Server_Info[0].channel = 3;
			Server_Info[0].servernumber = 0;

			string str = config.ReadString("ip", "121.200.112.133");
			strcpy(Server_Info[0].ip, str.c_str());

			Server_Info[0].port = config.ReadInt("port", 9305);
			Server_Info[0].serverload = 0;

			str = config.ReadString("name", " ���� 2000��/ ���� 150�� ");
			strcpy(Server_Info[0].chname, str.c_str());
			str = config.ReadString("guild", " D Guild ");
			strcpy(Server_Info[0].gname, str.c_str());
			
			/*
			config.SetSection("CHANNEL4");
			Server_Info[1].size = 0x68;
			Server_Info[1].type = Server_Info_Request->type + 1;
			Server_Info[1].unk1 = 11036;
			Server_Info[1].state = UpdateState();
			Server_Info[1].channel = 3;
			Server_Info[1].servernumber = 1;

			str = config.ReadString("ip", "119.201.80.130");
			strcpy(Server_Info[1].ip, str.c_str());

			Server_Info[1].port = config.ReadInt("port", 9305);
			Server_Info[1].serverload = 0;

			str = config.ReadString("name", "Test ChannelX10 2");
			strcpy(Server_Info[1].chname, str.c_str());
			str = config.ReadString("guild", " EGuild");
			strcpy(Server_Info[1].gname, str.c_str());

			Server_Info[1].checksum = cIOSocket.MakeDigest((unsigned char*)&Server_Info[1]);

			nOfPackets = 2; // �ʴ� ���ϴ� ������???
			*/
			
		}

		Server_Info[0].checksum = cIOSocket.MakeDigest((unsigned char*)&Server_Info[0]);

		nOfPackets = 1; // �ʴ� ���ϴ� ������???(�� ä�δ� ������ ������ ���ϴ°� ����) 
		//han : ���� ��Ŷ ���������� 1 = ������ 0 = no ������, � ���������� �� �𸣰���.
		buffer = (unsigned char*)&Server_Info;
		break;
	case CHANGE_DEFAULT_CHARACTER_RESPONSE:
		Default_Character_Change_Response.size = 0x1C;
		Default_Character_Change_Response.type = CHANGE_DEFAULT_CHARACTER_RESPONSE;
		Default_Character_Change_Response.unk1 = 11036;
		Default_Character_Change_Response.state = UpdateState();
		Default_Character_Change_Response.unk2 = 1;
		Default_Character_Change_Response.character = Default_Character_Change_Request->character;
		Default_Character_Change_Response.checksum = cIOSocket.MakeDigest((unsigned char*)&Default_Character_Change_Response);
		buffer = (unsigned char*)&Default_Character_Change_Response;
		MySql.SetDefaultCharacter(usr_id, Default_Character_Change_Request->character);
		break;
	case TRAINING_DONE_RESPONSE:
		TrainingDone_Response.size = 0x98;
		TrainingDone_Response.type = TRAINING_DONE_RESPONSE;
		TrainingDone_Response.unk1 = 11036;
		{
			int r = Random::UInt(1, 3);
			for (int i = 0; i < 9; i++) {
				TrainingDone_Response.items[i] = t_items[i] + (r * 10);
				TrainingDone_Response.zeros[i] = 0;
				TrainingDone_Response.gf[i] = 100;
			}
			TrainingDone_Response.items[9] = t_items[9] + r;
			TrainingDone_Response.zeros[9] = 0;
			TrainingDone_Response.gf[9] = 30;
		}
		TrainingDone_Response.unk2 = -1;
		TrainingDone_Response.unk3 = 1005;
		TrainingDone_Response.unk4 = 0;
		TrainingDone_Response.state = UpdateState();
		TrainingDone_Response.checksum = cIOSocket.MakeDigest((unsigned char*)&TrainingDone_Response);
		buffer = (unsigned char*)&TrainingDone_Response;
		break;
	default:
		break;
	}
}

char* PacketHandler::ServerResponse()
{
	Encrypt(buffer);
	return (char*)buffer;
}

int PacketHandler::ServerResponse(unsigned char* buf)
{
	Encrypt(buffer);
	memcpy(buf, buffer, buffer[0] * nOfPackets);

	return nOfPackets;
}

