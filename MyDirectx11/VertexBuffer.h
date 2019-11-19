#pragma once
#include "Bindable.h"
#include "GraphicsThrowMacros.h"
#include "Vertex.h"
namespace Bind
{
	class VertexBuffer :
		public Bindable
	{
	public:
		VertexBuffer(Graphics& gfx, const Dvtx::VertexBuffer& vbuf);
		//정점버퍼의 경우 Codex에서 구분할 UID 유니크 아이디는 사용자 태그를 통해 비교한다.
		VertexBuffer(Graphics& gfx,const std::string& tag, const Dvtx::VertexBuffer& vbuf);
		void Bind(Graphics& gfx)noexcept override;
		static std::shared_ptr<Bindable> Resolve(Graphics& gfx, const std::string& tag, const Dvtx::VertexBuffer& vbuf);
		
		template<typename... Ignore>
		static std::string GenerateUID(const std::string& tag, Ignore&&...ignore)
		{
			return GenerateUID_(tag);
		}
		std::string GetUID() const noexcept override;
	private:
		static std::string GenerateUID_(const std::string& tag);
	protected:
		std::string tag;
		UINT stride;
		Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer;
	};
}