#pragma once
#include <vector>
#include <Tools/Singleton.h>
#include "RenderMode/LuaGuiMode.h"

namespace RenderUI {
	class LuaUIMember : public Singleton<LuaUIMember>, public ILuaUIMember
	{
	public:
		virtual void Register(LuaMemberContext member) override;
		virtual const std::vector<LuaMember>& GetLuaMember();
	private:
		std::vector<LuaMember> members;
	};
}