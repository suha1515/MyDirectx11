#pragma once
//App Ŭ����
/*
	WinMain �Լ��� ���������°��� ��������
	�ʿ��� �޽��������� ��ɵ��� �����Ѵ�.
*/

#include "Window.h"
#include "Timer.h"
#include "ImguiManager.h"
#include "Camera.h"
#include "PointLight.h"
#include "Mesh.h"
#include <set>

class App
{
public:
	App();
	int Go();
	~App();
private:
	void DoFrame();
	void ShowImguiDemoWindow();
	void ShowRawInputWindow();
private:
	bool cursorEnabled = true;
	int x = 0, y = 0;
	ImguiManager imgui;
	Window wnd;
	Timer timer;
	float	speed_factor = 1.0f;
	Camera cam;
	PointLight light;
	Model nano{ wnd.Gfx(),"Models\\nano.gltf" };
};

