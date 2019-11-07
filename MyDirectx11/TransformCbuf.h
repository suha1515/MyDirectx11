#pragma once
#include "ConstantBuffer.h"
#include "Drawable.h"

class TransformCbuf :
	public Bindable
{
public:
	TransformCbuf(Graphics& gfx, const Drawable& parent);
	void Bind(Graphics& gfx)noexcept override;
private:
	//VertexConstantBuffer<DirectX::XMMATRIX> vcbuf;
	//버텍스 상수버퍼또한 static화하여 각 객체마다 공유하도록 사용한다.
	static std::unique_ptr < VertexConstantBuffer<DirectX::XMMATRIX>> pVcbuf;
	const Drawable& parent;
};

