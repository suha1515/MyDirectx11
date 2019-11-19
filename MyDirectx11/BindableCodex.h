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
		// Codex클래스가 다양한 바인더블 객체를 저장해야하므로 각 바인더 오브젝트들은
		//  서로 다른 개수의 생성자를 가지고있다 그것을 대비하여 파라메터 팩으로 생성자를 구성한다.
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
			//찾는 바인더블 오브젝트가 없다면 새로 만든다.
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
		//해쉬 기반의 맵, 모든 바인더블객체는 고유한 해쉬키를 가지게된다.
		std::unordered_map<std::string, std::shared_ptr<Bindable>>binds;
	};


}