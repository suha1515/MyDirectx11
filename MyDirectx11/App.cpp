#include "App.h"
#include "Box.h"
#include "Melon.h"
#include "Pyramid.h"
#include "Sheet.h"
#include "SkinnedBox.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "BsMath.h"
#include "Surface.h"
#include "GDIPlusManager.h"
#include "imgui/imgui.h"

GDIPlusManager gdipm;

App::App()
	:wnd(800,600,"My Window")
{
	class Factory
	{
	public:
		Factory(Graphics& gfx)
			:
			gfx(gfx)
		{}
		std::unique_ptr<Drawable> operator()()
		{
			switch (typedist(rng))
			{
			case 0:
				return std::make_unique<Pyramid>(
					gfx, rng, adist, ddist,
					odist, rdist
					);
			case 1:
				return std::make_unique<Box>(
					gfx, rng, adist, ddist,
					odist, rdist, bdist
					);
			case 2:
				return std::make_unique<Melon>(
					gfx, rng, adist, ddist,
					odist, rdist, longdist, latdist
					);
			case 3:
				return std::make_unique<Sheet>(gfx, rng, adist, ddist, odist, rdist);
			case 4:
				return std::make_unique<SkinnedBox>(gfx, rng, adist, ddist, odist, rdist);
			default:
				assert(false && "bad drawable type in factory");
				return {};
			}
		}
	private:
		Graphics& gfx;
		std::mt19937 rng{ std::random_device{}() };
		std::uniform_real_distribution<float> adist{ 0.0f,PI * 2.0f };
		std::uniform_real_distribution<float> ddist{ 0.0f,PI * 0.5f };
		std::uniform_real_distribution<float> odist{ 0.0f,PI * 0.08f };
		std::uniform_real_distribution<float> rdist{ 6.0f,20.0f };
		std::uniform_real_distribution<float> bdist{ 0.4f,3.0f };
		std::uniform_int_distribution<int> latdist{ 5,20 };
		std::uniform_int_distribution<int> longdist{ 10,40 };
		std::uniform_int_distribution<int> typedist{ 0,4 };
	};

	drawables.reserve(nDrawables);
	std::generate_n(std::back_inserter(drawables), nDrawables, Factory{ wnd.Gfx() });

	wnd.Gfx().SetProjection(DirectX::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 40.0f));
	const auto s = Surface::FromFile("Images\\Cat50.png");
	//const auto s = Surface::FromFile("Images\\kappa50.png");
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
App::~App()
{
}
void App::DoFrame()
{
	//const float t = timer.Peek();
	//std::ostringstream oss;
	//oss << "Time elapsed : " << std::setprecision(1) << std::fixed << t << "s";
	//wnd.SetTitle(oss.str());
	//setprecision은  fixed 존재시 정수부+소수부 기준으로 다섯자리 반올림 없을시 소수부 5자리 반올림
	auto dt = timer.Mark();

	if (wnd.kbd.KeyIsPressed(VK_SPACE))
		wnd.Gfx().DisableImgui();
	else
		wnd.Gfx().EnableImgui();
	wnd.Gfx().BeginFrame(0.07f, 0.0f, 0.12f);	//BeginFrame에 버퍼클리어등 래핑되어있다.

	for (auto& d : drawables)
	{
		//처음보는방법인데 삼항연산자로 업데이트를 종료할수있는게 신기하다
		//IsKeyPressed 로 키가 눌렸는지 확인하고 눌렸으면 0.0f 아니면 delttime전달삼항연산자.. 자주보자..
		d->Update(wnd.kbd.KeyIsPressed(VK_SPACE)?0.0f:dt);
		d->Draw(wnd.Gfx());
	}

	if (show_demo_window)
	{
		ImGui::ShowDemoWindow(&show_demo_window);
	}

	wnd.Gfx().EndFrame();
} 
