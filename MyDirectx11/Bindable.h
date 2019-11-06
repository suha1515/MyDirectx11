#pragma once
#include "Graphics.h"

//파이프라인에 바인딩하는 오브젝트에 대한 인터페이스

class Bindable
{
public:
	virtual void Bind(Graphics& gfx) noexcept = 0;
	virtual ~Bindable() = default;
protected:
	static ID3D11DeviceContext* GetContext(Graphics& gfx) noexcept;
	static ID3D11Device* GetDevice(Graphics& gfx) noexcept;
	static DxgiInfoManager& GetInfoManager(Graphics& gfx) noexcept(!IS_DEBUG);
	
};

