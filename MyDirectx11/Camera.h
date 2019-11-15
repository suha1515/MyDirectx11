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
	Camera() noexcept;
	DirectX::XMMATRIX GetMatrix() const noexcept;
	void SpwanControlWindow() noexcept;
	void Reset() noexcept;
	void Rotate(float dx, float dy) noexcept;
	void Translate(DirectX::XMFLOAT3 translation) noexcept;
private:
	DirectX::XMFLOAT3 pos;
	float pitch;
	float yaw;
	static constexpr float travelSpeed = 12.0f;
	static constexpr float rotationSpeed = 0.004f;
};

