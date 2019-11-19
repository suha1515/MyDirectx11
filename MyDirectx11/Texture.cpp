#include "Texture.h"
#include "Surface.h"
#include "GraphicsThrowMacros.h"
#include "BindableCodex.h"

namespace Bind
{
	namespace wrl = Microsoft::WRL;

	Texture::Texture(Graphics& gfx, const std::string& path, UINT slot)
		:
		path(path),
		slot(slot)
	{
		INFOMAN(gfx);

		// load surface 텍스처 로딩.
		const auto s = Surface::FromFile(path);

		//텍스처 자원을 만든다
		//텍스처를 만들기위해 텍스처를 설명하는 구조체를 만든다. (버퍼만들기와 같다)
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = s.GetWidth();
		textureDesc.Height = s.GetHeight();
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;	//쉐이더 자원으로 이용하기위한 플래그
		textureDesc.CPUAccessFlags = 0;						//생성후에는 CPU에 의해 다뤄지지 않는다
		textureDesc.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA sd = {};							//텍스처 정보를 초기화 하기위한 구조체 
		sd.pSysMem = s.GetBufferPtr();							//텍스처 버퍼를 가르키는 시스템메모리의 주소를 지정
		sd.SysMemPitch = s.GetWidth() * sizeof(Surface::Color);	//pitch는 픽셀의 가로해상도 크기 만큼이며 가로해상도* 픽셀의 크기이다.
		wrl::ComPtr<ID3D11Texture2D>pTexture;
		GFX_THROW_INFO(GetDevice(gfx)->CreateTexture2D(&textureDesc, &sd, &pTexture));

		//위 코드는 텍스처를 생성하는것으로 밑에서 추가적인 작업이 필요하다
		//바로 텍스처를 파이프라인에 바인드 할 수 없다 view를 바인드 해야한다.

		//텍스처에 대한 리소스뷰를 만들어야한다
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;			//밉
		srvDesc.Texture2D.MipLevels = 1;

		//쉐이더 리소스뷰를 만든다.
		GFX_THROW_INFO(GetDevice(gfx)->CreateShaderResourceView(
			pTexture.Get(), &srvDesc, &pTextureView
		));
	}
	void Texture::Bind(Graphics& gfx) noexcept
	{
		//만들어진 텍스처에 대한 쉐이더 리소스 뷰를 픽셀쉐이더리소스에 바인딩한다
		// slot 변수의 추가로 텍스쳐 자원에대해 슬롯에 따라 따로 지정할 수 있다.
		GetContext(gfx)->PSSetShaderResources(slot, 1u, pTextureView.GetAddressOf());

	}
	std::shared_ptr<Bindable> Texture::Resolve(Graphics& gfx, const std::string& path, UINT slot)
	{
		return Codex::Resolve<Texture>(gfx, path, slot);
	}
	std::string Texture::GenerateUID(const std::string& path, UINT slot)
	{
		//텍스쳐 바인딩 객체는 경로와 슬롯으로 UID를 구분짓는다.
		using namespace std::string_literals;
		return typeid(Texture).name() + "#"s + path + "#" + std::to_string(slot);
	}
	std::string Texture::GetUID() const noexcept
	{
		return GenerateUID(path, slot);
	}
}