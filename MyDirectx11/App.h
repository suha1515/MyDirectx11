#pragma once
//App Ŭ����
/*
	WinMain �Լ��� ���������°��� ��������
	�ʿ��� �޽��������� ��ɵ��� �����Ѵ�.
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

