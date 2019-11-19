#pragma once
#include "Graphics.h"
#include "ConditionalNoexcept.h"

namespace Bind
{

	//파이프라인에 바인딩하는 오브젝트에 대한 인터페이스

	class Bindable
	{
	public:
		virtual void Bind(Graphics& gfx) noexcept = 0;
		//모든 바인딩 오브젝트는 아래함수를 통해 키값을 반환한다.
		virtual std::string GetUID() const noexcept
		{
			assert(false);
			return "";
		}
		virtual ~Bindable() = default;
	protected:
		static ID3D11DeviceContext* GetContext(Graphics& gfx) noexcept;
		static ID3D11Device* GetDevice(Graphics& gfx) noexcept;
		static DxgiInfoManager& GetInfoManager(Graphics& gfx) noxnd;
	};
}
