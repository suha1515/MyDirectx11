#pragma once
#include "Graphics.h"
#include "SolidSphere.h"
#include "ConstantBuffer.h"
#include "ConditionalNoexcept.h"
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
	void Draw(Graphics& gfx) const noxnd;
	void Bind(Graphics& gfx, DirectX::FXMMATRIX view) const noexcept;
private:
	// 상수버퍼를 넘길때 hlsl 쉐이더에서는 해당 변수사이에 padding 값이 있다고 가정하고받는다
	// 하지만 우리의 구조체는 패딩값없이 바로붙어있으므로 hlsl에서 작은값이 왔다고 에러 메시지를 보내게된다
	// 이경우 구조체 변수사이에 패딩 변수를 넣거나 alignas 키워드를 사용하여 해당 변수사이에 패딩값을 넣자

	//float3 의 경우 hlsl 에서 16패딩값을 가지고 있다고 가정할것이다. 그냥 float은 되는듯
	struct PointLightCBuf
	{
		alignas(16)DirectX::XMFLOAT3 pos;			//광원의 위치
		alignas(16)DirectX::XMFLOAT3 ambient;		//주변광
		alignas(16)DirectX::XMFLOAT3 diffuseColor; //분산광
		float	diffuseIntensity;
		float	attConst;
		float	attLin;
		float	attQuad;
	};
private:
	PointLightCBuf		cbData;							//광원 상수버퍼
	mutable SolidSphere mesh;							//월드에서 광원의 시각화를 위한 구체
	mutable Bind::PixelConstantBuffer<PointLightCBuf> cbuf;	//픽쉘세이더 상수버퍼에 들어갈 값
};

