#include "TransformCbuf.h"

TransformCbuf::TransformCbuf(Graphics& gfx, const Drawable& parent)
	:parent(parent)
{
	//정점 쉐이더에 전달할 행렬이므로 정점쉐이더 상수버퍼를 만든다.
	if (!pVcbuf)
		pVcbuf = std::make_unique<VertexConstantBuffer<Transforms>>(gfx);
}

void TransformCbuf::Bind(Graphics& gfx) noexcept
{
	//해당 객체의 트랜스폼을 가져와 상수버퍼를 만들고 바인딩한다.
	const auto model = parent.GetTransformXM();
	const Transforms tf = {
		DirectX::XMMatrixTranspose(model),
		DirectX::XMMatrixTranspose(
			model *
			gfx.GetCamera() *
			gfx.GetProjection()
		)
	};
	pVcbuf->Update(gfx,tf);
	pVcbuf->Bind(gfx);
}

std::unique_ptr<VertexConstantBuffer<TransformCbuf::Transforms>>TransformCbuf::pVcbuf;
