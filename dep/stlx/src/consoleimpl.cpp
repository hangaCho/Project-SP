#include <os/winconsole/consoleimpl.h>

namespace stlx {

ConsoleImpl g_con;

bool ConsoleImpl::IsWaiting[ConsoleImpl::REASONS];
bool * ConsoleImpl::isKeyDownArray = new bool[0xFF];
bool * ConsoleImpl::wasKeyDownArray = new bool[0xFF];
void * prioritize_memset_01 = memset(ConsoleImpl::GetIsKeyDownArray(), 0, sizeof(bool) * (0xFF));
void * prioritize_memset_02 = memset(ConsoleImpl::GetWasKeyDownArray(), 0, sizeof(bool) * (0xFF));
unsigned long ConsoleImpl::WaitingTime[ConsoleImpl::REASONS];
ConsoleImpl * cns(const ConsoleEvent * Event) {
	return (ConsoleImpl *)Event->Console;
}

};