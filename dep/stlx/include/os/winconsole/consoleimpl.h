#pragma once

#define _NTDDSCRD_H2_
#define _WINSCARD_H_
#include <Windows.h>
#include <fstream>
#include <io.h>
#include <tchar.h>

#include <assert.h>
#include "AlignedMemoryAlloc.h"

#ifdef _COMPILE_WITH_AUDIO_
#	include <bass.h>
#	ifdef _MSC_VER
#		pragma comment(lib, "bass.lib")
#	endif
#endif

namespace stlx {

struct MouseEvent {
	short PosX;
	short PosY;
	bool Buttons[5];
	bool wasButtons[5];
};
struct Button {
	SMALL_RECT Pos;
	bool isEnabled;
};
struct ButtonEvent {
	Button * Btn;
	short PosX;
	short PosY;
	unsigned int Id;
};
struct KeyboardEvent {
	unsigned short Key;
	bool Down;
	bool * isKeyDown;
	bool * wasKeyDown;
};
struct ConsoleEvent {
	unsigned short EventType;
	MouseEvent Mouse;
	KeyboardEvent Keyb;
	ButtonEvent Btn;
	void * Console;
};
class EventCallback {
public:
	virtual void OnEvent(const ConsoleEvent *){}
};
enum Colors {
	COLOR_LAST = -1,
	COLOR_BLACK = 0,
	COLOR_DEFAULT = 0,
	COLOR_DARK_BLUE = 1,
	COLOR_DARK_GREEN = 2,
	COLOR_DARK_AQUA = 3,
	COLOR_DARK_CYAN = 3,
	COLOR_DARK_RED = 4,
	COLOR_DARK_PURPLE = 5,
	COLOR_DARK_PINK = 5,
	COLOR_DARK_MAGENTA = 5,
	COLOR_DARK_YELLOW = 6,
	COLOR_DARK_WHITE = 7,
	COLOR_GREY = 8,
	COLOR_BLUE = 9,
	COLOR_GREEN = 10,
	COLOR_AQUA = 11,
	COLOR_CYAN = 11,
	COLOR_RED = 12,
	COLOR_PURPLE = 13,
	COLOR_PINK = 13,
	COLOR_MAGENTA = 13,
	COLOR_YELLOW = 14,
	COLOR_WHITE = 15
};
#ifdef _COMPILE_WITH_AUDIO_
typedef DWORD SOUND;
class Sound {
private:
	SOUND Snd;
public:
	Sound(const TCHAR * Filename, const bool Play = 0)
	{
		LoadSnd(Filename);
		if(Play)
			this->Play();
	}
	Sound(SOUND Stream, const bool Play = 0)
	{
		this->Snd = Stream;
		if(Play)
			this->Play();
	}
	Sound() {
		this->Snd = 0;
	}
	float GetFreqRangeLevel(float FreqStart = 0, float FreqEnd = 48000)
	{
		float Buf[1024];
		unsigned long usesize = 1024;
		if(!this->GetData(Buf,usesize))
		{
			usesize = 512;
			if(!this->GetData(Buf,usesize))
			{
				usesize = 256;
				if(!this->GetData(Buf,usesize))
				{
					float level = 0.f;
					WORD low, high;
					DWORD dw = this->Level();
					low = LOWORD(dw);
					high = HIWORD(dw);
					dw = low + high;
					if(dw > 0)
						level = ((float)dw) / ( 32768.f * 2.f );
					return level;
				}
			}
		}

	}
	bool GetData(float * Buffer, DWORD Size)
	{
		DWORD Flag = BASS_DATA_FLOAT;
		if(Size == 128)
			Flag |= BASS_DATA_FFT256;
		else if(Size == 256)
			Flag |= BASS_DATA_FFT512;
		else if(Size == 512)
			Flag |= BASS_DATA_FFT1024;
		else if(Size == 1024)
			Flag |= BASS_DATA_FFT2048;
		else if(Size == 2048)
			Flag |= BASS_DATA_FFT4096;
		else if(Size == 4096)
			Flag |= BASS_DATA_FFT8192;
		else if(Size == 8192)
			Flag |= BASS_DATA_FFT16384;
		else
			Flag |= Size;
		return BASS_ChannelGetData(this->Snd,Buffer,Flag) > 0;
	}
	~Sound() {
		this->Free();
	}
	void Free()
	{
		if(this->Snd) {
			this->Stop();
			BASS_StreamFree(this->Snd);
			this->Snd = 0;
		}
	}
	void SetSound(SOUND Stream)
	{
		Free();
		this->Snd = Stream;
	}
	void Stop()
	{
		if(this->isPlaying())
			BASS_ChannelStop(this->Snd);
	}
	double Length() {
		QWORD result = BASS_ChannelGetLength(this->Snd,BASS_POS_BYTE);
		if(!result) return 0;
		return BASS_ChannelBytes2Seconds(this->Snd,result);
	}
	double GetPos() {
		QWORD result = BASS_ChannelGetPosition(this->Snd,BASS_POS_BYTE);
		if(!result) return 0;
		return BASS_ChannelBytes2Seconds(this->Snd,result);
	}
	DWORD Level() {
		return BASS_ChannelGetLevel(this->Snd);
	}
	DWORD GetStream()
	{
		return this->Snd;
	}
	float GetVolume()
	{
		float vl = 0.f;
		return ( (BASS_ChannelGetAttribute(this->Snd,BASS_ATTRIB_VOL,&vl)>0) ? vl : 0.f );
	}
	void SetVolume(float Volume = 1.f)
	{
		BASS_ChannelSetAttribute(this->Snd,BASS_ATTRIB_VOL,Volume);
	}
	void SetPos(double Position) {
		QWORD BytesPos = BASS_ChannelSeconds2Bytes(this->Snd,Position);
		if(!BytesPos) return;
		BASS_ChannelSetPosition(this->Snd,BytesPos,BASS_POS_BYTE);
	}
	void Resume()
	{
		if(!this->isPlaying())
			Play(0);
	}
	bool isPlaying() {
		return BASS_ChannelIsActive(this->Snd) != 0;
	}
	void Pause() {
		if(this->isPlaying())
			BASS_ChannelPause(this->Snd);
	}
	static void Play(SOUND Snd, bool PlayFromBeginning)
	{
		BASS_ChannelPlay(Snd,PlayFromBeginning);
	}
	void Play(bool PlayFromBeginning = 1)
	{
		Play(this->Snd,PlayFromBeginning);
	}
	void LoadSnd(const TCHAR * Filename, bool FixAudioLength = 1, bool AutoUnload = 0, bool SoftwareMix = 0, bool Use3D = 0, bool Loop = 0, bool Mono = 0, bool Force8bits = 0)
	{
		this->Snd = BASS_StreamCreateFile(0,Filename,0,0, (FixAudioLength ? BASS_STREAM_PRESCAN : 0) | (AutoUnload ? BASS_STREAM_AUTOFREE : 0) | (SoftwareMix ? BASS_SAMPLE_SOFTWARE : 0) | (Use3D ? BASS_SAMPLE_3D : 0) | (Loop ? BASS_SAMPLE_LOOP : 0) | (Mono ? BASS_SAMPLE_MONO : 0) | (Force8bits ? BASS_SAMPLE_8BITS : 0));
	}
};
#endif
class ConsoleImpl {
public:
	ConsoleImpl() {}

#	ifdef _COMPILE_WITH_AUDIO_
	void Init(const TCHAR * Title = 0, EventCallback * callback = 0, bool deleteCallback = 1, bool InitAudio = 1, unsigned long AudioFrequency = 48000, bool Mono = 0, unsigned int AllocChannels = 64, bool Use3D = 0, bool Force8Bits = 0)
	{
		Sounds = new Sound[AllocChannels];
		SndSz = AllocChannels;
#	else
	void Init(const TCHAR * Title = 0, EventCallback * callback = 0, const bool deleteCallback = 1)
	{
#	endif
		FILE * hf;
		UpdateHandles();
		int hCrt = _open_osfhandle((long)OutputHandle, 0x4000); // _O_TEXT from <fcntl.h> (0x4000)
																// _open_osfhandle from os.h
		hf = _fdopen( hCrt, "w" );
		*stdout = *hf;
		setvbuf( stdout, 0, _IONBF, 0);
		if(Title)
			SetTitle(Title);
		SetEventCallback(callback);
		DelCallback = deleteCallback;
		IsRunning = 1;
		SetAllReasonsWaitTime(0,0);
		AutoNewLine = 0;
#		ifdef _COMPILE_WITH_AUDIO_
		if(InitAudio)
			InitializeAudio(AudioFrequency,Mono,Use3D,Force8Bits);
#		endif
		SendEvent(GenerateInitEvent());
	};
	~ConsoleImpl()
	{
		if(IsRunning)
			Close(0);
		if(DelCallback)
			delete Callback;
#		ifdef _COMPILE_WITH_AUDIO_
		delete[] Sounds;
		Sounds = 0;
		BASS_Free();
#		endif
		Callback = 0;
	};
	void UpdateHandles()
	{
		OutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		InputHandle = GetStdHandle(STD_INPUT_HANDLE);
		DWORD mode;
		GetConsoleMode(InputHandle, &mode);
		mode |= ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS;
		SetConsoleMode(InputHandle, mode);
	}
	EventCallback * GetEventCallback()
	{
		return Callback;
	}
	void SetEventCallback(EventCallback * Function)
	{
		Callback = Function;
	}
	void SendEvent(ConsoleEvent * Event)
	{
		Event->Console = (void *)this;
		OnEvent(Event);
	}
	void OnEvent(ConsoleEvent * Event)
	{
		if(Event && Event->EventType == EVENT_CLOSE)
		{
			ConsoleEvent * kevent = GenerateKeyEvent(0,0);
			if(kevent->Keyb.isKeyDown) delete[] kevent->Keyb.isKeyDown;
			if(kevent->Keyb.wasKeyDown) delete[] kevent->Keyb.wasKeyDown;
			Close(0);
		}
		if(Callback != 0 && Event != 0)
			Callback->OnEvent(Event);
		if(Event != 0)
		{
			if(Event->EventType == EVENT_KEY_INPUT)
			{
				if(Event->Keyb.Key < 0xFF)
				{
					Event->Keyb.wasKeyDown[ (Event->Keyb.Key) ] = Event->Keyb.Down;
				}
			}
			delete Event;
		}
	}
	static void CheckErrors(bool Silent = 0)
	{
		TCHAR * Str = 0;
		unsigned long err = GetLastError();
		if(err == 0 || err == 203 || Silent)
			return;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			0,err,0,(LPTSTR)Str,0,0);
		if(!Str)
			return;
		MessageBox(GetConsoleWindow(),Str,_T("Error"),MB_OK|MB_ICONERROR);
		LocalFree((HLOCAL)Str);
	}

private:
	void Close(const bool _SendEvent)
	{
		if(!IsRunning)
			return;
		IsRunning = 0;
		if(_SendEvent)
		{
			SendEvent(GenerateCloseEvent());
			eWait(REASON_CLOSE);
		}
	}
public:
	void Close()
	{
		if(!IsRunning)
			return;
		IsRunning = 0;
		SendEvent(GenerateCloseEvent());
		eWait(REASON_CLOSE);
	}
	bool Run()
	{
		return IsRunning;
	}
	static void GetHumanizedChar(char * Buffer, unsigned int Size, const unsigned short Key)
	{
		if(!Buffer || !Size || !Key) return;
#		define _S(a,b) case a: strcpy_s(Buffer,Size,b); break;
		switch(Key)
		{
			_S(VK_BACK,"Back Key");
			_S(VK_TAB,"Tab Key");
			_S(VK_RETURN,"Return Key");
			_S(VK_SHIFT,"Shift Key");
			_S(VK_CONTROL,"Control Key");
			_S(VK_MENU,"Alt Key");
			_S(VK_ESCAPE,"Escape Key");
			_S(VK_SPACE,"Space Key");
			_S(VK_END,"End Key");
			_S(VK_HOME,"Home Key");
			_S(VK_LEFT,"Left Key");
			_S(VK_UP,"Up Key");
			_S(VK_RIGHT,"Right Key");
			_S(VK_DOWN,"Down Key");
			_S(VK_SNAPSHOT,"Snapshot Key");
			_S(VK_INSERT,"Ins Key");
			_S(VK_DELETE,"Del Key");
			_S(VK_LWIN,"Left Windows Key");
			_S(VK_RWIN,"Right Windows Key");
			_S(VK_NUMLOCK,"NumLock Key");
			_S(VK_SCROLL,"ScrollLock Key");
			_S(VK_BROWSER_BACK,"\'Back\' Browser Key");
			_S(VK_BROWSER_FORWARD,"\'Forward\' Browser Key");
			_S(VK_BROWSER_REFRESH,"\'Refresh\' Browser Key");
			_S(VK_BROWSER_STOP,"\'Stop\' Browser Key");
			_S(VK_BROWSER_SEARCH,"\'Search\' Browser Key");
			_S(VK_BROWSER_FAVORITES,"\'Favorites\' Browser Key");
			_S(VK_BROWSER_HOME,"\'Home\' Browser Key");
			_S(VK_VOLUME_MUTE,"\'Mute\' Volume Key");
			_S(VK_VOLUME_DOWN,"\'Down\' Volume Key");
			_S(VK_VOLUME_UP,"\'Up\' Volume Key");
			_S(VK_MEDIA_NEXT_TRACK,"\'Next\' Media Key");
			_S(VK_MEDIA_PREV_TRACK,"\'Prev\' Media Key");
			_S(VK_MEDIA_STOP,"\'Stop\' Media Key");
			_S(VK_MEDIA_PLAY_PAUSE,"\'Play/Pause\' Media Key");
			_S(VK_LAUNCH_MAIL,"\'Mail\' Launch Key");
			_S(VK_LAUNCH_MEDIA_SELECT,"\'Media\' Launch Key");
			_S(VK_LAUNCH_APP1,"\'App1\' Launch Key");
			_S(VK_LAUNCH_APP2,"\'App2\' Launch Key");
#			undef _S
		default:
			{
				if(Key >= 'A' && Key <= 'Z')
					sprintf_s(Buffer,Size,"%c Key",(char)Key);
				else if( Key >= VK_F1 && Key <= VK_F24 )
				{
					int fnum = Key - (VK_F1-1);
					sprintf_s(Buffer,Size,"F%d Key",fnum);
				}
				else
					strcpy_s(Buffer,Size,"Unknown Key");
			}
			break;
		}
	}
  private:
	Arr<Button *> Buttons;
  public:
	void RemoveButton(unsigned int Index)
	{
		if(Index < Buttons.size())
		{	
			Buttons[Index]->isEnabled = 0;
			Buttons[Index]->Pos.Bottom = 0;
			Buttons[Index]->Pos.Left = 0;
			Buttons[Index]->Pos.Right = 0;
			Buttons[Index]->Pos.Top = 0;
		}
	}
	void RemoveButton(Button * Btn)
	{
		for(unsigned int i = 0; i < Buttons.size(); i++)
		{
			if(Buttons[i] == Btn)
			{
				Buttons[i]->isEnabled = 0;
				Buttons[i]->Pos.Bottom = 0;
				Buttons[i]->Pos.Left = 0;
				Buttons[i]->Pos.Right = 0;
				Buttons[i]->Pos.Top = 0;
			}
		}
	}
	void ClearButtons()
	{
		Buttons.clear();
	}
	unsigned int AddButton(SMALL_RECT Rect)
	{
		Button * NewButton = new Button;
		memcpy(&(NewButton->Pos),&Rect,sizeof(SMALL_RECT));
		NewButton->isEnabled = 1;
		Buttons.push_back(NewButton);
		return Buttons.size();
	}
	unsigned int AddButton(const short X, const short Y, const short Width = 1, const short Height = 1)
	{
		SMALL_RECT Rct;
		Rct.Left = X;
		Rct.Right = X+Width-2;
		Rct.Top = Y;
		Rct.Bottom = Y+Height;
		return AddButton(Rct);
	}
	unsigned int AddTextButton(short Length, const char * Text, const bool SetCursor = true, const short X = -1, const short Y = -1)
	{
		short nx = X;
		short ny = Y;
		short nowx, nowy;
		GetPosition(nowx, nowy);
		if(X < 0 || Y < 0)
		{
			if(X < 0)
				nx = nowx;
			if(Y < 0)
				ny = nowy;
		}
		SetPosition(nx,ny,0);
		Write(Text,Length);
		if(!SetCursor)
			SetPosition(nowx,nowy);
		return AddButton(nx,ny,Length,0);
	}
	unsigned int AddTextButton(const char * Text, const bool SetCursor = true, const short X = -1, const short Y = -1)
	{
		return AddTextButton((short)strlen(Text)+1,Text,SetCursor,X,Y);
	}
  private:
	bool IsButtonPressed(const Button * Btn, const COORD Position)
	{
		if(!Btn->isEnabled)
			return 0;
		return ((Position.X >= Btn->Pos.Left) && (Position.X <= Btn->Pos.Right) && (Position.Y >= Btn->Pos.Top) && (Position.Y <= Btn->Pos.Bottom));
	}
	bool CheckButtonPressEvents(COORD Position)
	{
		bool hasPressed = 0;
		for(unsigned int i = 0; i < Buttons.size(); i++)
		{
			if(IsButtonPressed(Buttons[i],Position))
			{
				hasPressed = 1;
				SendEvent(GenerateButtonEvent(Buttons[i],Position,i+1));
			}
		}
		return hasPressed;
	}
	int CheckSingleButtonPressEvents(COORD Position)
	{
		for(unsigned int i = 0; i < Buttons.size(); i++)
		{
			if(IsButtonPressed(Buttons[i],Position))
			{
				SendEvent(GenerateButtonEvent(Buttons[i],Position,i+1));
				return (int)i+1;
			}
		}
		return -1;
	}
  public:
	int WaitForButton()
	{
		DWORD r = 0;
		INPUT_RECORD Input;
		while(ReadConsoleInput(InputHandle,&Input,1,&r))
		{
			if(Input.EventType == MOUSE_EVENT)
			{
				ConsoleEvent * Event = GenerateMouseEvent(
					Input.Event.MouseEvent.dwMousePosition.X,
					Input.Event.MouseEvent.dwMousePosition.Y,
					(Input.Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)!=0,
					(Input.Event.MouseEvent.dwButtonState & FROM_LEFT_2ND_BUTTON_PRESSED)!=0,
					(Input.Event.MouseEvent.dwButtonState & FROM_LEFT_3RD_BUTTON_PRESSED)!=0,
					(Input.Event.MouseEvent.dwButtonState & FROM_LEFT_4TH_BUTTON_PRESSED)!=0,
					(Input.Event.MouseEvent.dwButtonState & RIGHTMOST_BUTTON_PRESSED)!=0);

				if( !Event->Mouse.Buttons[0] && Event->Mouse.wasButtons[0])
				{
					int b =	CheckSingleButtonPressEvents(Input.Event.MouseEvent.dwMousePosition);
					if(b != -1)
					{
						delete Event;
						return b;
					}
				}
				delete Event;
			}
		}
		return -1;
	}
	void WaitForKeyPress(const short Char = VK_RETURN)
	{
		DWORD r = 0;
		INPUT_RECORD Input;
		bool isNowPressed = GetAsyncKeyState(Char) != 0;
		while(ReadConsoleInput(InputHandle,&Input,1,&r))
		{
			if(Input.EventType == KEY_EVENT)
			{
				if(Input.Event.KeyEvent.wVirtualKeyCode == Char)
				{
					if(!Input.Event.KeyEvent.bKeyDown && isNowPressed)
					{
						return;
					}
					isNowPressed = Input.Event.KeyEvent.bKeyDown != 0;
				}
			}
		}
	}
	void UpdateEvents()
	{
		if(!IsRunning)
			return;
		DWORD r = 0;
		INPUT_RECORD Input;
		SendEvent(GenerateUpdateEvent());
		eWait(REASON_UPDATE);
		while(PeekConsoleInput(InputHandle,&Input,1,&r), r>0 && IsRunning)
		{
			ReadConsoleInput(InputHandle,&Input,1,&r);
			if(Input.EventType == KEY_EVENT)
			{
				SendEvent(GenerateKeyEvent(Input.Event.KeyEvent.wVirtualKeyCode,Input.Event.KeyEvent.bKeyDown != 0));
			}
			else if(Input.EventType == MOUSE_EVENT)
			{
				ConsoleEvent * Event = GenerateMouseEvent(
					Input.Event.MouseEvent.dwMousePosition.X,
					Input.Event.MouseEvent.dwMousePosition.Y,
					(Input.Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)!=0,
					(Input.Event.MouseEvent.dwButtonState & FROM_LEFT_2ND_BUTTON_PRESSED)!=0,
					(Input.Event.MouseEvent.dwButtonState & FROM_LEFT_3RD_BUTTON_PRESSED)!=0,
					(Input.Event.MouseEvent.dwButtonState & FROM_LEFT_4TH_BUTTON_PRESSED)!=0,
					(Input.Event.MouseEvent.dwButtonState & RIGHTMOST_BUTTON_PRESSED)!=0);

				if( !Event->Mouse.Buttons[0] && Event->Mouse.wasButtons[0])
					CheckButtonPressEvents(Input.Event.MouseEvent.dwMousePosition);

				SendEvent(Event);
			}
		}
	}
	void GetTitle(TCHAR * Buffer, unsigned long Size)
	{
		GetConsoleTitle(Buffer,Size);
	}
	void SetTitle(const TCHAR * Text)
	{
		SetConsoleTitle(Text);
	}
	void Writef(const char * Format, ...)
	{
		va_list list;
		va_start(list,Format);
		char Buffer[1024];
		vsprintf_s(Buffer,1024,Format,list);
		va_end(list);
		Write(Buffer);
	}
	void Write(const char * Text, size_t Length)
	{
		const size_t Size = GetFirstPowerOfTwo(Length)+1;
		char * Buffer = new char[Size];
		strncpy_s(Buffer,Size,Text,Length);
		Buffer[Size-1] = 0;
		if( (Length+1) < (Size-1))
			Buffer[Length+1] = 0;
		DWORD Written = 0;
		WriteConsoleA(OutputHandle,Buffer,Length-1,&Written,0);
		delete[] Buffer;
		eNewLine();
		eWait(REASON_WRITE);
	}
	void Write(const char * Text)
	{
		Write(Text,strlen(Text)+1);
	}
	void Write(const char Char)
	{
		char Text[2];
		Text[0] = Char;
		Text[1] = 0;
		Write(Text);
	}
	void NewLine()
	{
		DWORD Written = 0;
		WriteConsoleA(OutputHandle,"\n\0",1,&Written,0);
	}
	char GetPositionChar(const short * X = 0, const short * Y = 0)
	{
		TCHAR Buffer[2];
		Buffer[0] = 0;
		Buffer[1] = 1;
		GetPositionText(1,Buffer,2,X,Y);
		Buffer[1] = 0;
		return Buffer[0];
	}
	char GetPositionChar(const short X = -1, const short Y = -1)
	{
		TCHAR Buffer[2];
		Buffer[0] = 0;
		Buffer[1] = 1;
		GetPositionText(1,Buffer,2,X,Y);
		Buffer[1] = 0;
		return Buffer[0];
	}
	void GetWriteColor(Colors &Front, Colors &Back)
	{
		CONSOLE_SCREEN_BUFFER_INFO bufinf;
		ZeroMemory(&bufinf,sizeof(CONSOLE_SCREEN_BUFFER_INFO));
		Front = (Colors) (bufinf.wAttributes % 16);
		Back = (Colors) ((bufinf.wAttributes/16) % 16);
	}
	void SetWriteColor(Colors Front = COLOR_LAST, Colors Back = COLOR_LAST)
	{
		if(Front == COLOR_LAST || Back == COLOR_LAST)
		{
			Colors lf, lb;
			GetWriteColor(lf,lb);
			if(Front == COLOR_LAST)
				Front = lf;
			else
				Back = lb;
		}
		Front =(Colors) (((int)Front)%16);
		Back  =(Colors) (((int)Back )%16);
		unsigned short Clr = ((unsigned short)Back<<4)|(unsigned short)Front;
		SetConsoleTextAttribute(OutputHandle,Clr);
	}
	void GetPositionText(unsigned int Length, TCHAR * Buffer, unsigned int Size, const short * X = 0, const short * Y = 0)
	{
		if(!Buffer || !Size || !Length) return;
		unsigned int Len = __min(Length,Size);
		COORD ReadCoord;
		if(X) ReadCoord.X = *X;
		if(Y) ReadCoord.Y = *Y;
		DWORD r = 0;
		if(X && Y)
		{
			NormalizeCoord(ReadCoord);
			ReadConsoleOutputCharacter(OutputHandle,Buffer,Len,ReadCoord,&r);
		}
		else
		{
			short nowx, nowy;
			GetPosition(nowx, nowy);
			if(!X) ReadCoord.X = nowx;
			if(!Y) ReadCoord.Y = nowy;
			NormalizeCoord(ReadCoord);
			ReadConsoleOutputCharacter(OutputHandle,Buffer,Len,ReadCoord,&r);
		}
	}
	void SetAutoNewLine(const bool State)
	{
		AutoNewLine = State;
	}
	bool GetAutoNewLine()
	{
		return AutoNewLine;
	}
	void GetPositionText(unsigned int Length, TCHAR * Buffer, unsigned int Size, const short X = -1, const short Y = -1)
	{
		if(X != -1 && Y != -1)
			GetPositionText(Length,Buffer,Size,&X,&Y);
		else
		{
			short x, y;
			GetPosition(x,y);
			if(X != -1) x = X;
			if(Y != -1) y = Y;
			GetPositionText(Length,Buffer,Size,&x,&y);
		}
	}
	void pSetPositionText(unsigned int Length, const char * Buffer, const short * X = 0, const short * Y = 0, const bool SetCursor = 0)
	{
		if(!Length || !Buffer) return;
		COORD WriteCoord;
		if(X) WriteCoord.X = *X;
		if(Y) WriteCoord.Y = *Y;
		//DWORD r = 0;
		short lastx, lasty;
		GetPosition(lastx,lasty);
		if(X && Y)
		{
			NormalizeCoord(WriteCoord);
			SetPosition(*X,*Y);
			Write(Buffer,Length);
		}
		else
		{
			if(!X) WriteCoord.X = lastx;
			if(!Y) WriteCoord.Y = lasty;
			NormalizeCoord(WriteCoord);
			SetPosition(*X,*Y);
			Write(Buffer,Length);
		}
		if(!SetCursor)
			SetPosition(lastx,lasty);
	}
	void SetPositionText(unsigned int Length, const char * Buffer, const short X = -1, const short Y = -1, const bool SetCursor = 0)
	{
		if(X != -1 && Y != -1)
			pSetPositionText(Length,Buffer,&X,&Y,SetCursor);
		else
		{
			short x, y;
			GetPosition(x,y);
			if(X != -1) x = X;
			if(Y != -1) y = Y;
			pSetPositionText(Length,Buffer,&x,&y,SetCursor);
		}
	}
	void pSetPositionChar(const char Char, const short * X = 0, const short * Y = 0, const bool SetCursor = 0)
	{
		char Buffer[2];
		Buffer[0] = Char;
		Buffer[1] = 0;
		pSetPositionText(1,Buffer,X,Y,SetCursor);
	}
	void SetPositionChar(const char Char, const short X = -1, const short Y = -1, const bool SetCursor = 0)
	{
		pSetPositionChar(Char,&X,&Y,SetCursor);
	}
	void pSetPositionText(const char * Buffer, const short * X = 0, const short * Y = 0, const bool SetCursor = 0)
	{
		pSetPositionText(strlen(Buffer)+1,Buffer,X,Y,SetCursor);
	}
	void SetPositionText(const char * Buffer, const short X = -1, const short Y = -1, const bool SetCursor = 0)
	{
		SetPositionText(strlen(Buffer)+1,Buffer,X,Y,SetCursor);
	}
	void pSetOutlinedText(unsigned int Length, const TCHAR * Text, const short * X = 0, const short * Y = 0)
	{
		if(!Length || !Text) return;
		DWORD r = 0;
		COORD WriteCoord;
		if(X) WriteCoord.X = *X;
		if(Y) WriteCoord.Y = *Y;
		if(!Y || !X)
		{
			short nowx, nowy;
			GetPosition(nowx, nowy);
			if(!X) WriteCoord.X = nowx;
			if(!Y) WriteCoord.Y = nowy;
		}
		NormalizeCoord(WriteCoord);
		unsigned char Chars[] = {
			(unsigned char)0xDA, // ┌
			(unsigned char)0xC4, // ─
			(unsigned char)0xBF, // ┐
			(unsigned char)0xB3, // │
			(unsigned char)0xC0, // └
			(unsigned char)0xD9  // ┘
		};
		COORD scc;
		scc.X = WriteCoord.X-1;
		scc.Y = WriteCoord.Y-1;
		if(scc.X >= 0 && scc.Y >= 0)
			FillConsoleOutputCharacter(OutputHandle,Chars[0],1,scc,&r);
		scc.Y ++;
		if(scc.X >= 0 && scc.Y >= 0)
			FillConsoleOutputCharacter(OutputHandle,Chars[3],1,scc,&r);
		scc.Y ++;
		if(scc.X >= 0 && scc.Y >= 0)
			FillConsoleOutputCharacter(OutputHandle,Chars[4],1,scc,&r);
		scc.X = WriteCoord.X;
		scc.Y = WriteCoord.Y-1;
		if(scc.X >= 0 && scc.Y >= 0)
			FillConsoleOutputCharacter(OutputHandle,Chars[1],Length,scc,&r);
		scc.Y += 2;
		if(scc.X >= 0 && scc.Y >= 0)
			FillConsoleOutputCharacter(OutputHandle,Chars[1],Length,scc,&r);
		scc.X = WriteCoord.X + Length;
		scc.Y -= 2;
		if(scc.X >= 0 && scc.Y >= 0)
			FillConsoleOutputCharacter(OutputHandle,Chars[2],1,scc,&r);
		scc.Y ++;
		if(scc.X >= 0 && scc.Y >= 0)
			FillConsoleOutputCharacter(OutputHandle,Chars[3],1,scc,&r);
		scc.Y ++;
		if(scc.X >= 0 && scc.Y >= 0)
			FillConsoleOutputCharacter(OutputHandle,Chars[5],1,scc,&r);
		NormalizeCoord(WriteCoord);
		WriteConsoleOutputCharacter(OutputHandle,Text,Length,WriteCoord,&r);
	}
	void SetOutlinedText(unsigned int Length, const TCHAR * Text, const short X = -1, const short Y = -1)
	{
		if(X != -1 && Y != -1)
			pSetOutlinedText(Length,Text,&X,&Y);
		else
		{
			short x, y;
			GetPosition(x,y);
			if(X != -1) x = X;
			if(Y != -1) y = Y;
			pSetOutlinedText(Length,Text,&x,&y);
		}
	}
	void pSetOutlinedText(const TCHAR * Text, const short * X = 0, const short * Y = 0)
	{
		pSetOutlinedText(strlen((char*)Text),Text,X,Y);
	}
	void SetOutlinedText(const TCHAR * Text, const short X = -1, const short Y = -1)
	{
		SetOutlinedText(strlen((char*)Text),Text,X,Y);
	}
	void SetPosition(const short X, const short Y, const bool CleanLines = 0)
	{
		COORD Pos;
		Pos.X = X;
		Pos.Y = Y;
		if(CleanLines)
		{
			CONSOLE_SCREEN_BUFFER_INFO bufinfo;
			GetConsoleScreenBufferInfo(OutputHandle,&bufinfo);
			short Y_Diff = bufinfo.dwCursorPosition.Y - Y;
			DWORD d = 0;
			COORD Line;
			Line.X = 0;
			while(Y_Diff > 0)
			{
				Line.Y = Y_Diff;
				FillConsoleOutputCharacter(OutputHandle,' ',bufinfo.dwMaximumWindowSize.X,Line,&d);
				Y_Diff--;
			}
			FillConsoleOutputCharacter(OutputHandle,' ',bufinfo.dwMaximumWindowSize.X - Pos.X, Pos, &d);
		}
		NormalizeCoord(Pos);
		SetConsoleCursorPosition(OutputHandle,Pos);
		eWait(REASON_SET_CURSOR);
	}
	void GetPosition(short &X, short &Y)
	{
		CONSOLE_SCREEN_BUFFER_INFO bufinfo;
		GetConsoleScreenBufferInfo(OutputHandle,&bufinfo);
		X = bufinfo.dwCursorPosition.X;
		Y = bufinfo.dwCursorPosition.Y;
	}
	void GetPosition(short * X = 0, short * Y = 0)
	{
		if(!X && !Y) return;
		CONSOLE_SCREEN_BUFFER_INFO bufinfo;
		GetConsoleScreenBufferInfo(OutputHandle,&bufinfo);
		if(X) *X = bufinfo.dwCursorPosition.X;
		if(Y) *Y = bufinfo.dwCursorPosition.Y;
	}
	void GetRelativePosition(short * X = 0, short * Y = 0, const short RelativeX = 0, const short RelativeY = 0)
	{
		if(!X && !Y) return;
		GetPosition(X,Y);
		if(X) *X += RelativeX;
		if(Y) *Y += RelativeY;
	}
	void GetRelativePosition(short &X, short &Y, const short RelativeX = 0, const short RelativeY = 0)
	{
		GetPosition(X,Y);
		X += RelativeX;
		Y += RelativeY;
	}
	void SetRelativePosition(const short X = 0, const short Y = 0, const bool CleanLines = 0)
	{
		short FinalX, FinalY;
		GetRelativePosition(FinalX,FinalY,X,Y);
		SetPosition(FinalX,FinalY,CleanLines);
	}
	void GetConsoleShownSize(short &X, short &Y)
	{
		CONSOLE_SCREEN_BUFFER_INFO coninfo;
		GetConsoleScreenBufferInfo(OutputHandle,&coninfo);
		X = coninfo.srWindow.Right - coninfo.srWindow.Left;
		Y = coninfo.srWindow.Bottom - coninfo.srWindow.Top;
		X++;
		Y++;
	}
	void GetConsoleShownSize(short * X, short * Y)
	{
		if(!X && !Y) return;
		short x, y;
		GetConsoleShownSize(x,y);
		if(X) *X = x;
		if(Y) *Y = y;
	}
	void GetConsoleShownOffset(short &X, short &Y)
	{
		CONSOLE_SCREEN_BUFFER_INFO coninfo;
		GetConsoleScreenBufferInfo(OutputHandle,&coninfo);
		X = coninfo.srWindow.Left;
		Y = coninfo.srWindow.Top;
	}
	void GetConsoleShownOffset(short * X, short * Y)
	{
		if(!X && !Y) return;
		short x, y;
		GetConsoleShownSize(x,y);
		if(X) *X = x;
		if(Y) *Y = y;
	}
	void GetConsoleFullSize(short &X, short &Y)
	{
		CONSOLE_SCREEN_BUFFER_INFO coninfo;
		GetConsoleScreenBufferInfo(OutputHandle,&coninfo);
		X = coninfo.dwSize.X;
		Y = coninfo.dwSize.Y;
	}
	void GetConsoleFullSize(short * X = 0, short * Y = 0)
	{
		if(!X && !Y) return;
		short x,y;
		GetConsoleFullSize(x,y);
		if(X) *X = x;
		if(Y) *Y = y;
	}
	void SetConsoleFullSize(short X, short Y)
	{
		short xold,yold;
		GetConsoleFullSize(xold,yold);
		if(xold == X && yold == Y)
			return;
		COORD Size;
		Size.X = X;
		Size.Y = Y;
		short szx, szy;
		GetLargestSize(szx,szy);
		int Valid = SetConsoleScreenBufferSize(OutputHandle,Size);
		while (Valid == 0)
		{
			Size.X++;
			if(Size.X > szx)
				break;
			Valid = SetConsoleScreenBufferSize(OutputHandle,Size);
		}
		if(Valid != 0)
			return;
		Size.X = X;
		while(Valid == 0)
		{
			Size.Y++;
			if(Size.Y > szy)
				break;
			Valid = SetConsoleScreenBufferSize(OutputHandle,Size);
		}
		if(Valid != 0)
			return;
		Size.Y = Y;
		while((Size.Y < szy) && (Valid == 0))
		{
			while((Size.X < szx) && (Valid == 0))
			{
				Valid = SetConsoleScreenBufferSize(OutputHandle,Size);
				Size.X++;
			}
			Size.Y++;
		}
		if(Valid == 0)
		{
			Size.X = szx;
			Size.Y = szy;
			Valid = SetConsoleScreenBufferSize(OutputHandle,Size);
			if(Valid == 0)
			{
				Size.X--;
				Size.Y--;
				Valid = SetConsoleScreenBufferSize(OutputHandle,Size);
			}
		}
		return;
	}
	void GetLargestSize(short &X, short &Y)
	{
		COORD Sz = GetLargestConsoleWindowSize(OutputHandle);
		X = Sz.X;
		Y = Sz.Y;
	}
	void GetLargestSize(short * X, short * Y)
	{
		short x,y;
		GetLargestSize(x,y);
		if(X) *X = x;
		if(Y) *Y = y;
	}
	void FillLine(Colors Front, Colors Back = COLOR_BLACK, const short Y = -1)
	{
		short szx, szy;
		GetConsoleFullSize(szx,szy);
		const short xbs = 0, ybs = 0;
		COORD Nl;
		Nl.X = xbs;
		if(Y > -1)
			Nl.Y = Y;
		else
			GetPosition(0,&(Nl.Y));
		DWORD R = 0;
		if(Front == COLOR_LAST || Back == COLOR_LAST)
		{
			Colors lf, lb;
			GetWriteColor(lf,lb);
			if(Front == COLOR_LAST)
				Front = lf;
			else
				Back = lb;
		}
		Front =(Colors) (((int)Front)%16);
		Back  =(Colors) (((int)Back )%16);
		unsigned short Clr = ((unsigned short)Back<<4)|(unsigned short)Front;
		FillConsoleOutputAttribute(OutputHandle,Clr,szx,Nl,&R);
	}
	void FillLine(const char Char = ' ', const short Y = -1)
	{
		short szx, szy;
		GetConsoleFullSize(szx,szy);
		const short xbs=0, ybs=0;
		COORD Nl;
		Nl.X = xbs;
		if(Y > -1)
			Nl.Y = Y;
		else
			GetPosition(0,&(Nl.Y));
		DWORD R = 0;
		FillConsoleOutputCharacter(OutputHandle,Char,szx,Nl,&R);
	}
	void FillConsole(Colors Front, Colors Back = COLOR_BLACK)
	{
		short szx, szy;
		GetConsoleFullSize(szx,szy);
		const short xbs = 0, ybs = 0;
		COORD Nl;
		Nl.X = xbs;
		Nl.Y = 0;
		DWORD R = 0;
		if(Front == COLOR_LAST || Back == COLOR_LAST)
		{
			Colors lf, lb;
			GetWriteColor(lf,lb);
			if(Front == COLOR_LAST)
				Front = lf;
			else
				Back = lb;
		}
		Front =(Colors) (((int)Front)%16);
		Back  =(Colors) (((int)Back )%16);
		unsigned short Clr = ((unsigned short)Back<<4)|(unsigned short)Front;
		for(short i = ybs; i < szy; i++)
		{
			FillConsoleOutputAttribute(OutputHandle,Clr,szx,Nl,&R);
			Nl.Y++;
		}
	}
	void FillConsole(const char Char = ' ')
	{
		short szx, szy;
		GetConsoleFullSize(szx,szy);
		const short xbs=0, ybs=0;
		COORD Nl;
		Nl.X = xbs;
		Nl.Y = 0;
		DWORD R = 0;
		for(short i = ybs; i < szy; i++)
		{
			FillConsoleOutputCharacter(OutputHandle,Char,szx,Nl,&R);
			Nl.Y++;
		}
		eWait(REASON_FILL_CONSOLE);
		SetPosition(0,0);
	}
	void FillRect(const SMALL_RECT &Rect, const char Char = ' ')
	{
		COORD Nl;
		Nl.X = Rect.Left;
		Nl.Y = Rect.Top;
		size_t len = Rect.Right - Rect.Left;
		size_t hei = Rect.Bottom - Rect.Top;
		DWORD r = 0;
		for(size_t i = 0; i < hei; i++)
		{
			FillConsoleOutputCharacter(OutputHandle,Char,len,Nl,&r);
			Nl.Y++;
		}
	}
	HANDLE GetOutputHandle()
	{
		return OutputHandle;
	}
	HANDLE GetInputHandle()
	{
		return InputHandle;
	}
	HWND GetWindow()
	{
		return GetConsoleWindow();
	}
	enum EventType {
		EVENT_INIT = 0,
		EVENT_FRAMEUPDATE,
		EVENT_MOUSE_INPUT,
		EVENT_KEY_INPUT,
		EVENT_BUTTONPRESS,
		EVENT_CLOSE
	};
	enum Reason {
		REASON_WRITE = 0,
		REASON_CLOSE,
		REASON_UPDATE,
		REASON_SET_CURSOR,
		REASON_FILL_CONSOLE,
		REASON_BLEEP,
		REASON_BLEEP_SEQ,
		REASONS
	};
	struct BleepInfo {
		unsigned long Time;
		unsigned long Frequency;
		unsigned long PauseAfterBleep;
	};
	void SetReasonWaitTime(const Reason reason, const unsigned long Time = 0, const bool Wait = 1)
	{
		if(!(reason >= 0 && reason < REASONS)) return;
		WaitingTime[reason] = Time;
		IsWaiting[reason] = Wait;
	}
	void SetAllReasonsWaitTime(const unsigned long Time = 0, const bool Wait = 0)
	{
		for(unsigned int i = 0; i < (unsigned int)REASONS; i++)
		{
			WaitingTime[i] = Time;
			IsWaiting[i] = Wait;
		}
	}
	bool IsCursorVisible()
	{
		CONSOLE_CURSOR_INFO cinfo;
		ZeroMemory(&cinfo,sizeof(CONSOLE_CURSOR_INFO));
		GetConsoleCursorInfo(OutputHandle,&cinfo);
		return (cinfo.bVisible == 1);
	}
	void SetCursorVisible(bool State)
	{
		CONSOLE_CURSOR_INFO cinfo;
		ZeroMemory(&cinfo,sizeof(CONSOLE_CURSOR_INFO));
		GetConsoleCursorInfo(OutputHandle,&cinfo);
		cinfo.bVisible = State ? 1 : 0; // WinAPI's BOOL is a integer. Beware.
		SetConsoleCursorInfo(OutputHandle,&cinfo);
	}
	size_t GetCursorWidth()
	{
		CONSOLE_CURSOR_INFO cinfo;
		ZeroMemory(&cinfo,sizeof(CONSOLE_CURSOR_INFO));
		GetConsoleCursorInfo(OutputHandle,&cinfo);
		return cinfo.dwSize;
	}
	void SetCursorWidth(size_t Width)
	{
		CONSOLE_CURSOR_INFO cinfo;
		ZeroMemory(&cinfo,sizeof(CONSOLE_CURSOR_INFO));
		GetConsoleCursorInfo(OutputHandle,&cinfo);
		cinfo.dwSize = Width;
		SetConsoleCursorInfo(OutputHandle,&cinfo);
	}
	bool IsConsoleFullscreen()
	{
		DWORD Value = 0;
		GetConsoleDisplayMode(&Value);
		return (Value>0);
	}
	void SetConsoleFullscreen(bool State)
	{
		DWORD Value = State ? 1 : 2; // Fullscreen = 1, Windowed = 2
		short SizeX = 0, SizeY = 0;
		this->GetConsoleFullSize(SizeX,SizeY);
		COORD ScreenBufferSize;
		ScreenBufferSize.X = SizeX;
		ScreenBufferSize.Y = SizeY;
		SetConsoleDisplayMode(OutputHandle,Value,&ScreenBufferSize);
	}
	size_t Scan(char * Dst, size_t Size)
	{
		DWORD ReadSize = 0;
		ReadConsoleA(InputHandle,Dst,Size,&ReadSize,0);
		for(unsigned int i = 0; i < Size; i++)
		{
			if(Dst[i] <= 0 || Dst[i] == '\r' || Dst[i] == '\n')
			{
				Dst[i] = 0;
				break;
			}
		}
		return strlen(Dst);
	}
	bool Scanf(long &Dst)
	{
		//DWORD ReadSize = 0;
		char Text[1024];
		Scan(Text,1024);
		char * Pointer = (char *) (INT_MAX-1);
		long Value = strtol(Text,&Pointer,0);
		if(Pointer != (char *)(INT_MAX-1))
		{
			Dst = Value;
			return 1;
		}
		return 0;
	}
	bool Scanf(int &Dst)
	{
		long Value = 0;
		bool Ret = Scanf(Value);
		if(Ret)
			Dst = (int)Value;
		return Ret;
	}
	bool Scanf(double &Dst)
	{
		char Text[1024];
		Scan(Text,1024);
		char * Pointer = (char *)(INT_MAX-1);
		double Value = strtod(Text,&Pointer);
		if(Pointer != (char *)(INT_MAX-1))
		{
			Dst = Value;
			return 1;
		}
		return 0;
	}
	bool Scanf(float &Dst)
	{
		double Value = 0.0;
		bool Ret = Scanf(Value);
		if(Ret)
			Dst = (float)Value;
		return Ret;
	}
  private:
	  // Nothing here.
  public:
	static void ShellExec(const TCHAR * FileOrURL)
	{
		ShellExecute(0,_T("open"),FileOrURL,0,0,SW_SHOWNORMAL);
	}
	static void Wait(const unsigned long Ticks)
	{
		if(!Ticks) return;
		Sleep(Ticks);
		return;
	}
	static void WaitForVSync(unsigned short FastenRate = 1)
	{
		Wait(GetVSyncWaitTime(FastenRate));
	}
	static unsigned long GetVSyncWaitTime(unsigned short FastenRate = 1)
	{
		if(FastenRate == 0) FastenRate = 1;
		return (unsigned long) (1.f / (60.f * ((float)FastenRate) ) * 1000.f);
	}
	static ConsoleEvent * GenerateUpdateEvent()
	{
		ConsoleEvent * Event = GenerateDefaultEvent();
		Event->EventType = EVENT_FRAMEUPDATE;
		return Event;
	}
	static ConsoleEvent * GenerateCloseEvent()
	{
		ConsoleEvent * Event = GenerateDefaultEvent();
		Event->EventType = EVENT_CLOSE;
		return Event;
	}
	static ConsoleEvent * GenerateButtonEvent(Button * Btn, COORD Position, unsigned int Id)
	{
		ConsoleEvent * Event = GenerateDefaultEvent();
		Event->EventType = EVENT_BUTTONPRESS;
		Event->Btn.Btn = Btn;
		Event->Btn.PosX = Position.X;
		Event->Btn.PosY = Position.Y;
		Event->Btn.Id = Id;
		return Event;
	}
	static ConsoleEvent * GenerateInitEvent()
	{
		ConsoleEvent * Event = GenerateDefaultEvent();
		Event->EventType = EVENT_INIT;
		return Event;
	}
	static ConsoleEvent * GenerateMouseEvent(const short X, const short Y, const bool Down1, const bool Down2, const bool Down3, const bool Down4, const bool Down5)
	{
		ConsoleEvent * Event = GenerateDefaultEvent();
		Event->EventType = EVENT_MOUSE_INPUT;
		Event->Mouse.PosX = X;
		Event->Mouse.PosY = Y;
		static bool LastPressed[5] = {0,0,0,0,0};
		Event->Mouse.wasButtons[0] = LastPressed[0];
		Event->Mouse.wasButtons[1] = LastPressed[1];
		Event->Mouse.wasButtons[2] = LastPressed[2];
		Event->Mouse.wasButtons[3] = LastPressed[3];
		Event->Mouse.wasButtons[4] = LastPressed[4];
		LastPressed[0] = Down1;
		LastPressed[1] = Down2;
		LastPressed[2] = Down3;
		LastPressed[3] = Down4;
		LastPressed[4] = Down5;
		Event->Mouse.Buttons[0] = Down1;
		Event->Mouse.Buttons[1] = Down2;
		Event->Mouse.Buttons[2] = Down3;
		Event->Mouse.Buttons[3] = Down4;
		Event->Mouse.Buttons[4] = Down5;
		return Event;
	}
	static void GetMouseEventData(const ConsoleEvent * Event, short &X, short &Y, bool &Down1, bool &Down2, bool &Down3, bool &Down4, bool &Down5)
	{
		if(!Event) return;
		if(Event->EventType != EVENT_MOUSE_INPUT) return;
		X = Event->Mouse.PosX;
		Y = Event->Mouse.PosY;
		Down1 = Event->Mouse.Buttons[0];
		Down2 = Event->Mouse.Buttons[1];
		Down3 = Event->Mouse.Buttons[2];
		Down4 = Event->Mouse.Buttons[3];
		Down5 = Event->Mouse.Buttons[4];
	}
	static void GetMouseEventData(const ConsoleEvent * Event, short * X = 0, short * Y = 0, bool * Down1 = 0, bool * Down2 = 0, bool * Down3 = 0, bool * Down4 = 0, bool * Down5 = 0)
	{
		if(!Event) return;
		if(!(X || Y || Down1 || Down2 || Down3 || Down4 || Down5)) return;
		if(Event->EventType != EVENT_MOUSE_INPUT) return;
		if(X) *X = Event->Mouse.PosX;
		if(Y) *Y = Event->Mouse.PosY;
		if(Down1) *Down1 = Event->Mouse.Buttons[0];
		if(Down2) *Down2 = Event->Mouse.Buttons[1];
		if(Down3) *Down3 = Event->Mouse.Buttons[2];
		if(Down4) *Down4 = Event->Mouse.Buttons[3];
		if(Down5) *Down5 = Event->Mouse.Buttons[4];
	}
	static ConsoleEvent * GenerateDefaultEvent()
	{
		ConsoleEvent * Event = new ConsoleEvent;
		memset(Event,0,sizeof(ConsoleEvent));
		Event->Keyb.isKeyDown = ConsoleImpl::isKeyDownArray;
		Event->Keyb.wasKeyDown = ConsoleImpl::wasKeyDownArray;
		return Event;
	}
	static ConsoleEvent * GenerateKeyEvent(const unsigned short Key, const bool IsDown)
	{
		ConsoleEvent * Event = GenerateDefaultEvent();
		Event->EventType = EVENT_KEY_INPUT;
		Event->Keyb.Key = Key;
		Event->Keyb.Down = IsDown;
		if(Key < 0xFF)
		{
			isKeyDownArray[ Key ] = IsDown;
		}
		return Event;
	}
	static void GetKeyEventData(const ConsoleEvent * Event, unsigned short &Key, bool &IsDown)
	{
		if(!Event) return;
		if(Event->EventType != EVENT_KEY_INPUT) return;
		Key = (unsigned short)Event->Keyb.Key;
		IsDown = Event->Keyb.Down;
	}
	static COORD NormalizeCoord(COORD &Coord)
	{
		if(Coord.X < 0) Coord.X = 0;
		if(Coord.Y < 0) Coord.Y = 0;
		return Coord;
	}
	static SMALL_RECT sr(const short Top = 0, const short Left = 0, const short Bottom = 0, const short Right = 0)
	{
		SMALL_RECT r;
		r.Top = Top;
		r.Bottom = Bottom;
		r.Left = Left;
		r.Right = Right;
		return r;
	}
	static void GetKeyEventData(const ConsoleEvent * Event, unsigned short * Key = 0, bool * IsDown = 0)
	{
		if(!Event) return;
		if(Event->EventType != EVENT_KEY_INPUT) return;
		if(Key) *Key = (unsigned short)Event->Keyb.Key;
		if(IsDown) *IsDown = Event->Keyb.Down;
	}
	static DWORD Thread(LPTHREAD_START_ROUTINE Function, void * Params)
	{
		DWORD ThreadID = 0;
		CreateThread(0,0,Function,Params,0,&ThreadID);
		return ThreadID;
	}
	static void Bleep(BleepInfo * Info, size_t Size = 1)
	{
		if(!Info) return;
		for(size_t i = 0; i < Size; i++)
		{
			Bleep(Info[i].Time,Info[i].Frequency);
			if(Info[i].PauseAfterBleep) Wait(Info[i].PauseAfterBleep);
		}
		eWait(ConsoleImpl::REASON_BLEEP_SEQ);
	}
	static void Bleep(unsigned long Time, unsigned long Frequency)
	{
		Beep(Frequency, Time);
		eWait(ConsoleImpl::REASON_BLEEP);
	}
	static void AsyncBleep(unsigned long Time, unsigned long Frequency)
	{
		unsigned long * dat = new unsigned long[2];
		dat[0] = Time;
		dat[1] = Frequency;
		ConsoleImpl::Thread((LPTHREAD_START_ROUTINE)Bleep_Thread,dat);
	}
#	ifdef _COMPILE_WITH_AUDIO_
	Sound * Snd(unsigned int Index)
	{
		if(Index >= SndSz)
			return 0;
		return (&(Sounds[Index]));
	}
	Sound * Sounds;
	bool isValidSndIndex(unsigned int Index)
	{
		return (Index < SndSz);
	}
	unsigned int GetFreeSndIndex()
	{
		for(unsigned int i = 0; i < SndSz; i++)
		{
			if(Sounds[i].GetStream() != 0)
			{
				return i;
			}
		}
		return SndSz+1;
	}
	unsigned int SndSz;
#	endif
	static bool * GetIsKeyDownArray()
	{
		return ConsoleImpl::isKeyDownArray;
	}
	static bool * GetWasKeyDownArray()
	{
		return ConsoleImpl::wasKeyDownArray;
	}
	bool IsRunning;

private:
	static bool * isKeyDownArray;
	static bool * wasKeyDownArray;
	static void Bleep_Thread_Sq(void * Data)
	{
		char ** dat = (char **)Data;
		BleepInfo * Copy = (BleepInfo *)dat[0];
		size_t * sz = (size_t*)dat[1];
		delete[] dat;
		Bleep(Copy,*sz);
		delete[] Copy;
		delete sz;
	}
	static void Bleep_Thread(void * Data)
	{
		size_t * dat = (size_t *)Data;
		Bleep(dat[0],dat[1]);
		delete[] dat;
	}
	EventCallback * Callback;
	static bool IsWaiting[REASONS];
	bool AutoNewLine;
	bool DelCallback;
	static unsigned long WaitingTime[REASONS];
	HANDLE OutputHandle;
	HANDLE InputHandle;
	void eNewLine()
	{
		if(AutoNewLine)
			NewLine();
	}
	static void eWait(const Reason reason)
	{
		if((reason >= REASONS) || (reason < 0) || (WaitingTime[reason] == 0) || (!IsWaiting[reason])) return;
		Wait(WaitingTime[reason]);
	}
public:
	static unsigned long GetFirstPowerOfTwo(const unsigned long Number)
	{
		unsigned long Value = 1;
		while(Value < Number)
		{
			Value *= 2;
		}
		return Value;
	}
	static int InitError(const TCHAR * Component, unsigned int Line = 0)
	{
		static char Text[512];
		if(Line)
			sprintf_s(Text,512,"Initialization Error at line %u:\nComponent: %s\0",Line,Component);
		else
			sprintf_s(Text,512,"Initialization Error:\nComponent: %s\0",Component);
		MessageBoxA(NULL,Text,"Initialization Error",MB_ICONERROR);
		return 0;
	}
	static int SimpleError(const TCHAR * Description, unsigned int Line = 0)
	{
		static char Text[512];
		if(Line)
			sprintf_s(Text,512,"Error at line %u:\n%s\0",Line,Description);
		else
			sprintf_s(Text,512,"Error:\n%s\0",Description);
		MessageBoxA(0,Text,"Non-Fatal Error",MB_OK|MB_ICONWARNING);
		return 0;
	}
	static int FatalError(const TCHAR * Description, unsigned int Line = 0, const bool Quit = 0)
	{
		static char Text[512];
		if(Line)
			sprintf_s(Text,512,"Fatal Error at line %u:\n%s%s\0",Line,Description, (Quit?"\nProgram will now quit.":" ") );
		else
			sprintf_s(Text,512,"Fatal Error:\n%s%s\0",Description, (Quit?"\nProgram will now quit.":" ") );
		MessageBoxA(0,Text,"Fatal Error",MB_OK|MB_ICONERROR);
		return 0;
	}
#	ifdef _COMPILE_WITH_AUDIO_
	static int BassError(const TCHAR * Description = 0, unsigned int Line = 0, const bool ReturnErrorCode = 0 )
	{
		static char Text[512];
		int errcode = BASS_ErrorGetCode();
		const TCHAR * error = GetBassError(errcode);
		if(!Description && !Line)
			sprintf_s(Text,512,"Bass Error:\n%s\0",error);
		else if(!Description)
			sprintf_s(Text,512,"Bass Error at line %u:\n%s\0",Line,error);
		else if(!Line)
			sprintf_s(Text,512,"Bass Error:\n%s\n\nInfo: %s\0",error,Description);
		else
			sprintf_s(Text,512,"Bass Error at line %u:\n%s\n\nInfo: %s\0",Line,error,Description);
		MessageBoxA(NULL,Text,"Bass Audio Engine Error",MB_OK|MB_ICONERROR);
		return (ReturnErrorCode ? errcode : 0);
	}
	static const TCHAR * GetBassError(int error)
	{
		switch(error)
		{
		case BASS_OK:
			return "No Error Found.";
		case BASS_ERROR_MEM:
			return "Memory Error.";
		case BASS_ERROR_FILEOPEN:
			return "Cannot Open The File.";
		case BASS_ERROR_DRIVER:
			return "Cannot Find a Free/Valid Driver.";
		case BASS_ERROR_BUFLOST:
			return "A Sample Buffer was Lost.";
		case BASS_ERROR_HANDLE:
			return "Invalid Handle.";
		case BASS_ERROR_FORMAT:
			return "Unsupported Sample Format.";
		case BASS_ERROR_POSITION:
			return "Invalid Position.";
		case BASS_ERROR_INIT:
			return "BASS_Init has not been Successfully called.";
		case BASS_ERROR_START:
			return "BASS_Start has not been Successfully called.";
		case BASS_ERROR_ALREADY:
			return "Already Initialized/Paused/Whatever.";
		case BASS_ERROR_NOCHAN:
			return "Cannot get a free channel.";
		case BASS_ERROR_ILLTYPE:
			return "Illegal Type has been Specified.";
		case BASS_ERROR_ILLPARAM:
			return "Illegal Parameter has been Specified.";
		case BASS_ERROR_NO3D:
			return "No 3D Support.";
		case BASS_ERROR_NOEAX:
			return "No EAX Support.";
		case BASS_ERROR_DEVICE:
			return "Illegal Device Number.";
		case BASS_ERROR_NOPLAY:
			return "Not Playing.";
		case BASS_ERROR_FREQ:
			return "Illegal Sample Rate.";
		case BASS_ERROR_NOTFILE:
			return "The stream is not a file stream.";
		case BASS_ERROR_NOHW:
			return "No Hardware Voices Available.";
		case BASS_ERROR_EMPTY:
			return "The MOD File has no sequence data.";
		case BASS_ERROR_NONET:
			return "No internet connection could be opened.";
		case BASS_ERROR_CREATE:
			return "Cannot create the file.";
		case BASS_ERROR_NOFX:
			return "Effects are not available.";
		case BASS_ERROR_NOTAVAIL:
			return "Requested data is not available.";
		case BASS_ERROR_DECODE:
			return "The channel is a \"Decoding Channel\".";
		case BASS_ERROR_DX:
			return "A sufficiend DirectX Version is not installed.";
		case BASS_ERROR_TIMEOUT:
			return "Connection Timed Out.";
		case BASS_ERROR_FILEFORM:
			return "Unsupported File Format.";
		case BASS_ERROR_SPEAKER:
			return "Unavailable Speaker.";
		case BASS_ERROR_VERSION:
			return "Invalid Plugin Bass Version.";
		case BASS_ERROR_CODEC:
			return "Codec is not available/supported.";
		case BASS_ERROR_ENDED:
			return "The channel/file has ended.";
		case BASS_ERROR_BUSY:
			return "The device is busy.";
		case BASS_ERROR_UNKNOWN:
		default:
			return "Unknown Error.";
		}
	}
	void InitializeAudio(const unsigned long Frequency = 48000, const bool Mono = 0, const bool Use3D = 0, const bool Force8Bits = 0)
	{
		if(!BASS_Init(-1,Frequency, ((Mono ? BASS_DEVICE_MONO : 0)|(Use3D ? BASS_DEVICE_3D : 0)|(Force8Bits ? BASS_DEVICE_8BITS : 0)),0,0))
			BassError();
	}
#	endif
};

ConsoleImpl * cns(const ConsoleEvent * Event);

};