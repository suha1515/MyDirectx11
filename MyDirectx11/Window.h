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
	void EnableCursor() noexcept;			//���콺 Ŀ�� Ȱ��ȭ
	void DisableCursor()noexcept;			//���콺 Ŀ�� ��Ȱ��ȭ
	bool CursorEnabled() const noexcept;	
	static std::optional<int> ProcessMessages() noexcept;

	Graphics& Gfx();

private:
	void ConfineCursor() noexcept;		//���콺 ����
	void FreeCursor() noexcept;			//���콺 ��������
	void HideCursor() noexcept;			//���콺 �����
	void ShowCursor() noexcept;			//���콺 ���̱�
	void EnableImGuiMouse() noexcept;	//imgui ���콺 Ȱ��ȭ
	void DisableImGuiMouse() noexcept;	//imgui ���콺 Ȱ��ȭ
	static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
public:
	bool cursorEnabled = true;
	Keyboard kbd;						//Ű���� ��ü
	Mouse	 mouse;						//���콺 ��ü
	std::unique_ptr<Graphics> pGfx;		//�׷��� ����̽�
private:
	int width;
	int height;
	HWND hWnd;
	std::vector<BYTE> rawBuffer;

};

