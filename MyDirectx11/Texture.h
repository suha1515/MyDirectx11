#pragma once
#include "Bindable.h"

class Surface;

namespace Bind
{
	class Texture :public Bindable
	{
	public:
		Texture(Graphics& gfx, const std::string& path,UINT slot =0);
		void Bind(Graphics& gfx) noexcept override;
		static std::shared_ptr<Bindable> Resolve(Graphics& gfx, const std::string& path, UINT slot=0);
		static std::string GenerateUID(const std::string& path, UINT slot=0);
		std::string GetUID() const noexcept override;
	private:
		unsigned int slot; //다중 텍스쳐를 위한 슬롯
	protected:
		std::string path;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTextureView;
		//쉐이더에 묶이는 텍스처 자원이다.
	};

}