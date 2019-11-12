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
		//optional�� bool�� �����ε��Ͽ� ���� ���� �ִٸ� true�̴�
		//ProcessMessages �Լ����� PeekMessage ȣ ��� WM_QUIT�� ������ �����ִ� msg.wParam�� optional�� ��ȯ�Ͽ����Ƿ�
		//while���� ����������
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
	//imgui���� ������ ������ dt ������ ��ü�� �����⸦ ���Ѵ�.
	auto dt = timer.Mark()* speed_factor;

	wnd.Gfx().BeginFrame(0.07f, 0.0f, 0.12f);
	wnd.Gfx().SetCamera(cam.GetMatrix());
	//�������� ���������ο� ���ε�
	light.Bind(wnd.Gfx());

	for (auto& d : drawables)
	{
		d->Update(wnd.kbd.KeyIsPressed(VK_SPACE)?0.0f:dt);
		d->Draw(wnd.Gfx());
	}
	//������ ��ġ�� �׸���.
	light.Draw(wnd.Gfx());

	//imgui �����츦 ����� �ùķ��̼� ���ǵ带 ���Ѵ�.
	if (ImGui::Begin("Simulation Speed"))	//Begin�Լ��� �����츦 ����� �������̸��� �������� ���� �ּ�ȭ �ɰ�� false ��ȯ�̴�(���� ������ ������ ����)
	{
		ImGui::SliderFloat("Speed Factor", &speed_factor, 0.0f, 4.0f);		//�����̴��� ����� Imgui
		ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Text("Status: %s", wnd.kbd.KeyIsPressed(VK_SPACE) ? "PAUSED" : "RUNNING");
	}
	ImGui::End();//��� �۾��� ������ End �Լ� ȣ���Ͽ� ������ �����Ѵ� ������ Gui �ý��۰� �ٸ��� imgui�� �� �����Ӹ��� gui���� �������ϴ������̴� (������ ������)
	//imgui window to control Camera
	cam.SpwanControlWindow();
	//imgui window to control PointLight;
	light.SpawnControlWindow();
	
	//present
	wnd.Gfx().EndFrame();
} 
