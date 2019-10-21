#pragma once

#include <stlxtypes.h>
#include <data/json.h>
#include <os/time.h>

namespace stlx {

class BaseApp {
public:
	BaseApp(const String & name, int argc, char * argv[], String config_file = "config.json"s);
	~BaseApp();

	int Run();
	void Close() { _running = false; }
	virtual void OnStart() {}
	virtual void OnTick() {}
	virtual void OnQuit() {}

	auto GetStartTime()	const { return _start_time; }
	bool IsRunning()		const { return _running; }
	void SetTickSleep(u32 duration)			{ _tick_sleep = duration; }
	
	const String _name;
	UniquePtr<detail::json> _config;

protected:
	virtual void Setup();

	const String _config_file;
	u32 _tick_sleep = 1;
	bool _running = true;

private:
	void InitConsole();
	virtual void Loop();

	void SaveCfg();

	std::thread _con_thread;
	Time::Val _start_time;

};



};