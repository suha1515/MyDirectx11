#pragma once
#include "Drawable.h"
class TestPlane : public Drawable
{
public:
	TestPlane(Graphics& gfx, float size);
	void SetPos(DirectX::XMFLOAT3 pos) noexcept;
	void SetRotation(float roll, float pitch, float yaw) noexcept;
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
	void SpawnControlWindow(Graphics& gfx) noexcept;
private:
	struct PSMaterialConstant
	{
		float specularIntensity = 0.18f;
		float specularPower = 18.0f;
		 BOOL normalMappingEnabled = TRUE;	//hlsl 에서 bool이 4바이트이기에 Window.h에서 제공하는 int크기의 BOOL을 사용한다
		float padding[1];
	}pmc;
	DirectX::XMFLOAT3 pos = { 0.0f,0.0f,0.0f };
	float roll = 0.0f;
	float pitch = 0.0f;
	float yaw = 0.0f;

};

