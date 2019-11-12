#pragma once
#include "Graphics.h"
#include "SolidSphere.h"
#include "ConstantBuffer.h"

// 점광원 클래스
/*
	월드에서 점광원을 나타내는 클래스이다
*/

class PointLight
{
public:
	PointLight(Graphics& gfx, float radius = 0.5f);
	
	//광원 컨트롤을 위한 imgui
	void SpawnControlWindow() noexcept;

	//광원위치 초기화
	void Reset() noexcept;

	//광원 시각화 구체를 위한 Draw,Bind
	void Draw(Graphics& gfx) const noexcept (!IS_DEBUG);
	void Bind(Graphics& gfx) const noexcept;
private:
	struct PointLightCBuf
	{
		DirectX::XMFLOAT3 pos;			//광원의 위치
		DirectX::XMFLOAT3 material;		//머터리얼
		DirectX::XMFLOAT3 ambient;		//주변광
		DirectX::XMFLOAT3 diffuseColor; //분산광
		float	diffuseIntensity;
		float	attConst;
		float	attLin;
		float	attQuad;
	};
private:
	PointLightCBuf		cbData;							//광원 상수버퍼
	mutable SolidSphere mesh;							//월드에서 광원의 시각화를 위한 구체
	mutable PixelConstantBuffer<PointLightCBuf> cbuf;	//픽쉘세이더 상수버퍼에 들어갈 값
};

