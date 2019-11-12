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

	std::optional<int> comboBoxIndex;		//�޺��ڽ� �ε���
	std::set<int>	boxControllds;			//�ڽ���Ʈ��
};

