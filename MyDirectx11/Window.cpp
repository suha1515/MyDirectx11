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

	//클라이언트가 지정한 사이즈로 윈도우 크기를 계산한다.
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
	//마지막 변수는 void 포인터인데 Window클래스 주소를 넘김으로써 나중에 WM_CREATE나 WM_NCCREATE메시지 발생시
	//lParam에 전달되는 CREATESTURCTW 구조체의 lpCreateParams에 전달되게 된다.

	if (hWnd == nullptr)
	{
		throw BSWND_LAST_EXCEPT();
	}

	ShowWindow(hWnd, SW_SHOWDEFAULT);
	//Initialize ImGui Win32 Implement
	ImGui_ImplWin32_Init(hWnd);
	//그래픽 객체 생성
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
	// 메시지가 큐에 있는동안에 메시지를 내보내고 제거한다(PeekMeesage) 블록상태에 빠지지 않는다
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		//GetMessage는 WM_QUIT때 반환값이 있지만 PeekMeesage는 단순히 메시지가 있는지 없는지에 대해 반환하므로
		//WM_QUIT에 대해서 따로 검사해줘야한다.
		if (msg.message == WM_QUIT)
			return msg.wParam;		

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	//C++17 optionnal은 함수의 반환값이 하나가아닌 <T> 의 값과 아무런 값이 없는{} 값을 반환할 수 있다.
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

		//아래의 함수는 특정 Window의 속성들을 변경한다. 뿐만 아니라 여분의 Window 메모리에 특정 값을 지정할수 있다.
		//즉 기존에 WNDCLASSEX 구조체 작성후 윈도를 등록한후 각종 속성값을 변경할때 사용한다.
		//그중 GWLP_USERDATA는 윈도우를 생성할때 사용하기위한 공간으로써 초기값은 0이다.
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));

		//GWLP_WNDPROC은 WndProc 즉 윈도프로시져를 지정할 새로운 주소를 지정하는 곳이다.
		//즉 아래 코드는 WinApi는 C스타일의 코드이므로 C++의 클래스 멤버변수를 윈도 프로시져로 지정할수없다
		//그점을 우회해 SetWindowLongPtr을 사용하여 멤버변수의 주소를 넘겨 윈도프로시져로 사용하도록한다. (해당 멤버변수는 CALLBACK 이어야한다)
		//현재 Setup과 Thunk함수 둘다 static 이므로 직접적으로 API프로시저에 등록이 가능하다
		//하지만 Setup은 초기 설정이다.
		SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMsgThunk));

		return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT WINAPI Window::HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//위 HandleMsgSetUp 에서 HandleMsgThunk를 윈도 프로시저로 등록하면서 메시지 발생시 HandleMsgThunk에서 처리하게 되는데
	//HandleMsgThunk에서 Setup에서 지정한 GWLP_USERDATA에 등록한 Window 객체의 주소를 다시 캐스팅하여 
	//해당 객체의 handleMsg를 호출하여 클래스 멤버변수를 프로시저로 사용하게 된다.
	Window* const pWnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
}

LRESULT Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	const auto& imio = ImGui::GetIO();

	//WM_CLOSE 메시지 발생시 return 0을 하는 이유는 기존처럼 PostQuitMessage(69)발생시
	//운영체제에서 해당 윈도우를 없앤다 하지만 현재 프로시저가 클래스안에서 돌아가고 있는 점을 염두하면
	//해당 클래스의 소멸자에서 윈도우를 없애는것이 맞다 그렇지 않으면 두번 소멸자가 호출될것이다.
	switch (msg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
		/******************************KEYBOARD MESSAGE***********************************/
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN: //모든 시스템 메시지가 들어온다.
		//imgui의 추가로 imgui에서 캡처하기 원하는 키가 있을경우 키보드 메시지를 가로챈다.
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

		//클라이언트 영역에 있을경우. 
		if (pt.x >= 0 && pt.x < width && pt.y >= 0 && pt.y < height)
		{
			mouse.OnMouseMove(pt.x, pt.y);
			if (!mouse.IsInWindow())
			{
				SetCapture(hWnd);		//SetCaputure 함수는 마우스가 윈도우 밖으로 나가더라도. 계속해서 마우스의 값을 받는다.
										//혹은 마우스버튼을 누른채로 윈도우 밖을 나가더라도 캡처를 계속한다.
				mouse.OnMouseEnfer();
			}
		}
		//클라이언트 영역 밖에 있을경우
		else
		{
			if (wParam & (MK_LBUTTON | MK_RBUTTON))
				mouse.OnMouseMove(pt.x, pt.y);		//누른상태로 마우스가 밖으로 있을경우
			else
			{
				ReleaseCapture();					//누르지 않을경우는 캡처를 그만둔다.
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
		const int delta = GET_WHEEL_DELTA_WPARAM(wParam);	//이함수는 Wheel 값을 가져오며 120 이상이면 위로휠 120 아래면 아래로휠이다
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
	//FormatMessage는 hr코드에대한 설명을 string으로 전달한다.
	//다양한 플래그가있는데 FORMAT_MEESAGE_ALLOCATE_BUFFER 플래그로 새로운 버퍼를 할당하여
	//사용자가 정의한 포인터가 가르키도록 한다.(pMsgBuf) 그리고 해당 버퍼는 오류메시지의 설명이 들어있다.
	DWORD nMsgLen = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPSTR>(&pMsgBuf), 0, nullptr);

	//0일경우 뭔가 잘못됬다는뜻.
	if (nMsgLen == 0)
	{
		return "Unidentified error code";
	}
	std::string errorString = pMsgBuf;
	//LocalFree는 위에서 pMsgBuf가 할당되엇으므로 해제해주는 함수이다.
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