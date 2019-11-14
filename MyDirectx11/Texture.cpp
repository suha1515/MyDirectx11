#include "Texture.h"
#include "Surface.h"
#include "GraphicsThrowMacros.h"

namespace Bind
{
	namespace wrl = Microsoft::WRL;

	Texture::Texture(Graphics& gfx, const Surface& s)
	{
		INFOMAN(gfx);

		//�ؽ�ó �ڿ��� �����
		//�ؽ�ó�� ��������� �ؽ�ó�� �����ϴ� ����ü�� �����. (���۸����� ����)
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = s.GetWidth();
		textureDesc.Height = s.GetHeight();
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;	//���̴� �ڿ����� �̿��ϱ����� �÷���
		textureDesc.CPUAccessFlags = 0;						//�����Ŀ��� CPU�� ���� �ٷ����� �ʴ´�
		textureDesc.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA sd = {};							//�ؽ�ó ������ �ʱ�ȭ �ϱ����� ����ü 
		sd.pSysMem = s.GetBufferPtr();							//�ؽ�ó ���۸� ����Ű�� �ý��۸޸��� �ּҸ� ����
		sd.SysMemPitch = s.GetWidth() * sizeof(Surface::Color);	//pitch�� �ȼ��� �����ػ� ũ�� ��ŭ�̸� �����ػ�* �ȼ��� ũ���̴�.
		wrl::ComPtr<ID3D11Texture2D>pTexture;
		GFX_THROW_INFO(GetDevice(gfx)->CreateTexture2D(&textureDesc, &sd, &pTexture));

		//�� �ڵ�� �ؽ�ó�� �����ϴ°����� �ؿ��� �߰����� �۾��� �ʿ��ϴ�
		//�ٷ� �ؽ�ó�� ���������ο� ���ε� �� �� ���� view�� ���ε� �ؾ��Ѵ�.

		//�ؽ�ó�� ���� ���ҽ��並 �������Ѵ�
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;			//��
		srvDesc.Texture2D.MipLevels = 1;

		//���̴� ���ҽ��並 �����.
		GFX_THROW_INFO(GetDevice(gfx)->CreateShaderResourceView(
			pTexture.Get(), &srvDesc, &pTextureView
		));
	}
	void Texture::Bind(Graphics& gfx) noexcept
	{
		//������� �ؽ�ó�� ���� ���̴� ���ҽ� �並 �ȼ����̴����ҽ��� ���ε��Ѵ�
		GetContext(gfx)->PSSetShaderResources(0u, 1u, pTextureView.GetAddressOf());

	}
}