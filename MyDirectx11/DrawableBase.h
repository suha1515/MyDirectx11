#pragma once
#include "Drawable.h"
#include "IndexBuffer.h"

/*
	���� ť�� �ٿ�� ���������� 80���� ������Ʈ�� ����鼭 ������ ���ۿ� ���̴��� 80������ \
	�̷������ ��ȿ�����̸� Bindable ��ü�� ����ؼ� ����ҳ༮�� ������ ��ü���� �ٸ� Bindable ��ü�� �������Ѵ�
	
	��� ��ü���� �����ϴ� ������ static �����̴� ���� Box Ŭ������ Drawable Ŭ������ ��ӹް������Ƿ�
	�ش� static ������ ���� Box Ŭ���� �������� �����Ѵ� ������ Box Ŭ���������� ����ؾ��ϳ� Drawable�� static������ �ʿ��Ѱ��̴�
	���� ��ӹ޴� �ٸ� Sphere Ŭ������ �ִٰ� �����ϸ� �� Ŭ���� ���� �ش� static ������ �����Ѵ�,
	���⼭ �Ѱ��� Ʈ���� ����Ͽ��µ�. ���� Drawable Ŭ��������  static ������ ���� ���Ѵٸ� DrawableŬ������ Box,Sphere ��ü�� Ŭ���� ���̿�
	��� Ŭ���� DrawableBase ���ø� Ŭ������ ����� ���ø� Ŭ���������� <T> �� �ش� �Ķ���Ϳ� ����
	�����Ϸ����� �ٸ� Ŭ������ ����� �ȴ�.�׷���� �ٸ�Ŭ������ ��������鼭 static�� ��������ԵǴµ�

	�̷��� Ʈ���� �̿��Ͽ�
	Box Ŭ������
	DrawableBase<Box> �� ��ӹ޾� DrawableBase<BOX> Ŭ������ ������� boxŬ���������� ����������
	static ������ ���������.
*/

template<class T>
class DrawableBase : public Drawable
{
protected:
	//���������� Binable ��ü�̹Ƿ� ó�� �ѹ��� �ʱ�ȭ�ϸ�ȴ�.
	static bool IsStaticInitialized() noexcept
	{
		return !staticBinds.empty();
	}
	static void AddStaticBind(std::unique_ptr<Bindable> bind)noexcept (!IS_DEBUG)
	{
		assert("�ε������۸� ���ε��ϱ����ؼ��� ���Լ��� ����ؼ� �ȵ˴ϴ�" && typeid(*bind) != typeid(IndexBuffer));
		staticBinds.push_back(std::move(bind));
	}
	void AddStaticIndexBuffer(std::unique_ptr<IndexBuffer> ibuf) noexcept (!IS_DEBUG)
	{
		assert("Attempting to add index buffer a second time" && pIndexBuffer == nullptr);
		pIndexBuffer = ibuf.get();
		staticBinds.push_back(std::move(ibuf));
	}
	//static Bindable ��ü��  �ε��� ���۴� �����͸� �߰��ؾ������� ù��° ��ü����� ȣ���Ҽ� ������ ������ ¥���ִ�
	//�׷� ������ �ذ��ϱ����� �Ʒ� �Լ��� �߰��Ѵ�. staticBind ��ü�� �ε��� ��ü�� ã�� �ٽ� ������ ȣ���� ���ش�.
	void SetIndexFromStatic() noexcept (!IS_DEBUG)
	{
		assert("�ε��� ���۸� �ι� �߰��Ϸ��� �õ��߽��ϴ�." && pIndexBuffer == nullptr);
		for (const auto& b : staticBinds)
		{
			if (const auto p = dynamic_cast<IndexBuffer*>(b.get()))
			{
				pIndexBuffer = p;
				return;
			}
		}
		assert("���� ���ε� ��ü���߿� �ε��� ���۸� ã�� �� �����ϴ�" && pIndexBuffer != nullptr);
	}
private:
	const std::vector < std::unique_ptr<Bindable>> & GetStaticBinds() const noexcept override
	{
		return staticBinds;
	}  
private:
	static std::vector <std::unique_ptr<Bindable>> staticBinds;
};

template<class T>
std::vector<std::unique_ptr<Bindable>> DrawableBase<T>::staticBinds;