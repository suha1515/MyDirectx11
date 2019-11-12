#pragma once

#include "Bindable.h"
#include "GraphicsThrowMacros.h"

template<typename T>
class ConstantBuffer : public Bindable
{
public:
	//상수버퍼를 기록한다.
	//디바이스에서 Map,UnMap을 이용하여 해당버퍼의 주소에다 T& consts 의 데이터를 기록한다.
	void Update(Graphics& gfx, const T& consts)
	{
		INFOMAN(gfx);

		D3D11_MAPPED_SUBRESOURCE msr;
		GFX_THROW_INFO(GetContext(gfx)->Map(
			pConstantBuffer.Get(), 0u,
			D3D11_MAP_WRITE_DISCARD, 0u,
			&msr

		));
		memcpy(msr.pData, &consts, sizeof(consts));
		GetContext(gfx)->Unmap(pConstantBuffer.Get(), 0u);
	}
	//상수버퍼 생성자
	//버퍼의 용도를 정의하는 구조체로 새로운 버퍼를 만들고 해당 내용을 D3D11_SUBRESOURCE_DATA를 이용하여 전달한다.
	ConstantBuffer(Graphics& gfx, const T& consts,UINT slot = 0u)
		:slot(slot)
	{
		INFOMAN(gfx);

		D3D11_BUFFER_DESC cbd;
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.MiscFlags = 0u;
		cbd.ByteWidth = sizeof(consts);
		cbd.StructureByteStride = 0u;

		D3D11_SUBRESOURCE_DATA csd = {};
		csd.pSysMem = &consts;
		GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&cbd, &csd, &pConstantBuffer));
	}
	//데이터 없이 버퍼만 만든다.
	ConstantBuffer(Graphics& gfx, UINT slot = 0u)
		:slot(slot)
	{
		INFOMAN(gfx);

		D3D11_BUFFER_DESC cbd;
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.MiscFlags = 0u;
		cbd.ByteWidth = sizeof(T);
		cbd.StructureByteStride = 0u;
		GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&cbd, nullptr, &pConstantBuffer));
	}
protected:
	Microsoft::WRL::ComPtr<ID3D11Buffer> pConstantBuffer;
	UINT	slot;
};


//정점 쉐이더 전용 상수버퍼 템플릿.
template<typename T>
class VertexConstantBuffer : public ConstantBuffer<T>
{
	using ConstantBuffer<T>::pConstantBuffer;
	using ConstantBuffer<T>::slot;
	using Bindable::GetContext;
public:
	using ConstantBuffer<T>::ConstantBuffer;
	//부모로 ConstantBuffer의 생성자가 먼저 호출된후 디바이스에서 VSSetConstantBuffer 호출로 정점쉐이더에 상수버퍼를 바인딩한다.
	void Bind(Graphics& gfx) noexcept override
	{
		GetContext(gfx)->VSSetConstantBuffers(slot, 1u, pConstantBuffer.GetAddressOf());
	}
};
//픽셀 쉐이더 전용 상수버퍼 템플릿.
template<typename T>
class PixelConstantBuffer : public ConstantBuffer<T>
{
	using ConstantBuffer<T>::pConstantBuffer;
	using ConstantBuffer<T>::slot;
	using Bindable::GetContext;
public:
	using ConstantBuffer<T>::ConstantBuffer;
	//부모로 ConstantBuffer의 생성자가 먼저 호출된후 디바이스에서 VSSetConstantBuffer 호출로 픽셀쉐이더에 상수버퍼를 바인딩한다.
	void Bind(Graphics& gfx) noexcept override
	{
		GetContext(gfx)->PSSetConstantBuffers(slot, 1u, pConstantBuffer.GetAddressOf());
	}
};

