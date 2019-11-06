#include "Graphics.h"
#include "dxerr.h"
#include <sstream>
#include <d3dcompiler.h>
#include <cmath>
#include <DirectXMath.h>

namespace wrl = Microsoft::WRL;
//DirectxMath 네임스페이스
namespace dx = DirectX;


#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"D3DCompiler.lib")

//아래 매크로는 해당 표현식을 실행하고 나온 HRESULT의 값을 비교하여 예외를 던진다.
//hr은 로컬위치에 있다고 가정하므로 꼭 지역변수에 선언하고 사용한다.
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
}

 void Graphics::DrawTestTrangle(float angle,float x,float y)
{
	 //XM이 붙은 자료형이나 함수들은 SIMD에 최적화되어진 변수들이다.
	 //기존의 D3DXVECTOR 과 다르게 SIMD에 최적화되어 직접접근을 할수 없으며 인터페이스를통한 접근만이 가능하다
	 dx::XMVECTOR v = dx::XMVectorSet(3.0f, 3.0f, 0.0f, 0.0f);	
	 auto result = dx::XMVector4Dot(v, v);// XMVECTOR로 내적을 수행하는것으로 용도에따라 두원소만 내적하고싶으면 XMVector2Dot을 호출하면된다.
										  // 다만 XMVectorDot 함수는 SIMD최적화로 스칼라값이아닌 벡터를 산출하게된다 왜냐하면 SIMD에서 하나의 값을 산출하는것이 더 비효율적이라 4개의 같은 값을 가진 
										  // 벡터를 산출한다.
	 auto xx = dx::XMVectorGetX(result);	  // 이런식으로 각각의 원소에 접근하는함수를 호출해야한다.

	 auto result2 = dx::XMVector3Transform(v, dx::XMMatrixScaling(1.5, 0.0f, 0.0f));	//벡터와 행렬사이의 연산이다.
	HRESULT hr = 0;
	//버퍼 변수.
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

	//생성할 버퍼의 타입을 정한다.
	D3D11_BUFFER_DESC bd = {};
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0u;
	bd.MiscFlags = 0u;
	bd.ByteWidth = sizeof(vertices);
	bd.StructureByteStride = sizeof(Vertex);

	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = vertices;

	// CreateBuffer - 버텍스 버퍼, 인덱스 버퍼, 쉐이더 고정 버퍼등.. 버퍼들을 생성한다.
	// 1. D3D11_BUFFER_DESC 구조체를 가르키는 포인터 ( 버퍼를 설명하는 구조체)
	// 2. D3D11_SUBRESOURCE_DATA 구조체를 가르키는 포인터 - 초기 데이터를 설명하는 구조체. NULL이면 공간만 할당한다.
	// 3. 생성된 버퍼오브젝트의 ID3D11Buffer 인터페이스 포인터 주소.

	GFX_THROW_INFO(pDevice->CreateBuffer(&bd, &sd, &pVertexBuffer));
	// IASetVertexBuffers : 입력조립기에 버텍스 버퍼를 지정한다.
	// 1-  버텍스 배열중에 첫번재 슬롯의 인덱스 2- 배열안에 있는 버텍스 버퍼의 개수.
	// 3-  버텍스 버퍼 배열의 포인터(버텍스 버퍼들은  D3D11_BIND_VERTEX_BUFFER 플래그로 생성되야한다)
	const UINT stride = sizeof(Vertex);
	const UINT offset = 0u;
	pContext->IASetVertexBuffers(0u, 1u, pVertexBuffer.GetAddressOf(), &stride, &offset);

	// 인덱스 버퍼 생성
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
	//버퍼 디스크립터 생성
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

	//인덱스버퍼를 파이프라인에 바인드 한다.

	pContext->IASetIndexBuffer(pIndexBuffer.Get(),DXGI_FORMAT_R16_UINT,0u);

	//변환행렬을 위한 상수버퍼를 만든다
	struct ConstantBuffer
	{
		dx::XMMATRIX transform;		//4x4 float 행렬 SIMD 최적화가 이루어져있기 때문에 직접적으로 원소에 접근할 수 없다.
									//인터페이스를 통한 접근함수를 사용해야한다.
	};
	const ConstantBuffer cb =
	{
		{
				dx::XMMatrixTranspose(							//HLSL에서는 기본적으로 들어오는 행렬은 열위주로 여긴다 row_major 키워드로 행위주로 바꿀수는있지만 조금 느리게 만든다
				dx::XMMatrixRotationZ(angle) *
				dx::XMMatrixRotationX(angle) *					//그러므로 GPU상에서 해당 연산을 지우기위해 (최적화) 응용프로그램에서 해당 행렬을 전치하여 열위주로 바꾸면 된다. (CPU상에서의 전치는 빠른 연산이므로 GPU상에서보다 더 이득이 있다)
				dx::XMMatrixTranslation(x,y,4.0f) *				//Multiply 함수는 C스타일이지만 * 연산자를 오버로딩 했으므로 그냥 곱하기해도된다.
				dx::XMMatrixPerspectiveLH(1.0f,3.0f / 4.0f,0.5f,10.0f)
				)
		}
	};

	wrl::ComPtr<ID3D11Buffer> pConstanctBuffer;
	D3D11_BUFFER_DESC cbd;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;		//상수버퍼를 만들기위한 플래그
	cbd.Usage = D3D11_USAGE_DYNAMIC;			    //DYNAMIC 키워드로 버퍼를 만들게되면 CPU에서 프레임마다 업데이트하게된다.
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	//고로 CPU가 접근할수있게 플래그를 설정한다.
	cbd.MiscFlags = 0u;
	cbd.ByteWidth = sizeof(cb);
	cbd.StructureByteStride = 0u;
	D3D11_SUBRESOURCE_DATA csd = {};
	csd.pSysMem = &cb;
	GFX_THROW_INFO(pDevice->CreateBuffer(&cbd, &csd, &pConstanctBuffer));

	// 생성한 상수버퍼를 정점쉐이더에 바인딩한다. 
	pContext->VSSetConstantBuffers(0, 1u, pConstanctBuffer.GetAddressOf());

	struct ConstantBuffer2		//현재 색깔이 보간되어 무지개색으로 나오는것을 막고 solid 로 표현하기위해 상수버퍼를 정의한다.
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
	//상수버퍼2 생성
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

	//상수버퍼2를 픽셀쉐이더 바인딩한다.
	pContext->PSSetConstantBuffers(0u, 1u, pConstanceBuffer2.GetAddressOf());

	//픽셀 쉐이더를 만든다 
	wrl::ComPtr<ID3D11PixelShader> pPixelShader;
	//픽셀 쉐이더 포인터를 선언하고 바이트 코드로 이루어진 쉐이더 코드를 읽어야한다.
	//Blob = binary blob of data 이진 데이터 덩어리라느 뜻으로 아무 데이터나 될 수 있다.
	//즉 아래함수는 어떤게 저장되어있던지 읽어들인다. Blob 함수 매개변수또한 Com객체를 받으므로  해당 정보를 받기위해 선언을 해야한다
	wrl::ComPtr<ID3DBlob> pBlob;
	// D3DReadFileBolb() : 파일을 읽어들인다 1. 파일 경로 2. ID3DBlob 의 포인터 주소
	GFX_THROW_INFO(D3DReadFileToBlob(L"PixelShader.cso", &pBlob));
	GFX_THROW_INFO(pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader));

	//픽쉘세이더를 파이프라인에 바인드한다.
	//픽쉘세이더 다음 파이프라인은 Output Merger 즉 출력조립기 다음은 렌더타겟인데 즉 렌더타겟이 있어야
	//픽셀출력이 가능하다
	pContext->PSSetShader(pPixelShader.Get(), nullptr, 0u);

	//버텍스 쉐이더를 만든다 위 과정과 같다.
	wrl::ComPtr<ID3D11VertexShader> pVertexShader;

	GFX_THROW_INFO(D3DReadFileToBlob(L"VertexShader.cso", &pBlob));
	GFX_THROW_INFO(pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pVertexShader));
	//버텍스 쉐이더를 로드했으므로 이제 파이프라인에 정점 쉐이더를 바인드 해야한다.
	//VSSetShader 함수가 정점쉐이더를 바인딩하는 구문이며 첫번째 매개변수가 정점쉐이더 컴 객체의 주소를 전달한다.
	pContext->VSSetShader(pVertexShader.Get(), nullptr, 0);

	//버텍스 버퍼를 만들었으면 이제 정점정보를 쉐이더에 넘기기위한 레이아웃을 만들어야한다.
	//레이아웃 변수를 선언한후 해당 레이아웃을 설명할 구조체를 만들어야한다. 현재 Vertex는  하나의 요소(엘레멘트)를 가지고 있으므로
	//구조체에 하나만 정의되어 있다.
	wrl::ComPtr<ID3D11InputLayout> pInputLayout;
	// D3D11_INPUT_ELEMENT_DESC - 입력조립시에 들어갈 단일 요소들을 설명하는 구조체
	// 1-  시멘틱이름(쉐이더와 맞춰야한다), 2-  시멘틱 인덱스(각각의 시멘틱은 인덱스를 정할수있다) 즉.. 아래 시멘틱이름에서 Position0,Position1.. 이렇게 
	// 3-  DXGI_FORMAT 포맷 즉 어떠한 데이터가 요소에 들어가있는지 알려준다. 아래 정의한 정보는 2개의 float으로 이루어 졌음을 알린다., 4-  입력조립기의 번호를 넘긴다 (0-15사이),
	// 5-  각각의 요소 사이의 오프셋을 정한다, 6- 단일 입력슬롯을 위해 입력데이터클래스를 정의한다(2개가있는데 PRE_VERTEX,PER_INSTANCE이다 즉 인스턴스를 사용할지 안할지를 정해준다..7- 같은 인스턴스 데이터를 이용하여 그릴 인스턴스의 숫자. 
	const D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{"Position",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		//UNORM 키워드는 지금 R8G8B8A8 은 색깔의 rgba가 unsigned char로 이루어져 int 형으로 표현되어있다
		//UINT의 경우에는 쉐이더에 있는그대로 int형으로 보내지만 UNORM은 해당 0-255의 값을 0~1.0 사이의 값으로 노말라이즈한다.
	};
	// 입력레이아웃을 정했으면 이제 생성을 해야한다
	GFX_THROW_INFO(pDevice->CreateInputLayout(
		ied, (UINT)std::size(ied),
		pBlob->GetBufferPointer(),		//쉐이더의 바이트코드를 전달하게되는데 레이아웃구조체의 시멘틱과 쉐이더의 시멘틱이 일치하는지 확인하기 위해서다.(정점 쉐이더를 읽어야한다)
		pBlob->GetBufferSize(),
		&pInputLayout		//blob 읽기 함수와 마찬가지로 출력매개변수이므로 & 연산자를 사용한다.
	));
	//레이아웃을 생성했으면 파이프라인에 바인딩한다.
	pContext->IASetInputLayout(pInputLayout.Get());

	//렌더타겟을 파이프라인에 바인딩한다. OM = Output Merget
	//OMSetRenderTarget - 1개 이상의 렌더타겟과 깊이-스텐실 버퍼를 바인딩한다. (OM에서)
	// 1- 바인딩할 렌더타겟의 개수 (1이상이면 2번째 매개변수즉 렌더타겟의 배열개수와 같아야한다)
	// 2- 바인딩할 렌더타겟 배열의 포인터 NULL이면 바인딩된 렌더타겟이 없다
	// 3- 바인딩할 깊이-스텐실 버퍼의 포인터.
	// 이미 Graphics 클래스 생성자에서 백버퍼에대한 렌더타겟을 생성했으므로 해당 렌더타겟을 사용한다.
	// 2번째 매개변수로 바인딩할 렌더타겟의 배열포인터를  전달하는데 즉 여기서는 백버퍼의 포인터 주소를 넘기게된다
	// 우리는 Comptr<ID3D11RenderTargetView> 형식으로 변수를 선언했으므로 &를넘기면 해당 백버퍼의 주소를 담고있는 이중포인터의
	// 주소가 넘어갈것이라고 생각하는데. Comptr에서의 & 연산은 해당 변수 즉 ComPtr<interface> 인터페이스와의 연결을 끊고(Release)
	// 그후 포인터가 null값을 가르킨상태의 포인터 주소를 넘기게된다 이말인 즉슨 아래와같은 구문은 null을 넘기는것이라 오류를 일으키게되는데.
	//			Context->OMSetRenderTargets(1u, &pTarget, nullptr);
	// 하지만 위에 쉐이더파일을 읽는 D3DReadFileToBlob 함수에서는 &로 넘긴다 이것은 각 함수의 매개변수를 잘 살펴보아야한다.
	// Blob 함수의 경우 2번째 매개변수는 읽은 파일에대해 다시 ID3DBlob 인터페이스를 넘기기에 &연산자를 사용하여 기존의 인터페이스와 연결을 끊고 받아야
	// 메모리 릭이 일어나지 않는다. 그러므로 각 함수의 기능을 잘 확인해야 한다.

	pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), nullptr);	//이런방식으로 주소를 얻어야한다 &으로 새로운 값을 받지 않는이상 & 연산자는 해당 인터페이스와 연결을 끊는다
	
	// primitive topology 를 정하여 어떻게 정점이 그려질지를 정해야한다
	pContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST

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

	//모든 설정이 완료되었으면 그린다.
	GFX_THROW_INFO_ONLY(pContext->DrawIndexed((UINT) std::size(indices), 0u,0u));  
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
