#pragma once

#include "Bindable.h"
#include "BindableCodex.h"
#include <memory>
#include <unordered_map>

// Codex Ŭ����
/*
	�ٸ� Drwable ��ü������ ���ε� ������(Bindable) ���δ���ü��  
	�����ϱ����� �����ϴ� �߽� �����̳� 
	�̱��� �����̴�.
*/

namespace Bind
{
	class Codex
	{
	public:
		// CodexŬ������ �پ��� ���δ��� ��ü�� �����ؾ��ϹǷ� �� ���δ� ������Ʈ����
		//  ���� �ٸ� ������ �����ڸ� �������ִ� �װ��� ����Ͽ� �Ķ���� ������ �����ڸ� �����Ѵ�.
		template<class T,typename ... Params>
		static std::shared_ptr<Bindable> Resolve(Graphics& gfx,Params&&...p) noxnd
		{
			return Get().Resolve_<T>(gfx,std::forward<Params>(p)...);
		}
	private:
		template<class T,typename ... Params>
		std::shared_ptr<Bindable> Resolve_(Graphics& gfx,Params&&...p)  noxnd
		{
			const auto key = T::GenerateUID(std::forward<Params>(p)...);
			const auto i = binds.find(key);
			//ã�� ���δ��� ������Ʈ�� ���ٸ� ���� �����.
			if (i == binds.end())
			{
				auto bind = std::make_shared <T>(gfx, std::forward<Params>(p)...);
				binds[key] = bind;
				return bind;
			}
			else
				return i->second;
		}
		static Codex& Get()
		{
			static Codex codex;
			return codex;
		}
	private:
		//�ؽ� ����� ��, ��� ���δ���ü�� ������ �ؽ�Ű�� �����Եȴ�.
		std::unordered_map<std::string, std::shared_ptr<Bindable>>binds;
	};


}