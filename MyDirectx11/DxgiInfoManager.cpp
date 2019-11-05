#include "DxgiInfoManager.h"
#include "Window.h"
#include "Graphics.h"
#include <dxgidebug.h>
#include <memory>

#pragma comment(lib,"dxguid.lib")

#define GFX_THROW_NOINFO(hrcall) if(FAILED(hr = (hrcall))) throw Graphics::HrException(__LINE__,__FILE__,hr)

DxgiInfoManager::DxgiInfoManager()
{
	//DXGIGetDbugeInterface �� �Լ� �ñ״�ó �� �����Ѵ�.
	typedef HRESULT(WINAPI * DXGIGetDebugInterface)(REFIID, void**);

	//DXGIGetDebugInterface �Լ��� �����ϴ� Dll�� �ε��Ѵ�.
	const auto hModDxgiDebug = LoadLibraryEx("dxgidebug.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
	if (hModDxgiDebug == nullptr)
		throw BSWND_LAST_EXCEPT();


	// DXGIGetDebugInterface �� �ּҸ� dll���� �޴´� (dll���� �ּҸ� �޴´�)
	const auto DxgiGetDebugInterface = reinterpret_cast<DXGIGetDebugInterface>
		(reinterpret_cast<void*>(GetProcAddress(hModDxgiDebug, "DXGIGetDebugInterface")));

	if (DxgiGetDebugInterface == nullptr)
		throw BSWND_LAST_EXCEPT();
	
	HRESULT hr;
	//interface�� ������� �Լ��� ȣ���Ѵ�. (dxgiGetDebugInteface �Լ������Ͱ� �ҷ�������)
	//GFX_THROW_NOINFO(DxgiGetDebugInterface(__uuidof(IDXGIInfoQueue), reinterpret_cast<void*>(&pDxgiInfoQueue)));

	//Comptr�� ��ü
	GFX_THROW_NOINFO(DxgiGetDebugInterface(__uuidof(IDXGIInfoQueue), &pDxgiInfoQueue));

}
void DxgiInfoManager::Set() noexcept
{
	// GetMessage() �Լ��� ��� �޽��� ������ �ε����� �����Ѵ�
	// ������ �Ʒ� �Լ� ȣ�� ���Ŀ� ��������̴�.

	next = pDxgiInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
}

//GetMessages�� �����ĺ��� ��� �޽����� ���Ϳ� ��ԵǴµ�
//set�Լ��� ȣ���Ͽ� ���޽��� ������ �޽����� �޵����Ѵ�.
std::vector<std::string> DxgiInfoManager::GetMessages() const
{
	std::vector<std::string> messages;
	const auto end = pDxgiInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
	for (auto i = next; i < end; i++)
	{
		HRESULT hr;
		SIZE_T messageLength=0;
		//�޽����� ũ�⸦ ��´�.
		GFX_THROW_NOINFO(pDxgiInfoQueue->GetMessageA(DXGI_DEBUG_ALL, i, nullptr, &messageLength));
		//�޽����� ���� �޸𸮸� �Ҵ��Ѵ�.
		auto bytes = std::make_unique<byte[]>(messageLength);
		auto pMessage = reinterpret_cast<DXGI_INFO_QUEUE_MESSAGE*>(bytes.get());
		//�޽����� ���������� ���Ϳ� �����Ѵ�.
		GFX_THROW_NOINFO(pDxgiInfoQueue->GetMessageA(DXGI_DEBUG_ALL, i, pMessage, &messageLength));
		messages.emplace_back(pMessage->pDescription);
	}

	return messages;
}
