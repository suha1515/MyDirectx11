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
	//���ؽ� ������۶��� staticȭ�Ͽ� �� ��ü���� �����ϵ��� ����Ѵ�.
	static std::unique_ptr < VertexConstantBuffer<DirectX::XMMATRIX>> pVcbuf;
	const Drawable& parent;
};

