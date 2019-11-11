#pragma once
#include "Graphics.h"
//Camera 클래스
/*
	뷰행렬및 카메라기능을 제공한다
	imgui에서의 gui 구성도 제공(변수값 조절)
*/

class Camera
{
public:
	DirectX::XMMATRIX GetMatrix() const noexcept;
	void SpwanControlWindow() noexcept;
	void Reset() noexcept;
private:
	//from origin
	float radius = 20.f;
	float theta = 0.0f;
	float phi = 0.0f;
	//from itself
	float pitch= 0.0f;
	float yaw  = 0.0f;
	float roll = 0.0f;
};

