#include "App.h"
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
	:wnd(1280,720,"My Window"),light(wnd.Gfx())
{
	wnd.Gfx().SetProjection(DirectX::XMMatrixPerspectiveLH(1.0f, 9.0f / 16.0f, 0.5f, 40.0f));
}

void App::DoFrame()
{
	//imgui���� ������ ������ dt ������ ��ü�� �����⸦ ���Ѵ�.
	auto dt = timer.Mark() * speed_factor;

	wnd.Gfx().BeginFrame(0.07f, 0.0f, 0.12f);
	wnd.Gfx().SetCamera(cam.GetMatrix());
	//�������� ���������ο� ���ε�
	light.Bind(wnd.Gfx(), cam.GetMatrix());

	//�� �׸���
	nano.Draw(wnd.Gfx());

	//������ ��ġ�� �׸���.
	light.Draw(wnd.Gfx());

	while (const auto e = wnd.kbd.ReadKey())
	{
		if (e->IsPress() && e->GetCode() == VK_INSERT)
		{
			if (wnd.CursorEnabled())
			{
				wnd.DisableCursor();
				wnd.mouse.EnableRaw();
			}
			else
			{
				wnd.EnableCursor();
				wnd.mouse.DisableRaw();
			}
		}
	}
	//imgui window
	cam.SpwanControlWindow();
	light.SpawnControlWindow();
	ShowImguiDemoWindow();
	nano.ShowWindow();
	ShowRawInputWindow();


	//present
	wnd.Gfx().EndFrame();
}
void App::ShowImguiDemoWindow()
{
	static bool show_demo_window = true;
	if (show_demo_window)
	{
		ImGui::ShowDemoWindow(&show_demo_window);
	}
}
void App::ShowRawInputWindow()
{
	//���콺 ��ǥ���о imgui�� ǥ���Ѵ�.
	// ReadRawDelta�� optional�� ��ȯ�ϹǷ� ���н� false�� ��ȯ�ɰ��̴�
	while (const auto d = wnd.mouse.ReadRawDelta())
	{
		x += d->x;
		y += d->y;
	}
	if (ImGui::Begin("Raw Input"))
	{
		ImGui::Text("Tally: (%d,%d)", x, y);
		ImGui::Text("Cursor: %s", wnd.CursorEnabled() ? "enabled" : "disabled");
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