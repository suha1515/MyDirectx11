  #include "Drawable.h"
#include "GraphicsThrowMacros.h"
#include "IndexBuffer.h"
#include <cassert>

using namespace Bind;

void Drawable::Draw(Graphics& gfx) const noxnd
{
	for (auto& b : binds)
		b->Bind(gfx);
	for (auto& b : GetStaticBinds())
		b->Bind(gfx);

	gfx.DrawIndexed(pIndexBuffer->GetCount());
}

void Drawable::AddBind(std::unique_ptr<Bindable> bind) noxnd
{
	assert("인덱스 버퍼를 바인딩하기위해서 AddIndexBuffer를 사용하시오 " && typeid(*bind)!= typeid(IndexBuffer));
	binds.push_back(std::move(bind));
}

void Drawable::AddIndexBuffer(std::unique_ptr<class IndexBuffer> ibuf) noxnd
{
	assert("인덱스버퍼를 두번 추가했습니다" && pIndexBuffer == nullptr);
	pIndexBuffer = ibuf.get();
	binds.push_back(std::move(ibuf));
}
