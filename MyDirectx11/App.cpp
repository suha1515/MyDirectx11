#include "App.h"
#include "Box.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "BsMath.h"
#include "Surface.h"
#include "GDIPlusManager.h"
#include "imgui/imgui.h"

namespace dx = DirectX;

GDIPlusManager gdipm;

App::App()
	:wnd(800,600,"My Window"),light(wnd.Gfx())
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
			const DirectX::XMFLOAT3 mat = { cdist(rng),cdist(rng),cdist(rng) };
			return std::make_unique<Box>(
				gfx, rng, adist, ddist,
				odist, rdist, bdist,mat
				);
		}
	private:
		Graphics& gfx;
		std::mt19937 rng{ std::random_device{}() };
		std::uniform_real_distribution<float> adist{ 0.0f,PI * 2.0f };
		std::uniform_real_distribution<float> ddist{ 0.0f,PI * 0.5f };
		std::uniform_real_distribution<float> odist{ 0.0f,PI * 0.08f };
		std::uniform_real_distribution<float> rdist{ 6.0f,20.0f };
		std::uniform_real_distribution<float> bdist{ 0.4f,3.0f };
		std::uniform_real_distribution<float> cdist{ 0.0f,1.0f };
	};

	drawables.reserve(nDrawables);
	std::generate_n(std::back_inserter(drawables), nDrawables, Factory{ wnd.Gfx() });

	wnd.Gfx().SetProjection(DirectX::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 40.0f));
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
	//imgui에서 설정한 값으로 dt 조절로 물체의 빠르기를 정한다.
	auto dt = timer.Mark()* speed_factor;

	wnd.Gfx().BeginFrame(0.07f, 0.0f, 0.12f);
	wnd.Gfx().SetCamera(cam.GetMatrix());
	//점광원을 파이프라인에 바인딩
	light.Bind(wnd.Gfx());

	for (auto& d : drawables)
	{
		d->Update(wnd.kbd.KeyIsPressed(VK_SPACE)?0.0f:dt);
		d->Draw(wnd.Gfx());
	}
	//광원의 위치를 그린다.
	light.Draw(wnd.Gfx());

	//imgui 윈도우를 만들며 시뮬레이션 스피드를 정한다.
	if (ImGui::Begin("Simulation Speed"))	//Begin함수가 윈도우를 만들며 윈도우이름이 정해진다 만약 최소화 될경우 false 반환이다(내부 구성이 생기지 않음)
	{
		ImGui::SliderFloat("Speed Factor", &speed_factor, 0.0f, 4.0f);		//슬라이더를 만드는 Imgui
		ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Text("Status: %s", wnd.kbd.KeyIsPressed(VK_SPACE) ? "PAUSED" : "RUNNING");
	}
	ImGui::End();//모든 작업이 끝나면 End 함수 호출하여 렌더를 진행한다 기존의 Gui 시스템과 다르게 imgui는 매 프레임마다 gui들을 렌더링하는형식이다 (기존은 정적임)
	//imgui window to control Camera
	cam.SpwanControlWindow();
	//imgui window to control PointLight;
	light.SpawnControlWindow();
	
	//present
	wnd.Gfx().EndFrame();
} 
