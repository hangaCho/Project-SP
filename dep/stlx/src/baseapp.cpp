#include <baseapp.h>
#include <os/winconsole/console.h>
#include <data/json.h>
#include <os/time.h>



namespace stlx {

BaseApp * g_app = nullptr;

BaseApp::BaseApp(const String & name, int argc, char * argv[], String config_file)
	: _name(name), _config_file(config_file)
{
	g_app = this;
	Time::Update();
	_start_time = Time::Now();
}

BaseApp::~BaseApp() {
	g_con.Close();
	_con_thread.join();
	g_app = nullptr;
}

void BaseApp::Setup() {
	InitConsole();
	try {
		_config = MakeUnique<detail::json>(LoadJson(_config_file));

		auto app = _config->find("app");

		if(app != _config->end()) {
			auto console = app->find("console");

			if(console != app->end()) {
				auto cwnd = GetConsoleWindow();
				auto & con = *console;
				POINT sz = {con["console_size_x"], con["console_size_y"]};
				MoveWindow(cwnd, con["console_pos_x"], con["console_pos_y"],
					sz.x, sz.y, true);
			}
		}

	} catch(...) {
	}

	if(!_config) {
		_config = MakeUnique<detail::json>(detail::json::object());
	}

	Time::Update();
}

void BaseApp::SaveCfg() {
	if(!_config)
		return;

	auto & app = (*_config)["app"];

	{
		// Save console position
		auto cwnd = GetConsoleWindow();
		RECT rect;
		GetWindowRect(cwnd, &rect);

		if (!IsIconic(cwnd)) {
			auto & console = app["console"];
			console["console_pos_x"] = rect.left;
			console["console_pos_y"] = rect.top;
			console["console_size_x"] = rect.right - rect.left;
			console["console_size_y"] = rect.bottom - rect.top;
		}
	}

	std::ofstream file(_config_file, std::ios::out | std::ios::trunc);
	ASSERT(file.is_open());
	
	if(!file.is_open())
		return;

	file << _config->dump(3);
}

void BaseApp::Loop() {
	while(_running) {
		Time::Update();
		OnTick();

		if(_tick_sleep > 0)
			Sleep(_tick_sleep);
	}
}

int BaseApp::Run() {
	try {
		Setup();
	} catch(const std::exception & e) {
		Log::Error("Failed to setup application: ", e.what());
		system("pause");
		return EXIT_FAILURE;
	}

	try {
		Time::Update();
		OnStart();
		Loop();
		OnQuit();
		SaveCfg();
	} catch(const std::exception & e) {
		Log::Error("Exception: {}", e.what());
		system("pause");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void BaseApp::InitConsole() {
	AllocConsole();
	g_con.Init(_T((_name).c_str()), new Console());
	g_con.SetCursorVisible(0);

//	_con_thread = Thread([&]() {
		while(g_con.Run()) {
			Sleep(10);
			g_con.UpdateEvents();
			g_con.CheckErrors();
		}
	};

	PHANDLER_ROUTINE handler = [](DWORD fdwCtrlType) -> BOOL {
		switch(fdwCtrlType) {
		case CTRL_C_EVENT:
		case CTRL_CLOSE_EVENT:
			g_app->Close();
			Sleep(10000); // Wait for the program to close
			return true;
		}

		return false;
	};

//	SetConsoleCtrlHandler(handler, true);
	//SetConsoleCtrlHandler(0, true); // Ignore Ctrl-C
};

