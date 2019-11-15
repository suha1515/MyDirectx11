#pragma once
#include "BsWin.h"
#include "BsException.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Graphics.h"
#include <memory>


//WINAPI를 래핑하기위한 작업

class Window
{
public:
	class Exception :public BsException
	{
		using BsException::BsException;
	public:
		static std::string TranslateErrorCode(HRESULT hr) noexcept;
	};
	class HrException : public Exception
	{
	public:
		HrException(int line, const char* file, HRESULT hr) noexcept ;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;

		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorDescription() const noexcept;
	private:
		HRESULT hr;
	};
	class NoGfxException : public Exception
	{
	public:
		using Exception::Exception;
		const char* GetType() const noexcept override;
	};
private:
	class WindowClass
	{
	public:
		static const char* GetName() noexcept;
		static HINSTANCE  GetInstance() noexcept;
	private:
		WindowClass() noexcept;
		~WindowClass() noexcept;
		WindowClass(const WindowClass&) = delete;
		WindowClass& operator =(const WindowClass&) = delete;
		static constexpr const char* wndClassName = "Bs Direct3D Engine Window";
		static WindowClass wndClass;
		HINSTANCE hInst;
	};

public:
	Window(int width, int height, const char* name);
	~Window();
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
	void SetTitle(const std::string title);
	void EnableCursor() noexcept;			//마우스 커서 활성화
	void DisableCursor()noexcept;			//마우스 커서 비활성화
	bool CursorEnabled() const noexcept;	
	static std::optional<int> ProcessMessages() noexcept;

	Graphics& Gfx();

private:
	void ConfineCursor() noexcept;		//마우스 제한
	void FreeCursor() noexcept;			//마우스 제한해제
	void HideCursor() noexcept;			//마우스 숨기기
	void ShowCursor() noexcept;			//마우스 보이기
	void EnableImGuiMouse() noexcept;	//imgui 마우스 활성화
	void DisableImGuiMouse() noexcept;	//imgui 마우스 활성화
	static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
public:
	bool cursorEnabled = true;
	Keyboard kbd;						//키보드 객체
	Mouse	 mouse;						//마우스 객체
	std::unique_ptr<Graphics> pGfx;		//그래픽 디바이스
private:
	int width;
	int height;
	HWND hWnd;
	std::vector<BYTE> rawBuffer;

};

