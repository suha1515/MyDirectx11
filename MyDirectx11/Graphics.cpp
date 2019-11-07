#include "Graphics.h"
#include "dxerr.h"
#include <sstream>
#include <d3dcompiler.h>
#include <cmath>
#include <DirectXMath.h>
#include "GraphicsThrowMacros.h"

namespace wrl = Microsoft::WRL;
//DirectxMath 네임스페이스
namespace dx = DirectX;


#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"D3DCompiler.lib")


Graphics::Graphics(HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC sd = {};
	//---버퍼 가로,세로 포맷,주기등 버퍼정보 구조체
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	//---------------------------------------------
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.OutputWindow = hWnd;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;
	
	UINT swapCreateFlags = 0u;
#ifndef NDEBUG	
	swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
	//현재 기존의 Dxerr 이 반환하는 HRESULT가 Winapi와 맞지않아 클래스를 따로 만들었다.
	//그리하여 문제의 라인으로 예외처리를하는데 이 플래그를 설정하면 출력창에 DXGI WARNING 으로 문제와 경고를 내준다.
	//위에서 구조체 정보를 채울때 없는 핸들을 전달하면 이 장치초기화에서 문제가 일어나 우리의 예외처리는 이 함수라인을 가르키게된다.
	//또한 DXGI ERROR 메시지가 발생하여 출력창에서도 확인할수 있다.
	//그렇다면.. 이러한 에러를 예외메시지로 날릴수는 없을까? DxgiInfoManager 헤더 확인
#endif

	HRESULT hr = 0;
	//디바이스와 전면,후면 버퍼,스왑체인,렌더링 컨텍스트를 생성한다.
	GFX_THROW_INFO(D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		swapCreateFlags,			
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&sd,
		&pSwap,
		&pDevice,
		nullptr,
		&pContext));

	//스왑체인에 있는 텍스쳐 서브리소스(백버퍼)의 접근을 얻는다
	//변경됨 기존에는 ID3D11Resource 인터페이스 포인터를 직접 선언했지만 이제 스마트 포인터를 사용한다.
	wrl::ComPtr<ID3D11Resource> pBackBuffer;
	//스마트 포인터를 만든이유는 만약 CreatRenderTargetView 구문에서 예외를던져
	//한번만 사용하게될 pBackBuffer가 Release를 호출하지않으면 pBackBuffer는 댕글링 포인터가된다
	//그렇기에 com객체에 스마트 포인터 개념을 도입한 ComPtr을 사용한다.

	// GetBuffer
	// IUnkown com객체에서 Queryinterface랑 비슷하다 비슷한것이 맞다.
	// 첫번째 매개변수는 버퍼에대한 인덱스로 0번 인덱스는 백버퍼이며 2번째 인자는 UUID로 해당 ID3D11Resource 인터페이스의 UUID이다
	// 3번재 매개변수는 해당 리소스인터페이스의 정보를 받은 이중포인터이다.
	GFX_THROW_INFO(pSwap->GetBuffer(0, __uuidof(ID3D11Resource),&pBackBuffer));

	//백버퍼에대한 핸들을 pBackBuffer에 얻었으면 해당 정보를 RenderTargetView를 만드는데 사용할 수 있다.
	GFX_THROW_INFO(pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &pTarget));
	//첫번째 매개변수는 RenderTargetView를 만들기위한 버퍼의 정보, 두번째는 만들면서 부가적인정보, 3번째는 만들어진 타겟을 가르킬 이중포인터

	// 깊이버퍼 만들기
	// 프레임버퍼는 스왑체인을 만들면서 같이 생기지만  깊이버퍼를 만들기 위해서는 직접 텍스쳐를 만들어야한다
	// 텍스쳐를 얻고 출력조립기에 바인딩해야한다.

	//깊이-스텐실 버퍼 만들기 (깊이버퍼는 스텐실버퍼와 같은 공간을 공유하며 각각의 하는일이 다르다)
	//둘다 마스크의 한종류이며 깊이버퍼는 오직 깊이에대해서만 마스크를 하지만 스텐실은 더많은 종류의 마스킹을 담당한다.
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = TRUE;	//깊이테스트를 활성화한다.
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	// D3D11_DEPTH_WRITE_MASK - 깊이테스트의 마스크 종류는 ZERO,ALL 두개뿐이며 단순히 끄고 켜는것 밖에 없다.
	// Zero 일경우 쓰지않고 ALL 일경우 깊이-스텐실 버퍼에 기록한다.
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	// D3D11_COMPARISON_FUNC - 마스크를 어떻게 비교할지 정하며 픽셀이 쓰일지 버려질지 정해진다.
	// ex) D3D11_COMPARISON_LESS -> 만약 z값이 작으면 즉 스크린과 가깝다면 픽셀이 선택되며 이전의 것을 가린다.

	//스텐실부분은 잠시 넘어가며 깊이에 대해서만 다뤄보자. 구조체를 완성했으면 깊이버퍼를 만들어보자
	wrl::ComPtr<ID3D11DepthStencilState> pDSState;	//인터페이스 포인터에 대한 주소를 받아오며
	GFX_THROW_INFO(pDevice->CreateDepthStencilState(&dsDesc, &pDSState));//해당 인터페이스를 구조체 정보로 채운다.

	//깊이-스텐실 인터페이스를 파이프라인(출력조립기)에 바인딩한다.
	pContext->OMSetDepthStencilState(pDSState.Get(), 1u);

	// 이제 깊이-스텐실 텍스쳐를 만들어야 한다.
	wrl::ComPtr<ID3D11Texture2D> pDepthStencil;
	D3D11_TEXTURE2D_DESC descDepth = {};
	descDepth.Width = 800u;		//가로 세로는 SwapChain 과 같아야한다.
	descDepth.Height = 600u;
	descDepth.MipLevels = 1u;	//밉맵에 대해서는 나중에 다룬다.
	descDepth.ArraySize = 1u;	//나중에 단일 텍스쳐 자원에 대해서 배열단위로 텍스쳐를 넣을수 있다. 나중에 다룬다
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;	//텍스쳐의 포맷으로 보통 R32G32B32 이런 포맷이지만 D32 로 알수있다. 이것은 깊이를위해 있는것이다
												//즉 32bit가 깊이값을위해 사용된다.
	descDepth.SampleDesc.Count = 1u;		//안티앨리어싱을 위한 값들 나중에 다룬다.
	descDepth.SampleDesc.Quality = 0u;
	
	descDepth.Usage = D3D11_USAGE_DEFAULT;			//용도플래그
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL; //바인드 플래그는 꼭 Depth_Stencil로 지정해야 해당 리소스를 깊이-스텐실로 사용한다

	//이제 텍스처 구조체를 다채웠으면 텍스처를 생성하자
	//1- 텍스쳐 정보를 담은 구조체의 포인터, 2- 텍스쳐를 채울 SubResource 하지만 깊이버퍼이므로 아무것도 채우지 않아도 된다
	//3- 텍스쳐pp
	GFX_THROW_INFO(pDevice->CreateTexture2D(&descDepth, nullptr, &pDepthStencil));

	//백버퍼에서 뷰를 만든것처럼 깊이-스텐실 뷰를 만들어서 출력조립기에 바인딩해야한다.
	CD3D11_DEPTH_STENCIL_VIEW_DESC descDsv = {};
	descDsv.Format = DXGI_FORMAT_D32_FLOAT;
	descDsv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDsv.Texture2D.MipSlice = 0u;
	GFX_THROW_INFO(pDevice->CreateDepthStencilView(pDepthStencil.Get(), &descDsv, &pDSV));

	//깊이-스텐실 뷰를 출력조립기에 바인딩한다.
	pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), pDSV.Get());

	// 그후 렌더타겟을 정했으면 이제 뷰포트를 정해야한다.
	D3D11_VIEWPORT vp;
	vp.Width = 800;
	vp.Height = 600;
	vp.MinDepth = 0;
	vp.MaxDepth = 1;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	//RS는 레스터라이즈 Rasterize Stage 의 줄임말 그리고 함수이름이 Viewports (복수형)임을 보아 다수의 뷰포트를 넘길 수 있다.
	pContext->RSSetViewports(1u, &vp);

}
void Graphics::EndFrame()
{
	HRESULT hr = 0;

	//화면 표현에는 SawpChain을 사용한다
	//첫번째 매개변수는  Sync interval 이다 이것은 MSDN에서는 잘 이해가 가진않는데..
	//일단 영상에서는 장치의 refresh rate 가 60hz 이지만 만약 고정적인 60hz를 표현하지 못할것같아
	// 30프레임으로 조정하고자할때 2u로 지정한다 아마 60hz /2  = 30hz 인것일까? 1u면 60hz이다
	// 즉 지정한 refreshRate/ syncinterval 인듯..
	// 두번째 인다는 기타 플래그이다.
#ifndef NDEBUG
	infoManager.Set();
#endif
	if(FAILED(hr = pSwap->Present(1u, 0u)))
	{
		//Present 함수는 HRESULT중 DXGI_ERROR_DEVICE_REMOVED를 반환하기도하는데
		//해당 상황을 대비하여 따로 조건문을 건다 해당 코드가 발생하면 디바이스에서 해당문제의 이류를 알아내는 함수를 호출할수 있으며
		//REMOVE의 원인으로는 보통 crash,혹은 gpu 오버클록킹이다.
		if (hr == DXGI_ERROR_DEVICE_REMOVED)
			throw GFX_DEVICE_REMOVED_EXCEPT(pDevice->GetDeviceRemovedReason());
		else
			throw GFX_EXCEPT(hr);
	}
}

