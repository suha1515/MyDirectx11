#pragma once

#include "Bindable.h"
#include "GraphicsThrowMacros.h"

template<typename T>
class ConstantBuffer : public Bindable
{
public:
	//������۸� ����Ѵ�.
	//����̽����� Map,UnMap�� �̿��Ͽ� �ش������ �ּҿ��� T& consts �� �����͸� ����Ѵ�.
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
	//������� ������
	//������ �뵵�� �����ϴ� ����ü�� ���ο� ���۸� ����� �ش� ������ D3D11_SUBRESOURCE_DATA�� �̿��Ͽ� �����Ѵ�.
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
	//������ ���� ���۸� �����.
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


//���� ���̴� ���� ������� ���ø�.
template<typename T>
class VertexConstantBuffer : public ConstantBuffer<T>
{
	using ConstantBuffer<T>::pConstantBuffer;
	using ConstantBuffer<T>::slot;
	using Bindable::GetContext;
public:
	using ConstantBuffer<T>::ConstantBuffer;
	//�θ�� ConstantBuffer�� �����ڰ� ���� ȣ����� ����̽����� VSSetConstantBuffer ȣ��� �������̴��� ������۸� ���ε��Ѵ�.
	void Bind(Graphics& gfx) noexcept override
	{
		GetContext(gfx)->VSSetConstantBuffers(slot, 1u, pConstantBuffer.GetAddressOf());
	}
};
//�ȼ� ���̴� ���� ������� ���ø�.
template<typename T>
class PixelConstantBuffer : public ConstantBuffer<T>
{
	using ConstantBuffer<T>::pConstantBuffer;
	using ConstantBuffer<T>::slot;
	using Bindable::GetContext;
public:
	using ConstantBuffer<T>::ConstantBuffer;
	//�θ�� ConstantBuffer�� �����ڰ� ���� ȣ����� ����̽����� VSSetConstantBuffer ȣ��� �ȼ����̴��� ������۸� ���ε��Ѵ�.
	void Bind(Graphics& gfx) noexcept override
	{
		GetContext(gfx)->PSSetConstantBuffers(slot, 1u, pConstantBuffer.GetAddressOf());
	}
};

