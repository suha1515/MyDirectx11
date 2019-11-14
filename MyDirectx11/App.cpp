#include "App.h"
//Drawable-------
#include "Box.h"
#include "Cylinder.h"
#include "Pyramid.h"
#include "SkinnedBox.h"
#include "AssetTest.h"
//---------------
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
	wnd.Gfx().SetProjection(DirectX::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 40.0f));
}

void App::DoFrame()
{
	//imgui에서 설정한 값으로 dt 조절로 물체의 빠르기를 정한다.
	auto dt = timer.Mark() * speed_factor;

	wnd.Gfx().BeginFrame(0.07f, 0.0f, 0.12f);
	wnd.Gfx().SetCamera(cam.GetMatrix());
	//점광원을 파이프라인에 바인딩
	light.Bind(wnd.Gfx(), cam.GetMatrix());

	//모델의 트랜스폼 지정
	const auto transform = dx::XMMatrixRotationRollPitchYaw(pos.roll, pos.pitch, pos.yaw)
		* dx::XMMatrixTranslation(pos.x, pos.y, pos.z);
	nano.Draw(wnd.Gfx(),transform);

	//광원의 위치를 그린다.
	light.Draw(wnd.Gfx());

	//imgui window
	cam.SpwanControlWindow();
	light.SpawnControlWindow();
	ShowModelWindow();

	//present
	wnd.Gfx().EndFrame();
}
void App::ShowModelWindow()
{
	if (ImGui::Begin("Model"))
	{
		using namespace std::string_literals;

		ImGui::Text("Orientation");
		ImGui::SliderAngle("Roll", &pos.roll, -180.f, 180.f);
		ImGui::SliderAngle("Pitch", &pos.pitch, -180.f, 180.f);
		ImGui::SliderAngle("Yaw", &pos.yaw, -180.f, 180.f);

		ImGui::Text("Position");
		ImGui::SliderFloat("X", &pos.x, -20.0f, 20.0f);
		ImGui::SliderFloat("Y", &pos.x, -20.0f, 20.0f);
		ImGui::SliderFloat("Z", &pos.x, -20.0f, 20.0f);
	}
	ImGui::End();
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