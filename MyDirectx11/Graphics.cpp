#include "Graphics.h"
#include "dxerr.h"
#include <sstream>
#include <d3dcompiler.h>
#include <cmath>
#include <DirectXMath.h>

namespace wrl = Microsoft::WRL;
//DirectxMath ���ӽ����̽�
namespace dx = DirectX;


#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"D3DCompiler.lib")

//�Ʒ� ��ũ�δ� �ش� ǥ������ �����ϰ� ���� HRESULT�� ���� ���Ͽ� ���ܸ� ������.
//hr�� ������ġ�� �ִٰ� �����ϹǷ� �� ���������� �����ϰ� ����Ѵ�.
#define GFX_EXCEPT_NOINFO(hr) Graphics::HrException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_NOINFO(hrcall) if( FAILED( hr = (hrcall) ) ) throw Graphics::HrException( __LINE__,__FILE__,hr )

#ifndef NDEBUG
#define GFX_EXCEPT(hr) Graphics::HrException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#define GFX_THROW_INFO(hrcall) infoManager.Set(); if( FAILED( hr = (hrcall) ) ) throw GFX_EXCEPT(hr)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) Graphics::DeviceRemovedException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#define GFX_THROW_INFO_ONLY(call) infoManager.Set(); (call); {auto v = infoManager.GetMessages(); if(!v.empty()){throw Graphics::InfoException(__LINE__,__FILE__,v);}}
#else
#define GFX_EXCEPT(hr) Graphics::HrException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_INFO(hrcall) GFX_THROW_NOINFO(hrcall)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) Graphics::DeviceRemovedException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_INFO_ONLY(call) (call)
#endif

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
}

 void Graphics::DrawTestTrangle(float angle,float x,float y)
{
	 //XM�� ���� �ڷ����̳� �Լ����� SIMD�� ����ȭ�Ǿ��� �������̴�.
	 //������ D3DXVECTOR �� �ٸ��� SIMD�� ����ȭ�Ǿ� ���������� �Ҽ� ������ �������̽������� ���ٸ��� �����ϴ�
	 dx::XMVECTOR v = dx::XMVectorSet(3.0f, 3.0f, 0.0f, 0.0f);	
	 auto result = dx::XMVector4Dot(v, v);// XMVECTOR�� ������ �����ϴ°����� �뵵������ �ο��Ҹ� �����ϰ������ XMVector2Dot�� ȣ���ϸ�ȴ�.
										  // �ٸ� XMVectorDot �Լ��� SIMD����ȭ�� ��Į���̾ƴ� ���͸� �����ϰԵȴ� �ֳ��ϸ� SIMD���� �ϳ��� ���� �����ϴ°��� �� ��ȿ�����̶� 4���� ���� ���� ���� 
										  // ���͸� �����Ѵ�.
	 auto xx = dx::XMVectorGetX(result);	  // �̷������� ������ ���ҿ� �����ϴ��Լ��� ȣ���ؾ��Ѵ�.

	 auto result2 = dx::XMVector3Transform(v, dx::XMMatrixScaling(1.5, 0.0f, 0.0f));	//���Ϳ� ��Ļ����� �����̴�.
	HRESULT hr = 0;
	//���� ����.
	wrl::ComPtr<ID3D11Buffer> pVertexBuffer;
	struct Vertex
	{
		struct
		{
			float x;
			float y;
			float z;
		}pos;
	};

	Vertex vertices[] =
	{
		{-1.0f,-1.0f,-1.0f},
		{ 1.0f,-1.0f,-1.0f},
		{-1.0f, 1.0f,-1.0f},
		{ 1.0f, 1.0f,-1.0f},
		{-1.0f,-1.0f, 1.0f},
		{ 1.0f,-1.0f, 1.0f},
		{-1.0f, 1.0f, 1.0f},
		{ 1.0f, 1.0f, 1.0f},
	};

	//������ ������ Ÿ���� ���Ѵ�.
	D3D11_BUFFER_DESC bd = {};
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0u;
	bd.MiscFlags = 0u;
	bd.ByteWidth = sizeof(vertices);
	bd.StructureByteStride = sizeof(Vertex);

	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = vertices;

	// CreateBuffer - ���ؽ� ����, �ε��� ����, ���̴� ���� ���۵�.. ���۵��� �����Ѵ�.
	// 1. D3D11_BUFFER_DESC ����ü�� ����Ű�� ������ ( ���۸� �����ϴ� ����ü)
	// 2. D3D11_SUBRESOURCE_DATA ����ü�� ����Ű�� ������ - �ʱ� �����͸� �����ϴ� ����ü. NULL�̸� ������ �Ҵ��Ѵ�.
	// 3. ������ ���ۿ�����Ʈ�� ID3D11Buffer �������̽� ������ �ּ�.

	GFX_THROW_INFO(pDevice->CreateBuffer(&bd, &sd, &pVertexBuffer));
	// IASetVertexBuffers : �Է������⿡ ���ؽ� ���۸� �����Ѵ�.
	// 1-  ���ؽ� �迭�߿� ù���� ������ �ε��� 2- �迭�ȿ� �ִ� ���ؽ� ������ ����.
	// 3-  ���ؽ� ���� �迭�� ������(���ؽ� ���۵���  D3D11_BIND_VERTEX_BUFFER �÷��׷� �����Ǿ��Ѵ�)
	const UINT stride = sizeof(Vertex);
	const UINT offset = 0u;
	pContext->IASetVertexBuffers(0u, 1u, pVertexBuffer.GetAddressOf(), &stride, &offset);

	// �ε��� ���� ����
	const unsigned short indices[] =
	{
		0,2,1,	2,3,1,
		1,3,5,	3,7,5,
		2,6,3,	3,6,7,
		4,5,7,	4,7,6,
		0,4,2,	2,4,6,
		0,1,4,	1,5,4,
	};
	wrl::ComPtr<ID3D11Buffer> pIndexBuffer;
	//���� ��ũ���� ����
	D3D11_BUFFER_DESC ibd = {};
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.CPUAccessFlags = 0u;
	ibd.MiscFlags = 0u;
	ibd.ByteWidth = sizeof(indices);
	ibd.StructureByteStride = sizeof(unsigned short);
	D3D11_SUBRESOURCE_DATA isd = {};
	isd.pSysMem = indices;
	GFX_THROW_INFO(pDevice->CreateBuffer(&ibd, &isd, &pIndexBuffer));

	//�ε������۸� ���������ο� ���ε� �Ѵ�.

	pContext->IASetIndexBuffer(pIndexBuffer.Get(),DXGI_FORMAT_R16_UINT,0u);

	//��ȯ����� ���� ������۸� �����
	struct ConstantBuffer
	{
		dx::XMMATRIX transform;		//4x4 float ��� SIMD ����ȭ�� �̷�����ֱ� ������ ���������� ���ҿ� ������ �� ����.
									//�������̽��� ���� �����Լ��� ����ؾ��Ѵ�.
	};
	const ConstantBuffer cb =
	{
		{
				dx::XMMatrixTranspose(							//HLSL������ �⺻������ ������ ����� �����ַ� ����� row_major Ű����� �����ַ� �ٲܼ��������� ���� ������ �����
				dx::XMMatrixRotationZ(angle) *
				dx::XMMatrixRotationX(angle) *					//�׷��Ƿ� GPU�󿡼� �ش� ������ ��������� (����ȭ) �������α׷����� �ش� ����� ��ġ�Ͽ� �����ַ� �ٲٸ� �ȴ�. (CPU�󿡼��� ��ġ�� ���� �����̹Ƿ� GPU�󿡼����� �� �̵��� �ִ�)
				dx::XMMatrixTranslation(x,y,4.0f) *				//Multiply �Լ��� C��Ÿ�������� * �����ڸ� �����ε� �����Ƿ� �׳� ���ϱ��ص��ȴ�.
				dx::XMMatrixPerspectiveLH(1.0f,3.0f / 4.0f,0.5f,10.0f)
				)
		}
	};

	wrl::ComPtr<ID3D11Buffer> pConstanctBuffer;
	D3D11_BUFFER_DESC cbd;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;		//������۸� ��������� �÷���
	cbd.Usage = D3D11_USAGE_DYNAMIC;			    //DYNAMIC Ű����� ���۸� ����ԵǸ� CPU���� �����Ӹ��� ������Ʈ�ϰԵȴ�.
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	//��� CPU�� �����Ҽ��ְ� �÷��׸� �����Ѵ�.
	cbd.MiscFlags = 0u;
	cbd.ByteWidth = sizeof(cb);
	cbd.StructureByteStride = 0u;
	D3D11_SUBRESOURCE_DATA csd = {};
	csd.pSysMem = &cb;
	GFX_THROW_INFO(pDevice->CreateBuffer(&cbd, &csd, &pConstanctBuffer));

	// ������ ������۸� �������̴��� ���ε��Ѵ�. 
	pContext->VSSetConstantBuffers(0, 1u, pConstanctBuffer.GetAddressOf());

	struct ConstantBuffer2		//���� ������ �����Ǿ� ������������ �����°��� ���� solid �� ǥ���ϱ����� ������۸� �����Ѵ�.
	{
		struct
		{
			float r;
			float g;
			float b;
			float a;
		}face_colors[6];
	};

	const ConstantBuffer2 cb2 =
	{
		{
			{1.0f,0.0f,1.0f},
			{1.0f,0.0f,0.0f},
			{0.0f,1.0f,0.0f},
			{0.0f,0.0f,1.0f},
			{1.0f,1.0f,0.0f},
			{0.0f,1.0f,1.0f},
		}
	};
	//�������2 ����
	wrl::ComPtr<ID3D11Buffer> pConstanceBuffer2;
	D3D11_BUFFER_DESC cbd2;
	cbd2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd2.Usage = D3D11_USAGE_DEFAULT;
	cbd2.CPUAccessFlags = 0u;
	cbd2.MiscFlags = 0u;
	cbd2.ByteWidth = sizeof(cb2);
	cbd2.StructureByteStride = 0u;
	D3D11_SUBRESOURCE_DATA csd2 = {};
	csd2.pSysMem = &cb2;
	GFX_THROW_INFO(pDevice->CreateBuffer(&cbd2, &csd2, &pConstanceBuffer2));

	//�������2�� �ȼ����̴� ���ε��Ѵ�.
	pContext->PSSetConstantBuffers(0u, 1u, pConstanceBuffer2.GetAddressOf());

	//�ȼ� ���̴��� ����� 
	wrl::ComPtr<ID3D11PixelShader> pPixelShader;
	//�ȼ� ���̴� �����͸� �����ϰ� ����Ʈ �ڵ�� �̷���� ���̴� �ڵ带 �о���Ѵ�.
	//Blob = binary blob of data ���� ������ ������ ������ �ƹ� �����ͳ� �� �� �ִ�.
	//�� �Ʒ��Լ��� ��� ����Ǿ��ִ��� �о���δ�. Blob �Լ� �Ű��������� Com��ü�� �����Ƿ�  �ش� ������ �ޱ����� ������ �ؾ��Ѵ�
	wrl::ComPtr<ID3DBlob> pBlob;
	// D3DReadFileBolb() : ������ �о���δ� 1. ���� ��� 2. ID3DBlob �� ������ �ּ�
	GFX_THROW_INFO(D3DReadFileToBlob(L"PixelShader.cso", &pBlob));
	GFX_THROW_INFO(pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader));

	//�Ƚ����̴��� ���������ο� ���ε��Ѵ�.
	//�Ƚ����̴� ���� ������������ Output Merger �� ��������� ������ ����Ÿ���ε� �� ����Ÿ���� �־��
	//�ȼ������ �����ϴ�
	pContext->PSSetShader(pPixelShader.Get(), nullptr, 0u);

	//���ؽ� ���̴��� ����� �� ������ ����.
	wrl::ComPtr<ID3D11VertexShader> pVertexShader;

	GFX_THROW_INFO(D3DReadFileToBlob(L"VertexShader.cso", &pBlob));
	GFX_THROW_INFO(pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pVertexShader));
	//���ؽ� ���̴��� �ε������Ƿ� ���� ���������ο� ���� ���̴��� ���ε� �ؾ��Ѵ�.
	//VSSetShader �Լ��� �������̴��� ���ε��ϴ� �����̸� ù��° �Ű������� �������̴� �� ��ü�� �ּҸ� �����Ѵ�.
	pContext->VSSetShader(pVertexShader.Get(), nullptr, 0);

	//���ؽ� ���۸� ��������� ���� ���������� ���̴��� �ѱ������ ���̾ƿ��� �������Ѵ�.
	//���̾ƿ� ������ �������� �ش� ���̾ƿ��� ������ ����ü�� �������Ѵ�. ���� Vertex��  �ϳ��� ���(������Ʈ)�� ������ �����Ƿ�
	//����ü�� �ϳ��� ���ǵǾ� �ִ�.
	wrl::ComPtr<ID3D11InputLayout> pInputLayout;
	// D3D11_INPUT_ELEMENT_DESC - �Է������ÿ� �� ���� ��ҵ��� �����ϴ� ����ü
	// 1-  �ø�ƽ�̸�(���̴��� ������Ѵ�), 2-  �ø�ƽ �ε���(������ �ø�ƽ�� �ε����� ���Ҽ��ִ�) ��.. �Ʒ� �ø�ƽ�̸����� Position0,Position1.. �̷��� 
	// 3-  DXGI_FORMAT ���� �� ��� �����Ͱ� ��ҿ� ���ִ��� �˷��ش�. �Ʒ� ������ ������ 2���� float���� �̷�� ������ �˸���., 4-  �Է��������� ��ȣ�� �ѱ�� (0-15����),
	// 5-  ������ ��� ������ �������� ���Ѵ�, 6- ���� �Է½����� ���� �Էµ�����Ŭ������ �����Ѵ�(2�����ִµ� PRE_VERTEX,PER_INSTANCE�̴� �� �ν��Ͻ��� ������� �������� �����ش�..7- ���� �ν��Ͻ� �����͸� �̿��Ͽ� �׸� �ν��Ͻ��� ����. 
	const D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{"Position",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		//UNORM Ű����� ���� R8G8B8A8 �� ������ rgba�� unsigned char�� �̷���� int ������ ǥ���Ǿ��ִ�
		//UINT�� ��쿡�� ���̴��� �ִ±״�� int������ �������� UNORM�� �ش� 0-255�� ���� 0~1.0 ������ ������ �븻�������Ѵ�.
	};
	// �Է·��̾ƿ��� �������� ���� ������ �ؾ��Ѵ�
	GFX_THROW_INFO(pDevice->CreateInputLayout(
		ied, (UINT)std::size(ied),
		pBlob->GetBufferPointer(),		//���̴��� ����Ʈ�ڵ带 �����ϰԵǴµ� ���̾ƿ�����ü�� �ø�ƽ�� ���̴��� �ø�ƽ�� ��ġ�ϴ��� Ȯ���ϱ� ���ؼ���.(���� ���̴��� �о���Ѵ�)
		pBlob->GetBufferSize(),
		&pInputLayout		//blob �б� �Լ��� ���������� ��¸Ű������̹Ƿ� & �����ڸ� ����Ѵ�.
	));
	//���̾ƿ��� ���������� ���������ο� ���ε��Ѵ�.
	pContext->IASetInputLayout(pInputLayout.Get());

	//����Ÿ���� ���������ο� ���ε��Ѵ�. OM = Output Merget
	//OMSetRenderTarget - 1�� �̻��� ����Ÿ�ٰ� ����-���ٽ� ���۸� ���ε��Ѵ�. (OM����)
	// 1- ���ε��� ����Ÿ���� ���� (1�̻��̸� 2��° �Ű������� ����Ÿ���� �迭������ ���ƾ��Ѵ�)
	// 2- ���ε��� ����Ÿ�� �迭�� ������ NULL�̸� ���ε��� ����Ÿ���� ����
	// 3- ���ε��� ����-���ٽ� ������ ������.
	// �̹� Graphics Ŭ���� �����ڿ��� ����ۿ����� ����Ÿ���� ���������Ƿ� �ش� ����Ÿ���� ����Ѵ�.
	// 2��° �Ű������� ���ε��� ����Ÿ���� �迭�����͸�  �����ϴµ� �� ���⼭�� ������� ������ �ּҸ� �ѱ�Եȴ�
	// �츮�� Comptr<ID3D11RenderTargetView> �������� ������ ���������Ƿ� &���ѱ�� �ش� ������� �ּҸ� ����ִ� ������������
	// �ּҰ� �Ѿ���̶�� �����ϴµ�. Comptr������ & ������ �ش� ���� �� ComPtr<interface> �������̽����� ������ ����(Release)
	// ���� �����Ͱ� null���� ����Ų������ ������ �ּҸ� �ѱ�Եȴ� �̸��� �ｼ �Ʒ��Ͱ��� ������ null�� �ѱ�°��̶� ������ ����Ű�ԵǴµ�.
	//			Context->OMSetRenderTargets(1u, &pTarget, nullptr);
	// ������ ���� ���̴������� �д� D3DReadFileToBlob �Լ������� &�� �ѱ�� �̰��� �� �Լ��� �Ű������� �� ���캸�ƾ��Ѵ�.
	// Blob �Լ��� ��� 2��° �Ű������� ���� ���Ͽ����� �ٽ� ID3DBlob �������̽��� �ѱ�⿡ &�����ڸ� ����Ͽ� ������ �������̽��� ������ ���� �޾ƾ�
	// �޸� ���� �Ͼ�� �ʴ´�. �׷��Ƿ� �� �Լ��� ����� �� Ȯ���ؾ� �Ѵ�.

	pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), nullptr);	//�̷�������� �ּҸ� �����Ѵ� &���� ���ο� ���� ���� �ʴ��̻� & �����ڴ� �ش� �������̽��� ������ ���´�
	
	// primitive topology �� ���Ͽ� ��� ������ �׷������� ���ؾ��Ѵ�
	pContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST

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

	//��� ������ �Ϸ�Ǿ����� �׸���.
	GFX_THROW_INFO_ONLY(pContext->DrawIndexed((UINT) std::size(indices), 0u,0u));  
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
