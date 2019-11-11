#pragma once
#include "Graphics.h"
//Camera Ŭ����
/*
	����Ĺ� ī�޶����� �����Ѵ�
	imgui������ gui ������ ����(������ ����)
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

