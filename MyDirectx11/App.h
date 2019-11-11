#pragma once
//App 클래스
/*
	WinMain 함수가 더러워지는것을 막기위해
	필요한 메시지루프와 기능들을 래핑한다.
*/

#include "Window.h"
#include "Timer.h"
#include "ImguiManager.h"

class App
{
public:
	App();
	int Go();
	~App();
private:
	void DoFrame();
private:
	ImguiManager imgui;
	Window wnd;
	Timer timer;
	std::vector<std::unique_ptr<class Drawable>> drawables;
	static constexpr size_t nDrawables = 180;
};

