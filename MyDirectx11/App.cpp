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
	//imgui���� ������ ������ dt ������ ��ü�� �����⸦ ���Ѵ�.
	auto dt = timer.Mark() * speed_factor;

	wnd.Gfx().BeginFrame(0.07f, 0.0f, 0.12f);
	wnd.Gfx().SetCamera(cam.GetMatrix());
	//�������� ���������ο� ���ε�
	light.Bind(wnd.Gfx(), cam.GetMatrix());

	//���� Ʈ������ ����
	const auto transform = dx::XMMatrixRotationRollPitchYaw(pos.roll, pos.pitch, pos.yaw)
		* dx::XMMatrixTranslation(pos.x, pos.y, pos.z);
	nano.Draw(wnd.Gfx(),transform);

	//������ ��ġ�� �׸���.
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