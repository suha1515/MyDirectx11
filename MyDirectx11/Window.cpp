#include "Window.h"
#include <sstream>
#include "resource.h"
#include "WindowsThrowMacros.h"
#include "imgui/imgui_impl_win32.h"

Window::WindowClass Window::WindowClass::wndClass;


Window::WindowClass::WindowClass() noexcept
	: hInst(GetModuleHandle(nullptr))
{
	// register window class
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = HandleMsgSetup;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetInstance();
	wc.hIcon = static_cast<HICON>(LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 32, 32, 0));
	wc.hCursor = nullptr;
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = GetName();
	wc.hIconSm = static_cast<HICON>(LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 16, 16, 0));;
	RegisterClassEx(&wc);
}

Window::WindowClass::~WindowClass()
{
	UnregisterClass(wndClassName, GetInstance());
}

const char* Window::WindowClass::GetName() noexcept
{
	return wndClassName;
}

HINSTANCE Window::WindowClass::GetInstance() noexcept
{
	return wndClass.hInst;
}


Window::Window(int width, int height, const char* name)
	:width(width),height(height),hWnd(nullptr)
{

	//Ŭ���̾�Ʈ�� ������ ������� ������ ũ�⸦ ����Ѵ�.
	RECT wr;
	wr.left = 100;
	wr.right = width + wr.left;
	wr.top = 100;
	wr.bottom = height + wr.top;

	if (AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE) == 0)
	{
		throw BSWND_LAST_EXCEPT();
	}

	// Create window instance
	    hWnd = CreateWindow(WindowClass::GetName(), name,
		WS_CAPTION | WS_MINIMIZE | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
		nullptr, nullptr, WindowClass::GetInstance(), this);
	//������ ������ void �������ε� WindowŬ���� �ּҸ� �ѱ����ν� ���߿� WM_CREATE�� WM_NCCREATE�޽��� �߻���
	//lParam�� ���޵Ǵ� CREATESTURCTW ����ü�� lpCreateParams�� ���޵ǰ� �ȴ�.

	if (hWnd == nullptr)
	{
		throw BSWND_LAST_EXCEPT();
	}

	ShowWindow(hWnd, SW_SHOWDEFAULT);
	//Initialize ImGui Win32 Implement
	ImGui_ImplWin32_Init(hWnd);
	//�׷��� ��ü ����
	pGfx = std::make_unique<Graphics>(hWnd, width,height);
}

Window::~Window()
{
	ImGui_ImplWin32_Shutdown();
	DestroyWindow(hWnd);
}

void Window::SetTitle(const std::string title)
{
	if (SetWindowText(hWnd, title.c_str()) == 0)
		throw BSWND_LAST_EXCEPT();
}

std::optional<int> Window::ProcessMessages() noexcept
{
	MSG msg;
	// �޽����� ť�� �ִµ��ȿ� �޽����� �������� �����Ѵ�(PeekMeesage) ��ϻ��¿� ������ �ʴ´�
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		//GetMessage�� WM_QUIT�� ��ȯ���� ������ PeekMeesage�� �ܼ��� �޽����� �ִ��� �������� ���� ��ȯ�ϹǷ�
		//WM_QUIT�� ���ؼ� ���� �˻�������Ѵ�.
		if (msg.message == WM_QUIT)
			return msg.wParam;		

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	//C++17 optionnal�� �Լ��� ��ȯ���� �ϳ����ƴ� <T> �� ���� �ƹ��� ���� ����{} ���� ��ȯ�� �� �ִ�.
	return {};
}

Graphics& Window::Gfx()
{
	return *pGfx;
}

