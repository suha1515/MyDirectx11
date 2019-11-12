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
#include <set>

class App
{
public:
	App();
	int Go();
	~App();
private:
	void DoFrame();
	void SpawnSimulationWindow() noexcept;
	void SpawnBoxWindowManagerWindow() noexcept;
	void SpawnBoxWindows() noexcept;
private:
	ImguiManager imgui;
	Window wnd;
	Timer timer;
	std::vector<std::unique_ptr<class Drawable>> drawables;
	std::vector<class Box*> boxes;
	float	speed_factor = 1.0f;
	Camera cam;
	PointLight light;
	static constexpr size_t nDrawables = 180;

	std::optional<int> comboBoxIndex;		//콤보박스 인덱스
	std::set<int>	boxControllds;			//박스컨트롤
};

