#pragma once
#include "BsWin.h"
#include "BsException.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Graphics.h"
#include <memory>


//WINAPI�� �����ϱ����� �۾�

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
	static std::optional<int> ProcessMessages() noexcept;

	Graphics& Gfx();

private:
	static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
public:
	Keyboard kbd;						//Ű���� ��ü
	Mouse	 mouse;						//���콺 ��ü
	std::unique_ptr<Graphics> pGfx;		//�׷��� ����̽�
private:
	int width;
	int height;
	HWND hWnd;

};

//����Ű� �ִ� �����Ϸ��� ���� ����ο��� ������ ������ ����ϴµ�
//__LINE__ ��ũ�ΰ� ��������� �˷��ָ� __FILE__�� ����Ͽ��� ������ ������ �˷��ش�.
#define BSWND_EXCEPT(hr) Window::HrException(__LINE__,__FILE__,hr);
#define BSWND_LAST_EXCEPT() Window::HrException(__LINE__,__FILE__,GetLastError());
#define BSWND_NOGFX_EXCEPT() Window::NoGfxException(__LINE__,__FILE__);