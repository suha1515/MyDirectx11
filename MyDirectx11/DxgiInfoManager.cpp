#include "DxgiInfoManager.h"
#include "Window.h"
#include "Graphics.h"
#include <dxgidebug.h>
#include <memory>

#pragma comment(lib,"dxguid.lib")

#define GFX_THROW_NOINFO(hrcall) if(FAILED(hr = (hrcall))) throw Graphics::HrException(__LINE__,__FILE__,hr)

DxgiInfoManager::DxgiInfoManager()
{
	//DXGIGetDbugeInterface 의 함수 시그니처 을 정의한다.
	typedef HRESULT(WINAPI * DXGIGetDebugInterface)(REFIID, void**);

	//DXGIGetDebugInterface 함수를 포함하는 Dll을 로드한다.
	const auto hModDxgiDebug = LoadLibraryEx("dxgidebug.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
	if (hModDxgiDebug == nullptr)
		throw BSWND_LAST_EXCEPT();


	// DXGIGetDebugInterface 의 주소를 dll에서 받는다 (dll안의 주소를 받는다)
	const auto DxgiGetDebugInterface = reinterpret_cast<DXGIGetDebugInterface>
		(reinterpret_cast<void*>(GetProcAddress(hModDxgiDebug, "DXGIGetDebugInterface")));

	if (DxgiGetDebugInterface == nullptr)
		throw BSWND_LAST_EXCEPT();
	
	HRESULT hr;
	//interface를 얻기위해 함수를 호출한다. (dxgiGetDebugInteface 함수포인터가 불러와졌다)
	//GFX_THROW_NOINFO(DxgiGetDebugInterface(__uuidof(IDXGIInfoQueue), reinterpret_cast<void*>(&pDxgiInfoQueue)));

	//Comptr로 대체
	GFX_THROW_NOINFO(DxgiGetDebugInterface(__uuidof(IDXGIInfoQueue), &pDxgiInfoQueue));

}
void DxgiInfoManager::Set() noexcept
{
	// GetMessage() 함수의 모든 메시지 다음의 인덱스로 지정한다
	// 에러는 아래 함수 호출 이후에 얻어질것이다.

	next = pDxgiInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
}

//GetMessages는 실행후부터 모든 메시지를 벡터에 담게되는데
//set함수를 호출하여 모든메시지 다음의 메시지를 받도록한다.
std::vector<std::string> DxgiInfoManager::GetMessages() const
{
	std::vector<std::string> messages;
	const auto end = pDxgiInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
	for (auto i = next; i < end; i++)
	{
		HRESULT hr;
		SIZE_T messageLength=0;
		//메시지의 크기를 얻는다.
		GFX_THROW_NOINFO(pDxgiInfoQueue->GetMessageA(DXGI_DEBUG_ALL, i, nullptr, &messageLength));
		//메시지를 위한 메모리를 할당한다.
		auto bytes = std::make_unique<byte[]>(messageLength);
		auto pMessage = reinterpret_cast<DXGI_INFO_QUEUE_MESSAGE*>(bytes.get());
		//메시지와 에러설명을 벡터에 삽입한다.
		GFX_THROW_NOINFO(pDxgiInfoQueue->GetMessageA(DXGI_DEBUG_ALL, i, pMessage, &messageLength));
		messages.emplace_back(pMessage->pDescription);
	}

	return messages;
}
