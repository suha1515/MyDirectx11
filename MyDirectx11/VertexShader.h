#pragma once
#include "Bindable.h"
namespace Bind
{
	class VertexShader :
		public Bindable
	{
	public:
		VertexShader(Graphics& gfx, const std::string& path);
		void Bind(Graphics& gfx) noexcept override;
		ID3DBlob* GetBytecode() const noexcept;
		//�ڵ������� �ش� ���ε� ��ü�� �����´�. ������ ����
		static std::shared_ptr<VertexShader> Resolve(Graphics& gfx, const std::string& path);
		//�ش� ���ε� ��ü�� UID�� �����.
		static std::string GenerateUID(const std::string& path);
		//���ε� ��ü�� UID ��ȯ
		std::string	GetUID() const noexcept override;
	protected:
		//�������̴� ���
		std::string path;
		Microsoft::WRL::ComPtr<ID3DBlob> pBytecodeBlob;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader;
	};
}