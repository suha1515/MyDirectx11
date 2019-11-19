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
		//string key�� �Ѱ� ���δ��� ��ü�� �����´�. ������ ����ִ� shared_ptr��ȯ
		static std::shared_ptr<Bindable> Resolve(const std::string& key) noxnd
		{
			return Get().Resolve_(key);
		}
		//���δ��� ��ü�� �����Ѵ�. ���δ��� ��ü�� GetUID�� ID���� ��ȯ�ϴ� �޼��尡 �ִ�.
		static void Store(std::shared_ptr<Bindable>bind)
		{
			Get().Store_(std::move(bind));
		}
	private:
		std::shared_ptr<Bindable> Resolve_(const std::string& key) const noxnd
		{
			auto i = binds.find(key);
			if (i == binds.end())
				return {};
			else
				return i->second;
		}
		void Store_(std::shared_ptr<Bindable>bind)
		{
			binds[bind->GetUID()] = std::move(bind);
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