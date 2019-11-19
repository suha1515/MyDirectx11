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
		//코덱스에서 해당 바인딩 객체를 가져온다. 없을시 생성
		static std::shared_ptr<VertexShader> Resolve(Graphics& gfx, const std::string& path);
		//해당 바인딩 객체의 UID를 만든다.
		static std::string GenerateUID(const std::string& path);
		//바인딩 객체의 UID 반환
		std::string	GetUID() const noexcept override;
	protected:
		//정점쉐이더 경로
		std::string path;
		Microsoft::WRL::ComPtr<ID3DBlob> pBytecodeBlob;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader;
	};
}