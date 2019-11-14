#include "TransformCbuf.h"
namespace Bind
{
	TransformCbuf::TransformCbuf(Graphics& gfx, const Drawable& parent, UINT slot)
		:parent(parent)
	{
		//정점 쉐이더에 전달할 행렬이므로 정점쉐이더 상수버퍼를 만든다.
		if (!pVcbuf)
			pVcbuf = std::make_unique<VertexConstantBuffer<Transforms>>(gfx, slot);
	}

	void TransformCbuf::Bind(Graphics& gfx) noexcept
	{
		//해당 객체의 트랜스폼을 가져와 상수버퍼를 만들고 바인딩한다.
		const auto modelView = parent.GetTransformXM() * gfx.GetCamera();
		const Transforms tf = {
			DirectX::XMMatrixTranspose(modelView),
			DirectX::XMMatrixTranspose(
				modelView *
				gfx.GetProjection()
			)
		};
		pVcbuf->Update(gfx, tf);
		pVcbuf->Bind(gfx);
	}

	std::unique_ptr<VertexConstantBuffer<TransformCbuf::Transforms>>TransformCbuf::pVcbuf;
}