#include "Sampler.h"
#include "GraphicsThrowMacros.h"

Sampler::Sampler(Graphics& gfx)
{
	INFOMAN(gfx);

	//���ý�����Ʈ�� ������ ����ü�� ä���
	// ���� ���� ���θ�带 �����Ұ��ε�
	// ���θ��� �ؽ��İ� clamp,wrap�� ��� �������� �Ѿ�� ǥ������ �����ϸ�
	//D3D11_FILTER ����ü�� Ȯ���غ��� �پ��� �������ִ�.
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR  ;		//LINEAR,NEAREST,BILINEAR ���..
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	//���� ���÷� ������Ʈ�� �����Ѵ�.
	GFX_THROW_INFO(GetDevice(gfx)->CreateSamplerState(&samplerDesc, &pSampler));

}

void Sampler::Bind(Graphics& gfx) noexcept
{
	GetContext(gfx)->PSSetSamplers(0, 1, pSampler.GetAddressOf());
	//1��° �Ű������� �������� �������Ϳ� �����ȴ� �̰��� ���̴����� ���ٰ����� ���������̴�.
}
