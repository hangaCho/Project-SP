#ifndef CONSOLE_H
#define CONSOLE_H
#include <iostream>
#include <windows.h>
#include <atomic>
#include <concurrency.h>


namespace Log {
	static const void _write(const string & str, WORD color = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED) {
		static std::atomic_flag _mutex;
		lock_guard_flag lock(_mutex);

		HANDLE hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);

		SetConsoleTextAttribute(hConsoleOutput,
			color | FOREGROUND_INTENSITY);

		cout << str << endl;
		SetConsoleTextAttribute(hConsoleOutput, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
	}

	template <typename ...Args>
	void Info(Args &&... args) {
		_write(fmt::format(std::forward<Args>(args)...));
	}
	template <typename ...Args>
	void Out(Args &&... args) {
		_write(fmt::format(std::forward<Args>(args)...), FOREGROUND_GREEN);
	}
	template <typename ...Args>
	void Warning(Args &&... args) {
		_write(fmt::format(std::forward<Args>(args)...), FOREGROUND_GREEN | FOREGROUND_RED);
	}
	template <typename ...Args>
	void Error(Args &&... args) {
		_write(fmt::format(std::forward<Args>(args)...), FOREGROUND_RED);
	}
}

#endif // CONSOLE_H
