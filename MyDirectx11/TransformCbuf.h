#pragma once
#include "ConstantBuffer.h"
#include "Drawable.h"

//변환행렬 상수버퍼 클래스
/*
	정점 쉐이더에 전달할 월드,뷰,투영 행렬을 상수버퍼로 전달하는 클래스이다.
*/
namespace Bind
{
	class TransformCbuf :
		public Bindable
	{
	//TransformCbuf 의 확장성을 위한 리팩토링
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