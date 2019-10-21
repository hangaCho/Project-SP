#include <os/winconsole/console.h>
#include <os/time.h>

namespace stlx {

namespace Log {
	ConsoleOut< ConsoleType::Out > cout;
	ConsoleOut< ConsoleType::Log > clog;
	ConsoleOut< ConsoleType::Warning > cwar;
	ConsoleOut< ConsoleType::Error > cerr;
};

void Console::OnInit() {
	char title[512];
	c->GetTitle(title, sizeof(title));
	Log::Out(title, "\n");
	// ContinueButton = c->AddTextButton("button");
}

void Console::OnKeyInput(const KeyboardEvent * ev) {
	return;
	static Time::Val last_esc = 0;

	if(ev->Key == VK_ESCAPE && !ev->Down) {
		if(Time::Elapsed(last_esc) < 1000)
			c->Close();
		else {
			last_esc = Time::Now();
			Log::Out("Press Escape again to quit.");
		}
	}
}

void Console::OnUpdate() {
}

void Console::OnMouseInput(const MouseEvent *) {
}

void Console::OnClose() {
	//cout << "closing";
}

void Console::OnButtonPress(const ButtonEvent * /*ev*/) {
	//if(Event->Id == ContinueButton)
	//{
	//	c->FillConsole();
	//	c->RemoveButton(ContinueButton);
	//}
}

};