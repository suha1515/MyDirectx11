#pragma once
//App 클래스
/*
	WinMain 함수가 더러워지는것을 막기위해
	필요한 메시지루프와 기능들을 래핑한다.
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
	//모델 imgui 윈도우
	void ShowModelWindow();
private:
	ImguiManager imgui;
	Window wnd;
	Timer timer;
	float	speed_factor = 1.0f;
	Camera cam;
	PointLight light;
	Model nano{ wnd.Gfx(),"Models//nanosuit.obj" };

	//모델을 위한 트랜스폼 파라메터
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

