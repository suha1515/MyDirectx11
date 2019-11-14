#pragma once
#include "Bindable.h"

class Surface;

namespace Bind
{
	class Texture :public Bindable
	{
	public:
		Texture(Graphics& gfx, const class Surface& s);
		void Bind(Graphics& gfx) noexcept override;
	protected:
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTextureView;
		//쉐이더에 묶이는 텍스처 자원이다.
	};

}