void Graphics::ClearBuffer(float red, float green, float blue) noexcept
{
	const float color[] = { red,green,blue,1.0f };
	pContext->ClearRenderTargetView(pTarget.Get(), color);
	pContext->ClearDepthStencilView(pDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);
}

void Graphics::DrawIndexed(UINT count) noexcept(!IS_DEBUG)
{
	GFX_THROW_INFO_ONLY(pContext->DrawIndexed(count, 0u, 0u));
}

void Graphics::SetProjection(DirectX::FXMMATRIX proj) noexcept
{
	projection = proj;
}

DirectX::XMMATRIX Graphics::GetProjection() const noexcept
{
	return projection;
}



Graphics::HrException::HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs) noexcept
	:Exception(line,file), hr(hr)
{
	//모든 메시지를 하나의 스트링에 담는다
	for (const auto& m : infoMsgs)
	{
		info += m;
		info.push_back('\n');
	}
	//마지막 라인이있을경우 제거
	if (!info.empty())
		info.pop_back();
}

const char* Graphics::HrException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
		<< std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
		<< "[Error String] " << GetErrorString() << std::endl
		<< "[Description] " << GetErrorDescription() << std::endl;

	if (!info.empty())
		oss << "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;

		oss << GetOriginString();
	whatBuffer = oss.str();
		return whatBuffer.c_str();
}

const char* Graphics::HrException::GetType() const noexcept
{
	return "Bs Graphics Excetpion";
}

HRESULT Graphics::HrException::GetErrorCode() const noexcept
{
	return hr;
}

std::string Graphics::HrException::GetErrorString() const noexcept
{
	return DXGetErrorString(hr);
}

std::string Graphics::HrException::GetErrorDescription() const noexcept
{
	char buf[512];
	DXGetErrorDescription(hr, buf, sizeof(buf));
	return buf;
}
std::string Graphics::HrException::GetErrorInfo() const noexcept
{
	return info;
}
const char* Graphics::DeviceRemovedException::GetType() const noexcept
{
	return "Bs Grahpics Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)";
}

Graphics::InfoException::InfoException(int line, const char* file, std::vector<std::string> infoMsgs) noexcept
	:Exception(line,file)
{
	for (const auto& m : infoMsgs)
	{
		info += m;
		info.push_back('\n');
	}
	if (!info.empty())
		info.pop_back();
}

const char* Graphics::InfoException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Graphics::InfoException::GetType() const noexcept
{
	return "Bs Graphics Info Exception";
}

std::string Graphics::InfoException::GetErrorInfo() const noexcept
{
	return info;
}
