#pragma once
//App Ŭ����
/*
	WinMain �Լ��� ���������°��� ��������
	�ʿ��� �޽��������� ��ɵ��� �����Ѵ�.
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
	float	speed_factor = 1.0f;
	static constexpr size_t nDrawables = 180;
};

