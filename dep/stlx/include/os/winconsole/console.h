#pragma once

#include "consoleimpl.h"
#include <stlxtypes.h>
#include <format.h>

namespace stlx {

extern ConsoleImpl g_con;

enum class ConsoleType { Out, Log, Warning, Error };

template<ConsoleType T>
class ConsoleOut {
public:
	// overload for ostream, handles std::endl
	ConsoleOut<T> & operator <<(std::ostream&(*f)(std::ostream&)) {
	#ifdef NO_LOGS
		if(T == ConsoleType::Log) return *this;
	#endif
		if(f == endl)
			g_con.Write('\n');

		// TODO: Optional output to file here
		return *this;
	}
	ConsoleOut<T> & operator <<(const char * txt) {
	#ifdef NO_LOGS
		if(T == ConsoleType::Log) return *this;
	#endif
		SetColour();
		g_con.Write(txt);
		return *this;
	}
	ConsoleOut<T> & operator <<(const std::string & txt) {
	#ifdef NO_LOGS
		if(T == ConsoleType::Log) return *this;
	#endif
		SetColour();
		g_con.Write(txt.c_str());
		return *this;
	}
	ConsoleOut<T> & operator <<(u8 val) {
	#ifdef NO_LOGS
		if(T == ConsoleType::Log) return *this;
	#endif
		char txt[32];
		sprintf(txt, "%u", val);

		SetColour();
		g_con.Write(txt);
		return *this;
	}
	ConsoleOut<T> & operator <<(u16 val) {
	#ifdef NO_LOGS
		if(T == ConsoleType::Log) return *this;
	#endif
		char txt[32];
		sprintf(txt, "%u", val);

		SetColour();
		g_con.Write(txt);
		return *this;
	}
	ConsoleOut<T> & operator <<(u32 val) {
	#ifdef NO_LOGS
		if(T == ConsoleType::Log) return *this;
	#endif
		char txt[32];
		sprintf(txt, "%u", val);

		SetColour();
		g_con.Write(txt);
		return *this;
	}
	ConsoleOut<T> & operator <<(u64 val) {
	#ifdef NO_LOGS
		if(T == ConsoleType::Log) return *this;
	#endif
		char txt[32];
		sprintf(txt, "%llu", val);

		SetColour();
		g_con.Write(txt);
		return *this;
	}
	ConsoleOut<T> & operator <<(i8 val) {
	#ifdef NO_LOGS
		if(T == ConsoleType::Log) return *this;
	#endif
		char txt[32];
		sprintf(txt, "%i", val);

		SetColour();
		g_con.Write(txt);
		return *this;
	}
	ConsoleOut<T> & operator <<(i16 val) {
	#ifdef NO_LOGS
		if(T == ConsoleType::Log) return *this;
	#endif
		char txt[32];
		sprintf(txt, "%i", val);

		SetColour();
		g_con.Write(txt);
		return *this;
	}
	ConsoleOut<T> & operator <<(i32 val) {
	#ifdef NO_LOGS
		if(T == ConsoleType::Log) return *this;
	#endif
		char txt[32];
		sprintf(txt, "%i", val);

		SetColour();
		g_con.Write(txt);
		return *this;
	}
	ConsoleOut<T> & operator <<(i64 val) {
	#ifdef NO_LOGS
		if(T == ConsoleType::Log) return *this;
	#endif
		char txt[32];
		sprintf(txt, "%lld", val);

		SetColour();
		g_con.Write(txt);
		return *this;
	}
	ConsoleOut<T> & operator <<(f64 val) {
	#ifdef NO_LOGS
		if(T == ConsoleType::Log) return *this;
	#endif
		char txt[32];
		sprintf(txt, "%f", val);

		SetColour();
		g_con.Write(txt);
		return *this;
	}

	// 	template <typename E, typename = std::enable_if_t<std::is_enum<E>::value>>
	// 	ConsoleOut<T> & operator <<(E e) {
	// 		*this << (std::underlying_type_t<E>)e;
	// 		return *this;
	// 	}


	void SetColour() {
		switch(T) {
		case ConsoleType::Out:
			g_con.SetWriteColor(COLOR_GREEN);
			break;

		case ConsoleType::Log:
			g_con.SetWriteColor(COLOR_GREY);
			break;

		case ConsoleType::Warning:
			g_con.SetWriteColor(COLOR_YELLOW);
			break;

		case ConsoleType::Error:
			g_con.SetWriteColor(COLOR_RED);
			break;
		}
	}
};

namespace Log {
	extern ConsoleOut< ConsoleType::Out > cout;
	extern ConsoleOut< ConsoleType::Log > clog;
	extern ConsoleOut< ConsoleType::Warning > cwar;
	extern ConsoleOut< ConsoleType::Error > cerr;

	template <typename ...Args>
	void Info(Args &&... args) {
		clog << fmt::format(std::forward<Args>(args)...) << "\n";
	}
	template <typename ...Args>
	void Out(Args &&... args) {
		cout << fmt::format(std::forward<Args>(args)...) << "\n";
	}
	template <typename ...Args>
	void Warning(Args &&... args) {
		cwar << fmt::format(std::forward<Args>(args)...) << "\n";
	}
	template <typename ...Args>
	void Error(Args &&... args) {
		cerr << fmt::format(std::forward<Args>(args)...) << "\n";
	}
};

class Console : public EventCallback {
private:
	ConsoleImpl * c;
	//unsigned int ContinueButton;

	void OnUpdate();
	void OnKeyInput(const KeyboardEvent * Event);
	void OnInit();
	void OnMouseInput(const MouseEvent * Event);
	void OnClose();
	void OnButtonPress(const ButtonEvent * Event);

public:
	Console() { c = 0; }
	~Console() {
		c = 0;
	}
	void OnEvent(const ConsoleEvent * Event) {
		if((!c) && (!(c = cns(Event))))
			return;
		if(Event->EventType == ConsoleImpl::EVENT_FRAMEUPDATE) {
			OnUpdate();
		} else if(Event->EventType == ConsoleImpl::EVENT_INIT) {
			OnInit();
		} else if(Event->EventType == ConsoleImpl::EVENT_KEY_INPUT) {
			OnKeyInput(&(Event->Keyb));
		} else if(Event->EventType == ConsoleImpl::EVENT_MOUSE_INPUT) {
			OnMouseInput(&(Event->Mouse));
		} else if(Event->EventType == ConsoleImpl::EVENT_CLOSE) {
			OnClose();
		} else if(Event->EventType == ConsoleImpl::EVENT_BUTTONPRESS) {
			OnButtonPress(&(Event->Btn));
		}
	}
};

template<class _Elem, class _Traits> FORCEINLINE
auto & __CLRCALL_OR_CDECL endl(std::basic_ostream<_Elem, _Traits>& str) {
	return std::endl(str);
}

};