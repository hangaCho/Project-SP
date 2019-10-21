#include "ChannelServer.h"
#define SAVELOG 0

Ini config("..\\config.ini", "CONFIG"); //For debug
//Ini config(".//config.ini", "CONFIG"); //For Running
HANDLE hConsoleOutput;

int main()
{

	streambuf* coutBuf = cout.rdbuf();
	ofstream of("packets.txt");
	streambuf* fileBuf = of.rdbuf();
	if (SAVELOG)
	{
		cout.rdbuf(fileBuf);
	}
	//cout << hex << sizeof(QuestGainResponse) << endl;
	hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);

	{
		UniquePtr<ChannelServer> channel_server = MakeUnique<ChannelServer>();

		config.SetSection("CHANNELS");
		u32 port = config.ReadInt("port", 9305);

		if(channel_server->Start(port)) {
			Log::Out("----- Channel Server Started -----\n");
		}
		channel_server->CommLoop();
	}

	cout << endl;
	if (SAVELOG)
	{
		of.flush();
		of.close();
		cout.rdbuf(coutBuf);
	}
	cout << "Server Closing" << endl;
	cin.get();
	return 0;
}