#pragma once
#include "Bindable.h"

// 샘플러 스테이트
// 파이프라인에게 텍스쳐가 어떻게 보여질지에 대해서 도와준다.

namespace Bind
{
	class Sampler : public Bindable
	{
	public:
		Sampler(Graphics& gfx);
		void Bind(Graphics& gfx) noexcept override;
		static std::shared_ptr<Bindable> Resolve(Graphics& gfx);
		static std::string GenerateUID();
		std::string GetUID() const noexcept override;
	protected:
		Microsoft::WRL::ComPtr<ID3D11SamplerState> pSampler;
	};
}