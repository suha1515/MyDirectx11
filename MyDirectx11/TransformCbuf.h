#pragma once
#include "ConstantBuffer.h"
#include "Drawable.h"

//��ȯ��� ������� Ŭ����
/*
	���� ���̴��� ������ ����,��,���� ����� ������۷� �����ϴ� Ŭ�����̴�.
*/
namespace Bind
{
	class TransformCbuf :
		public Bindable
	{
	//TransformCbuf �� Ȯ�强�� ���� �����丵
	protected:
		struct Transforms
		{
			DirectX::XMMATRIX modelView;
			DirectX::XMMATRIX modelViewProj;
		};
	public:
		TransformCbuf(Graphics& gfx, const Drawable& parent, UINT slot = 0u);
		void Bind(Graphics& gfx)noexcept override;
	protected:
		void UpdateBindImpl(Graphics& gfx, const Transforms& tf) noexcept;
		Transforms GetTransforms(Graphics& gfx) noexcept;
	private:
		static std::unique_ptr < VertexConstantBuffer<Transforms>> pVcbuf;
		const Drawable& parent;
	};

}