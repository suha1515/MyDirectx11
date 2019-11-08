#pragma once
#include "Bindable.h"

// ���÷� ������Ʈ
// ���������ο��� �ؽ��İ� ��� ���������� ���ؼ� �����ش�.

class Sampler : public Bindable
{
public:
	Sampler(Graphics& gfx);
	void Bind(Graphics& gfx) noexcept override;
protected:
	Microsoft::WRL::ComPtr<ID3D11SamplerState> pSampler;
};

