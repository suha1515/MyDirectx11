#include "Sheet.h"
#include "BindableCommon.h"
#include "GraphicsThrowMacros.h"
#include "Plane.h"
#include "Surface.h"
#include "Texture.h"
#include "Sampler.h"

Sheet::Sheet(Graphics& gfx,
	std::mt19937& rng,
	std::uniform_real_distribution<float>& adist,
	std::uniform_real_distribution<float>& ddist,
	std::uniform_real_distribution<float>& odist,
	std::uniform_real_distribution<float>& rdist)
	:
	r(rdist(rng)),
	droll(ddist(rng)),
	dpitch(ddist(rng)),
	dyaw(ddist(rng)),
	dphi(odist(rng)),
	dtheta(odist(rng)),
	dchi(odist(rng)),
	chi(adist(rng)),
	theta(adist(rng)),
	phi(adist(rng))
{
	using namespace Bind;
	namespace dx = DirectX;
		struct Vertex
		{
			dx::XMFLOAT3 pos;
			struct
			{
				float u;			// 텍스처 구조체
				float v;
			} tex;
		};
		auto model = Plane::Make();
		model.vertices[0].Attr<Dvtx::VertexLayout::Texture2D>() = { 0.0f,0.0f };
		model.vertices[1].Attr<Dvtx::VertexLayout::Texture2D>() = { 1.0f,0.0f };
		model.vertices[2].Attr<Dvtx::VertexLayout::Texture2D>() = { 0.0f,1.0f };
		model.vertices[3].Attr<Dvtx::VertexLayout::Texture2D>() = { 1.0f,1.0f };

		AddBind(std::make_shared<Bind::Texture>(gfx, Surface::FromFile("Images\\MyCat2.png")));

		AddBind(std::make_shared<Bind::VertexBuffer>(gfx, model.vertices));

		AddBind(std::make_shared<Bind::Sampler>(gfx));

		auto pvs = std::make_shared<Bind::VertexShader>(gfx, "TextureVS.cso");
		auto pvsbc = pvs->GetBytecode();
		AddBind(std::move(pvs));

		AddBind(std::make_shared<Bind::PixelShader>(gfx, L"TexturePS.cso"));

		AddBind(std::make_shared<Bind::IndexBuffer>(gfx, model.indices));

		AddBind(std::make_shared<InputLayout>(gfx, model.vertices.GetLayout().GetD3DLayout(), pvsbc));


		AddBind(std::make_shared<Bind::Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

		AddBind(std::make_shared<Bind::TransformCbuf>(gfx, *this));
}
DirectX::XMMATRIX Sheet::GetTransformXM() const noexcept
{
	namespace dx = DirectX;
	return dx::XMMatrixRotationRollPitchYaw(pitch, yaw, roll) *
		dx::XMMatrixTranslation(r, 0.0f, 0.0f) *
		dx::XMMatrixRotationRollPitchYaw(theta, phi, chi);
}
