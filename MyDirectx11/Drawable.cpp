  #include "Drawable.h"
#include "GraphicsThrowMacros.h"
#include "IndexBuffer.h"
#include <cassert>

using namespace Bind;

void Drawable::Draw(Graphics& gfx) const noxnd
{
	for (auto& b : binds)
		b->Bind(gfx);

	gfx.DrawIndexed(pIndexBuffer->GetCount());
}

//한개로 축소.
void Drawable::AddBind(std::shared_ptr<Bindable> bind) noxnd
{
	//인덱스 버퍼의 경우
	if (typeid(*bind) == typeid(IndexBuffer))
	{
		assert("다수의 인덱스버퍼를 바인딩할수 없습니다." && pIndexBuffer == nullptr);
		pIndexBuffer = &static_cast<IndexBuffer&>(*bind);
	}
	binds.push_back(std::move(bind));
}
