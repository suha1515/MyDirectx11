#pragma once
#include "Drawable.h"
#include "IndexBuffer.h"

/*
	기존 큐브 뛰우기 예제에서는 80개의 오브젝트를 만들면서 각강의 버퍼와 쉐이더도 80개였다 \
	이런방식은 비효율적이며 Bindable 객체를 계속해서 사용할녀석과 각각의 객체마다 다른 Bindable 객체를 나눠야한다
	
	모든 객체에서 공유하는 변수는 static 변수이다 현재 Box 클래스가 Drawable 클래스를 상속받고있으므로
	해당 static 변수는 많은 Box 클래스 변수들이 공유한다 하지만 Box 클래스에서만 사용해야하난 Drawable의 static변수가 필요한것이다
	만약 상속받는 다른 Sphere 클래스가 있다고 가정하면 이 클래스 또한 해당 static 변수를 공유한다,
	여기서 한가지 트릭을 사용하였는데. 만약 Drawable 클래스에서  static 변수를 두지 못한다면 Drawable클래스와 Box,Sphere 구체와 클래스 사이에
	가운데 클래스 DrawableBase 템플릿 클래스를 만들며 템플릿 클래스에서는 <T> 즉 해당 파라메터에 따라
	컴파일러에서 다른 클래스를 만들게 된다.그런경우 다른클래스가 만들어지면서 static도 만들어지게되는데

	이러한 트릭을 이용하여
	Box 클래스는
	DrawableBase<Box> 로 상속받아 DrawableBase<BOX> 클래스가 만들어져 box클래스에서만 공유가능한
	static 변수가 만들어진다.
*/

template<class T>
class DrawableBase : public Drawable
{
protected:
	//공유가능한 Binable 객체이므로 처음 한번만 초기화하면된다.
	static bool IsStaticInitialized() noexcept
	{
		return !staticBinds.empty();
	}
	static void AddStaticBind(std::unique_ptr<Bindable> bind)noexcept (!IS_DEBUG)
	{
		assert("인덱스버퍼를 바인드하기위해서는 이함수를 사용해선 안됩니다" && typeid(*bind) != typeid(IndexBuffer));
		staticBinds.push_back(std::move(bind));
	}
	void AddStaticIndexBuffer(std::unique_ptr<IndexBuffer> ibuf) noexcept (!IS_DEBUG)
	{
		assert("Attempting to add index buffer a second time" && pIndexBuffer == nullptr);
		pIndexBuffer = ibuf.get();
		staticBinds.push_back(std::move(ibuf));
	}
	//static Bindable 객체중  인덱스 버퍼는 포인터를 추가해야하지만 첫번째 객체말고는 호출할수 없도록 구조가 짜여있다
	//그런 구조를 해결하기위해 아래 함수를 추가한다. staticBind 객체중 인덱스 객체만 찾고 다시 포인터 호출을 해준다.
	void SetIndexFromStatic() noexcept (!IS_DEBUG)
	{
		assert("인덱스 버퍼를 두번 추가하려고 시도했습니다." && pIndexBuffer == nullptr);
		for (const auto& b : staticBinds)
		{
			if (const auto p = dynamic_cast<IndexBuffer*>(b.get()))
			{
				pIndexBuffer = p;
				return;
			}
		}
		assert("정적 바인드 객체들중에 인덱스 버퍼를 찾을 수 없습니다" && pIndexBuffer != nullptr);
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