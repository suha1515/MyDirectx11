#pragma once
//App 클래스
/*
	WinMain 함수가 더러워지는것을 막기위해
	필요한 메시지루프와 기능들을 래핑한다.
*/

#include "Window.h"
#include "Timer.h"

class App
{
public:
	App();
	int Go();
private:
	void DoFrame();
private:
	Window wnd;
	Timer timer;
};

