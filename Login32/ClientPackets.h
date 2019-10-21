#ifndef __CLIENTPACKETS_H__
#define __CLIENTPACKETS_H__

#define LOGIN_REQ    9991
#define SERVERINFO_REQ    10503
#define AFTER_SERVERINFO_REQ    10519
#define CHANGE_DEFAULT_CHARACTER_REQ    10513
#define TRAINING_DONE_REQ    10517
#define LOGIN_STATUS_REQ    10515

struct LoginInfo {
	int size;
	int type;
	int unk1; //11036
	int checksum;
	int state;
	unsigned char verhash[36];
	int vercode1;
	char username[13];
	char password[13];
	short unk2;
	int unk3[11];
	int pid;
};

struct ServerInfoRequest {
	int size;
	int type;
	int unk1; //11036
	int checksum;
	int state;
	int unk2;
	int serverType;
};

struct DefaultCharacterChangeRequest {
	int size;
	int type;
	int unk1; //11036
	int checksum;
	int state;
	int unk2[4];
	Character character;
};

struct TrainingDoneRequest {
	int size;
	int type;
	int unk1; //11036
	int checksum;
	int state;
};

struct LoginStatusRequest {
	int size;
	int type;
	int unk1; //11036
	int checksum;
	int state;
	char username[6]; //(first 6 byte of username)
	short unk2; // ( seem to be fixed when different status for same account)
	int unk3; //(seem to be fixed when different status for same account)
	int unk4;
	int status; //1 = quit login server, 2 = channel back to login server
	int unk5;
	int unk6;
};

#endif