LRESULT WINAPI Window::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_NCCREATE)
	{
		const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
		Window* const pWnd = static_cast<Window*>(pCreate->lpCreateParams);

		//�Ʒ��� �Լ��� Ư�� Window�� �Ӽ����� �����Ѵ�. �Ӹ� �ƴ϶� ������ Window �޸𸮿� Ư�� ���� �����Ҽ� �ִ�.
		//�� ������ WNDCLASSEX ����ü �ۼ��� ������ ������� ���� �Ӽ����� �����Ҷ� ����Ѵ�.
		//���� GWLP_USERDATA�� �����츦 �����Ҷ� ����ϱ����� �������ν� �ʱⰪ�� 0�̴�.
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));

		//GWLP_WNDPROC�� WndProc �� �������ν����� ������ ���ο� �ּҸ� �����ϴ� ���̴�.
		//�� �Ʒ� �ڵ�� WinApi�� C��Ÿ���� �ڵ��̹Ƿ� C++�� Ŭ���� ��������� ���� ���ν����� �����Ҽ�����
		//������ ��ȸ�� SetWindowLongPtr�� ����Ͽ� ��������� �ּҸ� �Ѱ� �������ν����� ����ϵ����Ѵ�. (�ش� ��������� CALLBACK �̾���Ѵ�)
		//���� Setup�� Thunk�Լ� �Ѵ� static �̹Ƿ� ���������� API���ν����� ����� �����ϴ�
		//������ Setup�� �ʱ� �����̴�.
		SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMsgThunk));

		return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT WINAPI Window::HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//�� HandleMsgSetUp ���� HandleMsgThunk�� ���� ���ν����� ����ϸ鼭 �޽��� �߻��� HandleMsgThunk���� ó���ϰ� �Ǵµ�
	//HandleMsgThunk���� Setup���� ������ GWLP_USERDATA�� ����� Window ��ü�� �ּҸ� �ٽ� ĳ�����Ͽ� 
	//�ش� ��ü�� handleMsg�� ȣ���Ͽ� Ŭ���� ��������� ���ν����� ����ϰ� �ȴ�.
	Window* const pWnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
}

