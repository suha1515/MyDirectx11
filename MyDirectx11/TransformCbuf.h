#pragma once
#include "ConstantBuffer.h"
#include "Drawable.h"

//��ȯ��� ������� Ŭ����
/*
	���� ���̴��� ������ ����,��,���� ����� ������۷� �����ϴ� Ŭ�����̴�.
*/

class TransformCbuf :
	public Bindable
{
private:
	struct Transforms
	{
		DirectX::XMMATRIX modelViewProj;
		DirectX::XMMATRIX model;
	};
public:
	TransformCbuf(Graphics& gfx, const Drawable& parent,UINT slot = 0u);
	void Bind(Graphics& gfx)noexcept override;
private:
	static std::unique_ptr < VertexConstantBuffer<Transforms>> pVcbuf;
	const Drawable& parent;
};

