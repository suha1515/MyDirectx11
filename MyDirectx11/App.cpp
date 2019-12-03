#include "App.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "BsMath.h"
#include "Surface.h"
#include "GDIPlusManager.h"
#include "imgui/imgui.h"
#include "VertexBuffer.h"
#include "NormalMapTwerker.h"

//? �ϴ�.. ���� Ŀ�ǵ� �����̶� �����ִ����..
// �Ƹ� �ܼ�âó�� Ŀ�ǵ带 �Է��Ҽ��ְ��ϴ°��ϵ�ʹ�..
#include <shellapi.h>

namespace dx = DirectX;

GDIPlusManager gdipm;

App::App(const std::string& commandLine)
	:
	commandLine(commandLine),
	wnd(1280, 720, "My Window"),
	light(wnd.Gfx())
{
	// makeshift cli for doing some preprocessing bullshit (so many hacks here)
	if (this->commandLine != "")
	{
		int nArgs;
		const auto pLineW = GetCommandLineW();
		const auto pArgs = CommandLineToArgvW(pLineW, &nArgs);
		if (nArgs >= 4 && std::wstring(pArgs[1]) == L"--ntwerk-rotx180")
		{
			const std::wstring pathInWide = pArgs[2];
			const std::wstring pathOutWide = pArgs[3];
			NormalMapTwerker::RotateXAxis180(
				std::string(pathInWide.begin(), pathInWide.end()),
				std::string(pathOutWide.begin(), pathOutWide.end())
			);
			throw std::runtime_error("Normal map processed successfully. Just kidding about that whole runtime error thing.");
		}
	}
	wall.SetRootTransform(dx::XMMatrixTranslation(-1.5f, 0.0f, 0.0f));
	tp.SetPos({ 12.0f,0.0f,0.0f });
	gobber.SetRootTransform(dx::XMMatrixTranslation(0.0f, 0.0f, -4.0f));
	nano.SetRootTransform(dx::XMMatrixTranslation(0.0f, -7.0f, 6.0f));
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
	wall.Draw(wnd.Gfx());
	tp.Draw(wnd.Gfx());
	nano.Draw(wnd.Gfx());
	gobber.Draw(wnd.Gfx());
	//������ ��ġ�� �׸���.
	light.Draw(wnd.Gfx());
	//�׽�Ʈ �÷���
	while (const auto e = wnd.kbd.ReadKey())
	{
		if (!e->IsPress())
		{
			continue;
		}
		switch(e->GetCode())
		{
		case VK_ESCAPE:
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
			break;
		case VK_F1:
			showDemoWindow = true;
			break;

		}
	}

	if (!wnd.CursorEnabled())
	{
		if (wnd.kbd.KeyIsPressed('W'))
		{
			cam.Translate({ 0.0f,0.0f,dt });
		}
		if (wnd.kbd.KeyIsPressed('A'))
		{
			cam.Translate({ -dt,0.0f,0.0f });
		}
		if (wnd.kbd.KeyIsPressed('S'))
		{
			cam.Translate({ 0.0f,0.0f,-dt });
		}
		if (wnd.kbd.KeyIsPressed('D'))
		{
			cam.Translate({ dt,0.0f,0.0f });
		}
		if (wnd.kbd.KeyIsPressed('R'))
		{
			cam.Translate({ 0.0f,dt,0.0f });
		}
		if (wnd.kbd.KeyIsPressed('F'))
		{
			cam.Translate({ 0.0f,-dt,0.0f });
		}
	}
	while (const auto delta = wnd.mouse.ReadRawDelta())
	{
		if (!wnd.CursorEnabled())
		{
			cam.Rotate((float)delta->x, (float)delta->y);
		}
	}

	//imgui window
	cam.SpwanControlWindow();
	light.SpawnControlWindow();
	ShowImguiDemoWindow();
	gobber.ShowWindow(wnd.Gfx(),"gobber");
	wall.ShowWindow(wnd.Gfx(),"Wall");
	tp.SpawnControlWindow(wnd.Gfx());
	nano.ShowWindow(wnd.Gfx(),"Nano");
	

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