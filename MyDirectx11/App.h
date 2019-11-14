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
#include "Model.h"
#include <set>

class App
{
public:
	App();
	int Go();
	~App();
private:
	void DoFrame();
	//�� imgui ������
	void ShowModelWindow();
private:
	ImguiManager imgui;
	Window wnd;
	Timer timer;
	float	speed_factor = 1.0f;
	Camera cam;
	PointLight light;
	Model nano{ wnd.Gfx(),"Models//nanosuit.obj" };

	//���� ���� Ʈ������ �Ķ����
	struct
	{
		float roll = 0.0f;
		float pitch = 0.0f;
		float yaw = 0.0f;
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
	} pos;
};