LRESULT Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	const auto& imio = ImGui::GetIO();

	//WM_CLOSE �޽��� �߻��� return 0�� �ϴ� ������ ����ó�� PostQuitMessage(69)�߻���
	//�ü������ �ش� �����츦 ���ش� ������ ���� ���ν����� Ŭ�����ȿ��� ���ư��� �ִ� ���� �����ϸ�
	//�ش� Ŭ������ �Ҹ��ڿ��� �����츦 ���ִ°��� �´� �׷��� ������ �ι� �Ҹ��ڰ� ȣ��ɰ��̴�.
	switch (msg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
		/******************************KEYBOARD MESSAGE***********************************/
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN: //��� �ý��� �޽����� ���´�.
		//imgui�� �߰��� imgui���� ĸó�ϱ� ���ϴ� Ű�� ������� Ű���� �޽����� ����æ��.
		if (imio.WantCaptureKeyboard)
			break;
		if (!(lParam & 0x40000000) || kbd.AutorepeatIsEnabled())
		{
			kbd.OnKeyPressed(static_cast<unsigned char>(wParam));
		}
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		if (imio.WantCaptureKeyboard)
			break;
		kbd.OnKeyReleased(static_cast<unsigned char>(wParam));
		break;
	case WM_CHAR:
		if (imio.WantCaptureKeyboard)
			break;
		kbd.OnChar(static_cast<unsigned char>(wParam));
		break;
	case WM_KILLFOCUS:
		kbd.ClearState();
		break;
		/******************************KEYBOARD MESSAGE***********************************/

		/*********************************MOUSE MESSAGE***********************************/
	case WM_MOUSEMOVE:
	{	
		if (imio.WantCaptureKeyboard)
			break;
		const POINTS pt = MAKEPOINTS(lParam);

		//Ŭ���̾�Ʈ ������ �������. 
		if (pt.x >= 0 && pt.x < width && pt.y >= 0 && pt.y < height)
		{
			mouse.OnMouseMove(pt.x, pt.y);
			if (!mouse.IsInWindow())
			{
				SetCapture(hWnd);		//SetCaputure �Լ��� ���콺�� ������ ������ ��������. ����ؼ� ���콺�� ���� �޴´�.
										//Ȥ�� ���콺��ư�� ����ä�� ������ ���� �������� ĸó�� ����Ѵ�.
				mouse.OnMouseEnfer();
			}
		}
		//Ŭ���̾�Ʈ ���� �ۿ� �������
		else
		{
			if (wParam & (MK_LBUTTON | MK_RBUTTON))
				mouse.OnMouseMove(pt.x, pt.y);		//�������·� ���콺�� ������ �������
			else
			{
				ReleaseCapture();					//������ �������� ĸó�� �׸��д�.
				mouse.OnMouseLeave();
			}
		}
		break;
	}
	case WM_LBUTTONDOWN:
	{
		SetForegroundWindow(hWnd);
		if (imio.WantCaptureKeyboard)
			break;
		const POINTS pt = MAKEPOINTS(lParam);
		mouse.OnLeftPressed(pt.x, pt.y);
		break;
	}
	case WM_RBUTTONDOWN:
	{
		if (imio.WantCaptureKeyboard)
			break;
		const POINTS pt = MAKEPOINTS(lParam);
		mouse.OnRightPressed(pt.x, pt.y);
		break;
	}
	case WM_LBUTTONUP:
	{
		if (imio.WantCaptureKeyboard)
			break;
		const POINTS pt = MAKEPOINTS(lParam);
		mouse.OnLeftReleased(pt.x, pt.y);
		break;
	}
	case WM_RBUTTONUP:
	{
		if (imio.WantCaptureKeyboard)
			break;
		const POINTS pt = MAKEPOINTS(lParam);
		mouse.OnLeftReleased(pt.x, pt.y);
		break;
	}
	case WM_MOUSEWHEEL:
	{
		if (imio.WantCaptureKeyboard)
			break;
		const POINTS pt = MAKEPOINTS(lParam);
		const int delta = GET_WHEEL_DELTA_WPARAM(wParam);	//���Լ��� Wheel ���� �������� 120 �̻��̸� ������ 120 �Ʒ��� �Ʒ������̴�
		mouse.OnWheelDelta(pt.x, pt.y, delta);
		break;
	}
	/*********************************MOUSE MESSAGE***********************************/
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}
std::string Window::Exception::TranslateErrorCode(HRESULT hr) noexcept
{
	char* pMsgBuf = nullptr;
	//FormatMessage�� hr�ڵ忡���� ������ string���� �����Ѵ�.
	//�پ��� �÷��װ��ִµ� FORMAT_MEESAGE_ALLOCATE_BUFFER �÷��׷� ���ο� ���۸� �Ҵ��Ͽ�
	//����ڰ� ������ �����Ͱ� ����Ű���� �Ѵ�.(pMsgBuf) �׸��� �ش� ���۴� �����޽����� ������ ����ִ�.
	DWORD nMsgLen = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPSTR>(&pMsgBuf), 0, nullptr);

	//0�ϰ�� ���� �߸���ٴ¶�.
	if (nMsgLen == 0)
	{
		return "Unidentified error code";
	}
	std::string errorString = pMsgBuf;
	//LocalFree�� ������ pMsgBuf�� �Ҵ�Ǿ����Ƿ� �������ִ� �Լ��̴�.
	LocalFree(pMsgBuf);
	return errorString;
}

Window::HrException::HrException(int line,const char* file,HRESULT hr) noexcept
	: Exception(line,file), hr(hr)
{}

const char* Window::HrException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
		<< std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
		<< "[Description] " << GetErrorDescription() << std::endl
		<< GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Window::HrException::GetType() const noexcept
{
	return "Bs Window Exception";
}

HRESULT Window::HrException::GetErrorCode() const noexcept
{
	return hr;
}

std::string Window::HrException::GetErrorDescription() const noexcept
{
	return Exception::TranslateErrorCode(hr);
}

const char* Window::NoGfxException::GetType() const noexcept
{
	return "Bs Window Exception [No Graphics]";
}