#include "Box.h"
#include "BindableBase.h"
#include "GraphicsThrowMacros.h"
#include "Sphere.h"

Box::Box(Graphics& gfx, std::mt19937& rng, std::uniform_real_distribution<float>& adist, std::uniform_real_distribution<float>& ddist, std::uniform_real_distribution<float>& odist, std::uniform_real_distribution<float>& rdist)
	:r(rdist(rng)),droll(ddist(rng)),dpitch(ddist(rng)),dyaw(ddist(rng)),dphi(odist(rng)),dtheta(odist(rng)),dchi(odist(rng)),
	chi(adist(rng)),theta(adist(rng)),phi(adist(rng))
{
	namespace dx = DirectX;
	if (!IsStaticInitialized())
	{
		struct Vertex
		{
			struct
			{
				dx::XMFLOAT3 pos;			//XMFLOAT3 FLOAT3 형식은 연속된공간에서의 자료형이아니다.
			}pos;
		};
		auto model = Sphere::Make<Vertex>();
		model.Transform(dx::XMMatrixScaling(1.0f, 1.0f, 1.2f));
		AddStaticBind(std::make_unique<VertexBuffer>(gfx, model.vertices));				//버텍스 버퍼 생성

		auto pvs = std::make_unique<VertexShader>(gfx, L"VertexShader.cso");//버텍스 쉐이더 생성	
		auto pvsbc = pvs->GetBytecode();
		AddStaticBind(std::move(pvs));

		AddStaticBind(std::make_unique < PixelShader>(gfx, L"PixelShader.cso"));	//픽셀쉐이더 생성

		AddStaticIndexBuffer(std::make_unique < IndexBuffer>(gfx, model.indices));	//인덱스버퍼 생성

		struct ConstantBuffer2
		{
			struct
			{
				float r;
				float g;
				float b;
				float a;
			} face_colors[6];
		};
		const ConstantBuffer2 cb2 =	//상수버퍼 선언
		{
			{
				{ 1.0f,0.0f,1.0f },
				{ 1.0f,0.0f,0.0f },
				{ 0.0f,1.0f,0.0f },
				{ 0.0f,0.0f,1.0f },
				{ 1.0f,1.0f,0.0f },
				{ 0.0f,1.0f,1.0f },
			}
		};

		AddStaticBind(std::make_unique < PixelConstantBuffer<ConstantBuffer2>>(gfx, cb2));		//픽셀상수버퍼 지정

		const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =									//입력 레이아웃 구조체
		{
			{"Position",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		};

		AddStaticBind(std::make_unique<InputLayout>(gfx, ied, pvsbc));							//레이아웃 바인드

		AddStaticBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));	//그리는방법 지정

	}
	else
	{
		SetIndexFromStatic();
	}
	AddBind(std::make_unique<TransformCbuf>(gfx, *this));
}

void Box::Update(float dt) noexcept
{
	roll += droll * dt;
	pitch += dpitch * dt;
	yaw += dyaw * dt;
	theta += dtheta * dt;
	phi += dphi * dt;
	chi += dchi * dt;
}

DirectX::XMMATRIX Box::GetTransformXM() const noexcept
{
	return DirectX::XMMatrixRotationRollPitchYaw(pitch,yaw,roll)*		//박스기준으로회전
		DirectX::XMMatrixTranslation(r,0.0f,0.0f)*						//박스에서 반지름만큼이동
		DirectX::XMMatrixRotationRollPitchYaw(theta,phi,chi)*			//이동한곳기준 박스 공전
		DirectX::XMMatrixTranslation(0.0f,0.0f,20.f);					//카메라에서떨어지기
}
