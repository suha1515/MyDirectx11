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
	assert("�ε��� ���۸� ���ε��ϱ����ؼ� AddIndexBuffer�� ����Ͻÿ� " && typeid(*bind)!= typeid(IndexBuffer));
	binds.push_back(std::move(bind));
}

void Drawable::AddIndexBuffer(std::unique_ptr<class IndexBuffer> ibuf) noxnd
{
	assert("�ε������۸� �ι� �߰��߽��ϴ�" && pIndexBuffer == nullptr);
	pIndexBuffer = ibuf.get();
	binds.push_back(std::move(ibuf));
}
