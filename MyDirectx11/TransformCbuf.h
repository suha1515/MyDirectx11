#pragma once
#include "ConstantBuffer.h"
#include "Drawable.h"

//변환행렬 상수버퍼 클래스
/*
	정점 쉐이더에 전달할 월드,뷰,투영 행렬을 상수버퍼로 전달하는 클래스이다.
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

