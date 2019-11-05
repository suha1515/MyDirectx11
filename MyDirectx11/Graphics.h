#pragma once
#include "BsWin.h"
#include "BsException.h"
#include <d3d11.h>
#include <wrl.h>					//ComPointer가 있는 헤더
#include "DxgiInfoManager.h"

class Graphics
{
public:
	class Exception : public BsException
	{
		using BsException::BsException;
	};
	class HrException : public Exception 
	{
	public:
		HrException(int line, const char* file, HRESULT hr, std::vector<std::string>infoMsgs = {}) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		HRESULT GetErrorCode() const noexcept ;
		std::string GetErrorString() const noexcept ;
		std::string GetErrorDescription() const noexcept ;
		std::string GetErrorInfo() const noexcept;
	private:
		HRESULT hr;
		std::string info;
	};
	//HRESULT 결과를 담지않는다 Info만 저장하는 예외클래스
	class InfoException :public Exception
	{
	public:
		InfoException(int line, const char* file, std::vector<std::string> infoMsgs) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		std::string GetErrorInfo() const noexcept;
	private:
		std::string info;
	};
	class DeviceRemovedException : public HrException
	{
		using HrException::HrException;
	public:
		const char* GetType() const noexcept override;
	private:
		std::string reason;
	};
public:
	Graphics(HWND hwnd);
	Graphics(const Graphics&) = delete;
	Graphics& operator=(const Graphics&) = delete;
	~Graphics() = default;

	void EndFrame();
	void ClearBuffer(float red, float green, float blue) noexcept;
	void DrawTestTrangle(float angle);
private:
#ifndef NDEBUG			//디버그모드에서만 인포매니저를 사용한다.
	DxgiInfoManager infoManager;
#endif
	Microsoft::WRL::ComPtr< ID3D11Device> pDevice;
	Microsoft::WRL::ComPtr< IDXGISwapChain> pSwap;
	Microsoft::WRL::ComPtr< ID3D11DeviceContext> pContext;
	Microsoft::WRL::ComPtr< ID3D11RenderTargetView> pTarget;
	
};

