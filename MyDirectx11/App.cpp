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
		//optional은 bool을 오버로드하여 만약 값이 있다면 true이다
		//ProcessMessages 함수에서 PeekMessage 호 출시 WM_QUIT을 만나면 값이있는 msg.wParam을 optional로 반환하였으므로
		//while문을 빠져나간다
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
	//setprecision은  fixed 존재시 정수부+소수부 기준으로 다섯자리 반올림 없을시 소수부 5자리 반올림
	const float c = sin(timer.Peek()) / 2.0f + 0.5f;
	wnd.Gfx().ClearBuffer(c, c, 1.0f);
	wnd.Gfx().DrawTestTrangle(timer.Peek());
	wnd.Gfx().EndFrame();
} 
