#include "App.h"
#include <sstream>
#include <iomanip>

App::App()
	:wnd(800,600,"My Window")
{
}

int App::Go()
{
	while (true)
	{
		//optional�� bool�� �����ε��Ͽ� ���� ���� �ִٸ� true�̴�
		//ProcessMessages �Լ����� PeekMessage ȣ ��� WM_QUIT�� ������ �����ִ� msg.wParam�� optional�� ��ȯ�Ͽ����Ƿ�
		//while���� ����������
		if (const auto ecode = Window::ProcessMessages())
			return *ecode;
		DoFrame();
	}
}

void App::DoFrame()
{
	//const float t = timer.Peek();
	//std::ostringstream oss;
	//oss << "Time elapsed : " << std::setprecision(1) << std::fixed << t << "s";
	//wnd.SetTitle(oss.str());
	//setprecision��  fixed ����� ������+�Ҽ��� �������� �ټ��ڸ� �ݿø� ������ �Ҽ��� 5�ڸ� �ݿø�
	const float c = sin(timer.Peek()) / 2.0f + 0.5f;
	wnd.Gfx().ClearBuffer(c, c, 1.0f);
	wnd.Gfx().DrawTestTrangle(timer.Peek());
	wnd.Gfx().EndFrame();
} 
