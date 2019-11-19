#pragma once

#include "Bindable.h"
#include "BindableCodex.h"
#include <memory>
#include <unordered_map>

// Codex 클래스
/*
	다른 Drwable 객체사이의 바인드 가능한(Bindable) 바인더블객체를  
	공유하기위해 저장하는 중심 컨테이너 
	싱글톤 구조이다.
*/

namespace Bind
{
	class Codex
	{
	public:
		//string key를 넘겨 바인더블 객체를 가져온다. 없을시 비어있는 shared_ptr반환
		static std::shared_ptr<Bindable> Resolve(const std::string& key) noxnd
		{
			return Get().Resolve_(key);
		}
		//바인더블 객체를 저장한다. 바인더블 객체는 GetUID로 ID값을 반환하는 메서드가 있다.
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
		//해쉬 기반의 맵, 모든 바인더블객체는 고유한 해쉬키를 가지게된다.
		std::unordered_map<std::string, std::shared_ptr<Bindable>>binds;
	};


}