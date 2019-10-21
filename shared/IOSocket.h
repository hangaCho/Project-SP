#ifndef __IOSOCKET_H__
#define __IOSOCKET_H__

#include <Windows.h>

typedef int(__fastcall *fCall)(u8 *);
class IOSocket {
private:
	HMODULE hModule;

public:
	IOSocket();
	~IOSocket();

	fCall MakeDigest;
};


#endif
