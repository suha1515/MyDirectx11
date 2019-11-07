#include "Graphics.h"
#include "dxerr.h"
#include <sstream>
#include <d3dcompiler.h>
#include <cmath>
#include <DirectXMath.h>
#include "GraphicsThrowMacros.h"

namespace wrl = Microsoft::WRL;
//DirectxMath ���ӽ����̽�
namespace dx = DirectX;


#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"D3DCompiler.lib")


Graphics::Graphics(HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC sd = {};
	//---���� ����,���� ����,�ֱ�� �������� ����ü
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
	//���� ������ Dxerr �� ��ȯ�ϴ� HRESULT�� Winapi�� �����ʾ� Ŭ������ ���� �������.
	//�׸��Ͽ� ������ �������� ����ó�����ϴµ� �� �÷��׸� �����ϸ� ���â�� DXGI WARNING ���� ������ ��� ���ش�.
	//������ ����ü ������ ä�ﶧ ���� �ڵ��� �����ϸ� �� ��ġ�ʱ�ȭ���� ������ �Ͼ �츮�� ����ó���� �� �Լ������� ����Ű�Եȴ�.
	//���� DXGI ERROR �޽����� �߻��Ͽ� ���â������ Ȯ���Ҽ� �ִ�.
	//�׷��ٸ�.. �̷��� ������ ���ܸ޽����� �������� ������? DxgiInfoManager ��� Ȯ��
#endif

	HRESULT hr = 0;
	//����̽��� ����,�ĸ� ����,����ü��,������ ���ؽ�Ʈ�� �����Ѵ�.
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

	//����ü�ο� �ִ� �ؽ��� ���긮�ҽ�(�����)�� ������ ��´�
	//����� �������� ID3D11Resource �������̽� �����͸� ���� ���������� ���� ����Ʈ �����͸� ����Ѵ�.
	wrl::ComPtr<ID3D11Resource> pBackBuffer;
	//����Ʈ �����͸� ���������� ���� CreatRenderTargetView �������� ���ܸ�����
	//�ѹ��� ����ϰԵ� pBackBuffer�� Release�� ȣ������������ pBackBuffer�� ��۸� �����Ͱ��ȴ�
	//�׷��⿡ com��ü�� ����Ʈ ������ ������ ������ ComPtr�� ����Ѵ�.

	// GetBuffer
	// IUnkown com��ü���� Queryinterface�� ����ϴ� ����Ѱ��� �´�.
	// ù��° �Ű������� ���ۿ����� �ε����� 0�� �ε����� ������̸� 2��° ���ڴ� UUID�� �ش� ID3D11Resource �������̽��� UUID�̴�
	// 3���� �Ű������� �ش� ���ҽ��������̽��� ������ ���� �����������̴�.
	GFX_THROW_INFO(pSwap->GetBuffer(0, __uuidof(ID3D11Resource),&pBackBuffer));

	//����ۿ����� �ڵ��� pBackBuffer�� ������� �ش� ������ RenderTargetView�� ����µ� ����� �� �ִ�.
	GFX_THROW_INFO(pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &pTarget));
	//ù��° �Ű������� RenderTargetView�� ��������� ������ ����, �ι�°�� ����鼭 �ΰ���������, 3��°�� ������� Ÿ���� ����ų ����������

	// ���̹��� �����
	// �����ӹ��۴� ����ü���� ����鼭 ���� ��������  ���̹��۸� ����� ���ؼ��� ���� �ؽ��ĸ� �������Ѵ�
	// �ؽ��ĸ� ��� ��������⿡ ���ε��ؾ��Ѵ�.

	//����-���ٽ� ���� ����� (���̹��۴� ���ٽǹ��ۿ� ���� ������ �����ϸ� ������ �ϴ����� �ٸ���)
	//�Ѵ� ����ũ�� �������̸� ���̹��۴� ���� ���̿����ؼ��� ����ũ�� ������ ���ٽ��� ������ ������ ����ŷ�� ����Ѵ�.
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = TRUE;	//�����׽�Ʈ�� Ȱ��ȭ�Ѵ�.
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	// D3D11_DEPTH_WRITE_MASK - �����׽�Ʈ�� ����ũ ������ ZERO,ALL �ΰ����̸� �ܼ��� ���� �Ѵ°� �ۿ� ����.
	// Zero �ϰ�� �����ʰ� ALL �ϰ�� ����-���ٽ� ���ۿ� ����Ѵ�.
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	// D3D11_COMPARISON_FUNC - ����ũ�� ��� ������ ���ϸ� �ȼ��� ������ �������� ��������.
	// ex) D3D11_COMPARISON_LESS -> ���� z���� ������ �� ��ũ���� �����ٸ� �ȼ��� ���õǸ� ������ ���� ������.

	//���ٽǺκ��� ��� �Ѿ�� ���̿� ���ؼ��� �ٷﺸ��. ����ü�� �ϼ������� ���̹��۸� ������
	wrl::ComPtr<ID3D11DepthStencilState> pDSState;	//�������̽� �����Ϳ� ���� �ּҸ� �޾ƿ���
	GFX_THROW_INFO(pDevice->CreateDepthStencilState(&dsDesc, &pDSState));//�ش� �������̽��� ����ü ������ ä���.

	//����-���ٽ� �������̽��� ����������(���������)�� ���ε��Ѵ�.
	pContext->OMSetDepthStencilState(pDSState.Get(), 1u);

	// ���� ����-���ٽ� �ؽ��ĸ� ������ �Ѵ�.
	wrl::ComPtr<ID3D11Texture2D> pDepthStencil;
	D3D11_TEXTURE2D_DESC descDepth = {};
	descDepth.Width = 800u;		//���� ���δ� SwapChain �� ���ƾ��Ѵ�.
	descDepth.Height = 600u;
	descDepth.MipLevels = 1u;	//�Ӹʿ� ���ؼ��� ���߿� �ٷ��.
	descDepth.ArraySize = 1u;	//���߿� ���� �ؽ��� �ڿ��� ���ؼ� �迭������ �ؽ��ĸ� ������ �ִ�. ���߿� �ٷ��
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;	//�ؽ����� �������� ���� R32G32B32 �̷� ���������� D32 �� �˼��ִ�. �̰��� ���̸����� �ִ°��̴�
												//�� 32bit�� ���̰������� ���ȴ�.
	descDepth.SampleDesc.Count = 1u;		//��Ƽ�ٸ������ ���� ���� ���߿� �ٷ��.
	descDepth.SampleDesc.Quality = 0u;
	
	descDepth.Usage = D3D11_USAGE_DEFAULT;			//�뵵�÷���
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL; //���ε� �÷��״� �� Depth_Stencil�� �����ؾ� �ش� ���ҽ��� ����-���ٽǷ� ����Ѵ�

	//���� �ؽ�ó ����ü�� ��ä������ �ؽ�ó�� ��������
	//1- �ؽ��� ������ ���� ����ü�� ������, 2- �ؽ��ĸ� ä�� SubResource ������ ���̹����̹Ƿ� �ƹ��͵� ä���� �ʾƵ� �ȴ�
	//3- �ؽ���pp
	GFX_THROW_INFO(pDevice->CreateTexture2D(&descDepth, nullptr, &pDepthStencil));

	//����ۿ��� �並 �����ó�� ����-���ٽ� �並 ���� ��������⿡ ���ε��ؾ��Ѵ�.
	CD3D11_DEPTH_STENCIL_VIEW_DESC descDsv = {};
	descDsv.Format = DXGI_FORMAT_D32_FLOAT;
	descDsv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDsv.Texture2D.MipSlice = 0u;
	GFX_THROW_INFO(pDevice->CreateDepthStencilView(pDepthStencil.Get(), &descDsv, &pDSV));

	//����-���ٽ� �並 ��������⿡ ���ε��Ѵ�.
	pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), pDSV.Get());

	// ���� ����Ÿ���� �������� ���� ����Ʈ�� ���ؾ��Ѵ�.
	D3D11_VIEWPORT vp;
	vp.Width = 800;
	vp.Height = 600;
	vp.MinDepth = 0;
	vp.MaxDepth = 1;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	//RS�� �����Ͷ����� Rasterize Stage �� ���Ӹ� �׸��� �Լ��̸��� Viewports (������)���� ���� �ټ��� ����Ʈ�� �ѱ� �� �ִ�.
	pContext->RSSetViewports(1u, &vp);

}
void Graphics::EndFrame()
{
	HRESULT hr = 0;

	//ȭ�� ǥ������ SawpChain�� ����Ѵ�
	//ù��° �Ű�������  Sync interval �̴� �̰��� MSDN������ �� ���ذ� �����ʴµ�..
	//�ϴ� ���󿡼��� ��ġ�� refresh rate �� 60hz ������ ���� �������� 60hz�� ǥ������ ���ҰͰ���
	// 30���������� �����ϰ����Ҷ� 2u�� �����Ѵ� �Ƹ� 60hz /2  = 30hz �ΰ��ϱ�? 1u�� 60hz�̴�
	// �� ������ refreshRate/ syncinterval �ε�..
	// �ι�° �δٴ� ��Ÿ �÷����̴�.
#ifndef NDEBUG
	infoManager.Set();
#endif
	if(FAILED(hr = pSwap->Present(1u, 0u)))
	{
		//Present �Լ��� HRESULT�� DXGI_ERROR_DEVICE_REMOVED�� ��ȯ�ϱ⵵�ϴµ�
		//�ش� ��Ȳ�� ����Ͽ� ���� ���ǹ��� �Ǵ� �ش� �ڵ尡 �߻��ϸ� ����̽����� �ش繮���� �̷��� �˾Ƴ��� �Լ��� ȣ���Ҽ� ������
		//REMOVE�� �������δ� ���� crash,Ȥ�� gpu ����Ŭ��ŷ�̴�.
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
	//��� �޽����� �ϳ��� ��Ʈ���� ��´�
	for (const auto& m : infoMsgs)
	{
		info += m;
		info.push_back('\n');
	}
	//������ ������������� ����
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
