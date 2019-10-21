#include "LoginServer.h"
Ini config("..\\config.ini", "CONFIG"); //For debug
//Ini config(".//config.ini", "CONFIG"); //For Running
HANDLE hConsoleOutput;

int main()
{
	hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	//printf("%x\n",sizeof(TrainingDoneResponse));
	UniquePtr<LoginServer> login_server = MakeUnique<LoginServer>();

	if (login_server->Start())
		printf("----- Server Started -----\n");

	login_server->CommLoop();
	return 0;
}
