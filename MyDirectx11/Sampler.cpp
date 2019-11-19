#include "Sampler.h"
#include "GraphicsThrowMacros.h"
#include "BindableCodex.h"
namespace Bind
{
	Sampler::Sampler(Graphics& gfx)
	{
		INFOMAN(gfx);

		//샘플스테이트를 설명할 구조체를 채운다
		// 필터 모드와 랩핑모드를 선택할것인데
		// 랩핑모드는 텍스쳐가 clamp,wrap등 어떻게 범위밖을 넘어가면 표시할지 설명하며
		//D3D11_FILTER 열거체를 확인해보자 다양한 설정이있다.
		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;		//LINEAR,NEAREST,BILINEAR 등등..
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

		//그후 샘플러 스테이트를 지정한다.
		GFX_THROW_INFO(GetDevice(gfx)->CreateSamplerState(&samplerDesc, &pSampler));

	}

	void Sampler::Bind(Graphics& gfx) noexcept
	{
		GetContext(gfx)->PSSetSamplers(0, 1, pSampler.GetAddressOf());
		//1번째 매개변수는 슬롯으로 레지스터에 대응된다 이값은 쉐이더에서 접근가능한 레지스터이다.
	}
	std::shared_ptr<Bindable> Sampler::Resolve(Graphics&   gfx)
	{
		return Codex::Resolve<Sampler>(gfx);
	}
	std::string Sampler::GenerateUID()
	{
		return typeid(Sampler).name();
	}
	std::string Sampler::GetUID() const noexcept
	{
		return GenerateUID();
	}
}