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

//�Ѱ��� ���.
void Drawable::AddBind(std::shared_ptr<Bindable> bind) noxnd
{
	//�ε��� ������ ���
	if (typeid(*bind) == typeid(IndexBuffer))
	{
		assert("�ټ��� �ε������۸� ���ε��Ҽ� �����ϴ�." && pIndexBuffer == nullptr);
		pIndexBuffer = &static_cast<IndexBuffer&>(*bind);
	}
	binds.push_back(std::move(bind));
}
