#pragma once

#include "DrawableBase.h"
class Box :
	public DrawableBase<Box>
{
public:
	Box(Graphics& gfx, std::mt19937& rng,
		std::uniform_real_distribution<float>& adist,
		std::uniform_real_distribution<float>& ddist,
		std::uniform_real_distribution<float>& odist,
		std::uniform_real_distribution<float>& rdist);
	void Update(float dt) noexcept override;
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
private:
	//Positional
	float r;				//radius 중점으로부터의 반지름
	float roll = 0.0f;
	float pitch = 0.0f;
	float yaw = 0.0f;
	float theta;
	float phi;
	float chi;
	//Speed(delta/s)
	float droll;			//박스에대한 회전(아래 두개까지)
	float dpitch;
	float dyaw;
	float dtheta;			//월드의 중점에 대한회전(아래 두개까지)
	float dphi;
	float dchi;
};

