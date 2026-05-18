#pragma once
#include <vector>
#include <Tools/Singleton.h>
#include "RenderMode/LuaGuiMode.h"

namespace RenderUI {
	struct LuaMember {
		LuaGuiMode* member;
		std::string member_name;
		LuaMember() = default;
		LuaMember(LuaGuiMode* m, std::string n)
			: member(m), member_name(n)
		{
		}
	};

	class LuaUIMember : public Singleton<LuaUIMember>, public ILuaUIMember
	{
	public:
		virtual void Register(LuaMemberContext member) override;
		const std::vector<LuaMember>& GetLuaMember();

	private:
		std::vector<LuaMember> members;
	};
}